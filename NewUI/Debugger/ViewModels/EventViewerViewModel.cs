using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class EventViewerViewModel : ViewModelBase
	{
		[Reactive] public WriteableBitmap ViewerBitmap { get; private set; }
		[Reactive] public EventViewerTab SelectedTab { get; set; }
		
		[Reactive] public AvaloniaList<DebugEventViewModel> DebugEvents { get; private set; } = new();
		private DebugEventInfo[] _debugEvents = new DebugEventInfo[0];

		public CpuType CpuType { get; }
		public EventViewerConfig Config { get; }
		public object ConsoleConfig { get; set; }
		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private PictureViewer _picViewer;

		//For designer
		public EventViewerViewModel() : this(CpuType.Nes, new PictureViewer(), null) { }

		public EventViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			CpuType = cpuType;
			_picViewer = picViewer;
			Config = ConfigManager.Config.Debug.EventViewer;
			InitBitmap(new FrameInfo() { Width = 1, Height = 1 });

			ConsoleConfig = cpuType switch {
				CpuType.Cpu => Config.SnesConfig,
				CpuType.Nes => Config.NesConfig,
				CpuType.Gameboy => Config.GbConfig,
				_ => throw new Exception("Invalid cpu type")
			};

			FileMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			};

			ViewMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.AutoRefresh,
					OnClick = () => Config.AutoRefresh = !Config.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshOnBreakPause,
					OnClick = () => Config.RefreshOnBreakPause = !Config.RefreshOnBreakPause
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.ZoomIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomIn),
					OnClick = () => _picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => _picViewer.ZoomOut()
				},
			};

			this.WhenAnyValue(x => x.SelectedTab).Subscribe(x => RefreshTab(x));

			if(wnd == null) {
				return;
			}

			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		public void SaveConfig()
		{
			ConfigManager.Config.Save();
		}

		private void InitBitmap()
		{
			InitBitmap(DebugApi.GetEventViewerDisplaySize(CpuType));
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap(FrameInfo size)
		{
			if(ViewerBitmap == null || ViewerBitmap.Size.Width != size.Width || ViewerBitmap.Size.Height != size.Height) {
				ViewerBitmap = new WriteableBitmap(new PixelSize((int)size.Width, (int)size.Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		public void RefreshData()
		{
			DebugApi.TakeEventSnapshot(CpuType);
			RefreshTab(SelectedTab);
		}

		public void RefreshTab(EventViewerTab tab)
		{
			Dispatcher.UIThread.Post(() => {
				if(SelectedTab == EventViewerTab.PpuView) {
					InitBitmap();
					using(var framebuffer = ViewerBitmap.Lock()) {
						DebugApi.GetEventViewerOutput(CpuType, framebuffer.Address, (uint)(ViewerBitmap.Size.Width * ViewerBitmap.Size.Height * sizeof(UInt32)));
					}

					_picViewer.InvalidateVisual();
				} else {
					_debugEvents = DebugApi.GetDebugEvents(CpuType);

					if(DebugEvents.Count < _debugEvents.Length) {
						for(int i = 0; i < DebugEvents.Count; i++) {
							DebugEvents[i].Update(_debugEvents, i);
						}
						DebugEvents.AddRange(Enumerable.Range(DebugEvents.Count, _debugEvents.Length - DebugEvents.Count).Select(i => new DebugEventViewModel(_debugEvents, i)));
					} else if(DebugEvents.Count > _debugEvents.Length) {
						for(int i = 0; i < _debugEvents.Length; i++) {
							DebugEvents[i].Update(_debugEvents, i);
						}
						DebugEvents.RemoveRange(_debugEvents.Length, DebugEvents.Count - _debugEvents.Length);
					} else {
						for(int i = 0; i < DebugEvents.Count; i++) {
							DebugEvents[i].Update(_debugEvents, i);
						}
					}
				}
			});
		}

		public enum EventViewerTab
		{
			PpuView,
			ListView,
		}
	}

	public class DebugEventViewModel : INotifyPropertyChanged
	{
		public const int HdmaChannelFlag = 0x40;

		private DebugEventInfo[] _events = Array.Empty<DebugEventInfo>();
		private int _index;

		public event PropertyChangedEventHandler? PropertyChanged;

		private string _programCounter = "";
		public string ProgramCounter
		{
			get
			{
				UpdateFields();
				return _programCounter;
			}
		}

		public string Scanline { get; set; } = "";
		public string Cycle { get; set; } = "";
		public string Type { get; set; } = "";
		public string Address { get; set; } = "";
		public string Value { get; set; } = "";
		public string Details { get; set; } = "";

		public DebugEventViewModel(DebugEventInfo[] events, int index)
		{
			Update(events, index);
		}

		public void Update(DebugEventInfo[] events, int index)
		{
			_events = events;
			_index = index;
			
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("ProgramCounter"));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Scanline"));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Cycle"));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Type"));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Address"));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Value"));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Details"));
		}

		private void UpdateFields()
		{
			DebugEventInfo evt = _events[_index];
			_programCounter = "$" + evt.ProgramCounter.ToString("X4");
			Scanline = evt.Scanline.ToString();
			Cycle = evt.Cycle.ToString();
			Address = evt.Type == DebugEventType.Register ? "$" + evt.Operation.Address.ToString("X4") : "";
			Value = evt.Type == DebugEventType.Register ? "$" + evt.Operation.Value.ToString("X2") : "";
			Type = ResourceHelper.GetEnumText(evt.Type);
			if(evt.Type == DebugEventType.Register) {
				Type += evt.Operation.Type == MemoryOperationType.Write || evt.Operation.Type == MemoryOperationType.DmaWrite ? " (W)" : " (R)";
			}
			Details = GetDetails(evt);
		}

		private string GetDetails(DebugEventInfo evt)
		{
			bool isDma = evt.Operation.Type == MemoryOperationType.DmaWrite || evt.Operation.Type == MemoryOperationType.DmaRead;

			string details = "";
			if(evt.Type == DebugEventType.Breakpoint && evt.BreakpointId >= 0) {
				var breakpoints = BreakpointManager.Breakpoints;
				if(evt.BreakpointId < breakpoints.Count) {
					Breakpoint bp = breakpoints[evt.BreakpointId];
					details += " Type: " + bp.ToReadableType();
					details += " Addresses: " + bp.GetAddressString(true);
					if(bp.Condition.Length > 0) {
						details += " Condition: " + bp.Condition;
					}
				}
			}

			if(isDma) {
				bool indirectHdma = false;
				if((evt.DmaChannel & DebugEventViewModel.HdmaChannelFlag) != 0) {
					indirectHdma = evt.DmaChannelInfo.HdmaIndirectAddressing;
					details += "HDMA #" + (evt.DmaChannel & 0x07).ToString();
					details += indirectHdma ? " (indirect)" : "";
				} else {
					details += "DMA #" + (evt.DmaChannel & 0x07).ToString();
				}

				int aBusAddress;
				if(indirectHdma) {
					aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.TransferSize;
				} else {
					aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.SrcAddress;
				}

				if(!evt.DmaChannelInfo.InvertDirection) {
					details += " - $" + aBusAddress.ToString("X4") + " -> $" + (0x2100 | evt.DmaChannelInfo.DestAddress).ToString("X4");
				} else {
					details += " - $" + aBusAddress.ToString("X4") + " <- $" + (0x2100 | evt.DmaChannelInfo.DestAddress).ToString("X4");
				}
			}

			return details;
		}
	}
}
