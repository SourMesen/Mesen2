using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Mesen.Config;
using Avalonia.Input;
using Mesen.Localization;
using Mesen.Debugger.Labels;
using Avalonia.Interactivity;
using Mesen.Utilities;
using System.Linq;
using Avalonia.Threading;
using Mesen.Debugger.Utilities;
using DynamicData;
using DataBoxControl;

namespace Mesen.Debugger.Windows
{
	public class EventViewerWindow : MesenWindow, INotificationHandler
	{
		private EventViewerViewModel _model;
		private PixelPoint? _prevMousePos = null;

		[Obsolete("For designer only")]
		public EventViewerWindow() : this(CpuType.Snes) { }

		public EventViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PictureViewer viewer = this.GetControl<ScrollPictureViewer>("picViewer").InnerViewer;
			DataBox listView = this.GetControl<DataBox>("lstEvents");
			_model = new EventViewerViewModel(cpuType, viewer, listView, this);
			_model.Config.LoadWindowSettings(this);
			DataContext = _model;

			if(Design.IsDesignMode) {
				return;
			}

			viewer.PointerMoved += Viewer_PointerMoved;
			viewer.PointerExited += Viewer_PointerExited;
			viewer.PointerPressed += Viewer_PointerPressed;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void Viewer_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				PixelPoint? point = viewer.GetGridPointFromMousePoint(e.GetCurrentPoint(viewer).Position);
				if(point == null) {
					return;
				}

				DebugEventInfo? evt = DebugApi.GetEventViewerEvent(_model.CpuType, (ushort)point.Value.Y, (ushort)point.Value.X);
				if(evt != null) {
					_model.UpdateSelectedEvent(evt);
					int index = _model.ListView.RawDebugEvents.IndexOf(evt.Value);
					if(index >= 0) {
						_model.ListView.Selection.Clear();
						_model.ListView.Selection.Select(index);
					}
				} else {
					_model.UpdateSelectedEvent(null);
				}
			}
		}

		private void Viewer_PointerExited(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				TooltipHelper.HideTooltip(viewer);
			}
			_prevMousePos = null;
			_model.GridHighlightPoint = null;
		}

		private void Viewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				PixelPoint? point = viewer.GetGridPointFromMousePoint(e.GetCurrentPoint(viewer).Position);
				if(point == _prevMousePos) {
					return;
				}
				_prevMousePos = point;

				DebugEventInfo? evt = null;
				if(point != null) {
					evt = DebugApi.GetEventViewerEvent(_model.CpuType, (ushort)point.Value.Y, (ushort)point.Value.X);
					_model.UpdateHighlightPoint(point.Value, evt);
				}

				if(point != null && evt != null) {
					TooltipHelper.ShowTooltip(viewer, new DynamicTooltip() { Items = GetTooltipData(evt.Value) }, 15);
				} else {
					TooltipHelper.HideTooltip(viewer);
				}
			}
		}

		private TooltipEntries GetTooltipData(DebugEventInfo evt)
		{
			TooltipEntries entries = new();
			entries.AddEntry("Type", ResourceHelper.GetEnumText(evt.Type));
			
			entries.AddSeparator("LocationSeparator");

			entries.AddEntry("Scanline", evt.Scanline.ToString());
			entries.AddEntry(_model.CpuType == CpuType.Snes ? "H-Clock" : "Cycle", evt.Cycle.ToString());
			entries.AddEntry("PC", "$" + evt.ProgramCounter.ToString("X" + _model.CpuType.GetAddressSize()));

			entries.AddSeparator("AddressValueSeparator");

			if(evt.Flags.HasFlag(EventFlags.ReadWriteOp)) {
				bool isWrite = evt.Operation.Type == MemoryOperationType.Write || evt.Operation.Type == MemoryOperationType.DmaWrite;
				bool isDma = evt.Operation.Type == MemoryOperationType.DmaWrite || evt.Operation.Type == MemoryOperationType.DmaRead;

				CodeLabel? label = LabelManager.GetLabel(new AddressInfo() { Address = (int)evt.Operation.Address, Type = _model.CpuType.ToMemoryType() });
				string registerText = "$" + evt.Operation.Address.ToString("X4");
				if(label != null) {
					registerText = label.Label + " (" + registerText + ")";
				}
				if(evt.RegisterId >= 0) {
					registerText += $" ({evt.GetRegisterName()} - ${evt.RegisterId:X2})";
				}

				entries.AddEntry(evt.Type == DebugEventType.Register ? "Register" : "Address", registerText + (isWrite ? " (Write)" : " (Read)") + (isDma ? " (DMA)" : ""));
				entries.AddEntry("Value", "$" + evt.Operation.Value.ToString("X2"));
			}

			string details = EventViewerViewModel.GetEventDetails(_model.CpuType, evt, false);
			if(details.Length > 0) {
				entries.AddSeparator("DetailsSeparator");
				entries.AddEntry("Details", details);
			}

			entries.EndUpdate();

			return entries;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					RomInfo romInfo = EmuApi.GetRomInfo();
					if(!romInfo.CpuTypes.Contains(_model.CpuType)) {
						_model.CpuType = romInfo.ConsoleType.GetMainCpuType();
						Dispatcher.UIThread.Post(() => {
							if(DebugWindowManager.GetDebugWindow<EventViewerWindow>(x => x._model.CpuType == _model.CpuType) != this) {
								//Found another window for the same CPU type, close this one (can happen when opening event viewers with SGB on both SNES & GB)
								Close();
							} else {
								_model.UpdateConfig();
							}
						});
					} else {
						Dispatcher.UIThread.Post(() => {
							_model.UpdateConfig();
						});
					}
					break;

				case ConsoleNotificationType.EventViewerRefresh:
					CpuType cpuType = (CpuType)e.Parameter;
					if(_model.CpuType == cpuType && _model.Config.AutoRefresh && !ToolRefreshHelper.LimitFps(this, 80)) {
						_model.RefreshData(true);
					}
					break;

				case ConsoleNotificationType.StateLoaded:
					_model.RefreshData();
					break;

				case ConsoleNotificationType.CodeBreak:
					if(_model.Config.RefreshOnBreakPause) {
						_model.RefreshData();
					}
					break;
			}
		}

		public static EventViewerWindow GetOrOpenWindow(CpuType cpuType)
		{
			EventViewerWindow? wnd = DebugWindowManager.GetDebugWindow<EventViewerWindow>(x => x._model.CpuType == cpuType);
			if(wnd == null) {
				return DebugWindowManager.OpenDebugWindow<EventViewerWindow>(() => new EventViewerWindow(cpuType));
			} else {
				wnd.BringToFront();
			}
			return wnd;
		}
	}
}
