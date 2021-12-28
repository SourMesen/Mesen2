#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Mesen.Config;
using Avalonia.VisualTree;
using Avalonia.Input;
using System.Collections.Generic;
using Mesen.Localization;
using Mesen.Debugger.Labels;
using System.Collections.ObjectModel;
using Avalonia.Interactivity;

namespace Mesen.Debugger.Windows
{
	public class EventViewerWindow : Window
	{
		private NotificationListener _listener;
		private EventViewerViewModel _model;
		private DispatcherTimer _timer;

		//For designer
		[Obsolete] public EventViewerWindow() : this(CpuType.Cpu) { }

		public EventViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif

			PictureViewer viewer = this.FindControl<PictureViewer>("picViewer");
			_model = new EventViewerViewModel(cpuType, viewer, this);
			_model.Config.LoadWindowSettings(this);
			DataContext = _model;

			if(Design.IsDesignMode) {
				return;
			}

			viewer.PointerMoved += Viewer_PointerMoved;
			viewer.PointerLeave += Viewer_PointerLeave;

			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateConfig());
			_listener = new NotificationListener();
		}

		private void Viewer_PointerLeave(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				ToolTip.SetTip(viewer, null);
				ToolTip.SetIsOpen(viewer, false);
			}
		}

		private void Viewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				PointerPoint point = e.GetCurrentPoint(viewer);
				DebugEventInfo evt = new DebugEventInfo();
				DebugApi.GetEventViewerEvent(_model.CpuType, ref evt, (ushort)(point.Position.Y / _model.Config.ImageScale), (ushort)(point.Position.X / _model.Config.ImageScale));

				if(evt.ProgramCounter != UInt32.MaxValue) {
					//Force tooltip to update its position
					ToolTip.SetHorizontalOffset(viewer, 1);
					ToolTip.SetHorizontalOffset(viewer, 0);

					ToolTip.SetTip(viewer, new DynamicTooltip() { Items = GetTooltipData(evt) });
					ToolTip.SetIsOpen(viewer, true);
				} else {
					ToolTip.SetTip(viewer, null);
					ToolTip.SetIsOpen(viewer, false);
				}
			}
		}

		private List<TooltipEntry> GetTooltipData(DebugEventInfo evt)
		{
			List<TooltipEntry> entries = new() {
				new("Type", ResourceHelper.GetEnumText(evt.Type)),
				new("Scanline", evt.Scanline.ToString()),
				new(_model.CpuType == CpuType.Cpu ? "H-Clock" : "Cycle", evt.Cycle.ToString()),
				new("PC", "$" + evt.ProgramCounter.ToString("X" + _model.CpuType.GetAddressSize())),
			};

			switch(evt.Type) {
				case DebugEventType.Register:
					bool isWrite = evt.Operation.Type == MemoryOperationType.Write || evt.Operation.Type == MemoryOperationType.DmaWrite;
					bool isDma = evt.Operation.Type == MemoryOperationType.DmaWrite || evt.Operation.Type == MemoryOperationType.DmaRead;

					CodeLabel? label = LabelManager.GetLabel(new AddressInfo() { Address = (int)evt.Operation.Address, Type = SnesMemoryType.CpuMemory });
					string registerText = "$" + evt.Operation.Address.ToString("X4");
					if(label != null) {
						registerText = label.Label + " (" + registerText + ")";
					}

					entries.Add(new("Register", registerText + (isWrite ? " (Write)" : " (Read)") + (isDma ? " (DMA)" : "")));
					entries.Add(new("Value", "$" + evt.Operation.Value.ToString("X2")));

					if(isDma && _model.CpuType != CpuType.Gameboy) {
						bool indirectHdma = false;
						string channel = (evt.DmaChannel & 0x07).ToString();

						if((evt.DmaChannel & DebugEventViewModel.HdmaChannelFlag) != 0) {
							indirectHdma = evt.DmaChannelInfo.HdmaIndirectAddressing;
							channel += indirectHdma ? " (Indirect HDMA)" : " (HDMA)";
							entries.Add(new("Line Counter", "$" + evt.DmaChannelInfo.HdmaLineCounterAndRepeat.ToString("X2")));
						}

						entries.Add(new("Channel", channel));

						entries.Add(new("Mode", evt.DmaChannelInfo.TransferMode.ToString()));

						int aBusAddress;
						if(indirectHdma) {
							aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.TransferSize;
						} else {
							aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.SrcAddress;
						}

						if(!evt.DmaChannelInfo.InvertDirection) {
							entries.Add(new("Transfer", "$" + aBusAddress.ToString("X4") + " -> $" + evt.DmaChannelInfo.DestAddress.ToString("X2")));
						} else {
							entries.Add(new("Transfer", "$" + aBusAddress.ToString("X4") + " <- $" + evt.DmaChannelInfo.DestAddress.ToString("X2")));
						}
					}
					break;

				case DebugEventType.Breakpoint:
					ReadOnlyCollection<Breakpoint> breakpoints = BreakpointManager.Breakpoints;
					if(evt.BreakpointId >= 0 && evt.BreakpointId < breakpoints.Count) {
						Breakpoint bp = breakpoints[evt.BreakpointId];
						entries.Add(new("CPU Type", ResourceHelper.GetEnumText(bp.CpuType)));
						entries.Add(new("BP Type", bp.ToReadableType()));
						entries.Add(new("BP Addresses", bp.GetAddressString(true)));
						if(bp.Condition.Length > 0) {
							entries.Add(new("BP Condition", bp.Condition));
						}
					}
					break;
			}

			return entries;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_timer.Start();
			_listener.OnNotification += listener_OnNotification;
			_model.RefreshData();
		}

		private void UpdateConfig()
		{
			if(_model.ConsoleConfig is SnesEventViewerConfig snesCfg) {
				DebugApi.SetEventViewerConfig(_model.CpuType, snesCfg.ToInterop());
			} else if(_model.ConsoleConfig is NesEventViewerConfig nesCfg) {
				DebugApi.SetEventViewerConfig(_model.CpuType, nesCfg.ToInterop());
			} else if(_model.ConsoleConfig is GbEventViewerConfig gbCfg) {
				DebugApi.SetEventViewerConfig(_model.CpuType, gbCfg.ToInterop());
			}

			_model.RefreshTab();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_model.Config.SaveWindowSettings(this);
			_timer.Stop();
			_listener.Dispose();
			_model.SaveConfig();
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.EventViewerRefresh:
					if(_model.Config.AutoRefresh) {
						_model.RefreshData();
					}
					break;

				case ConsoleNotificationType.CodeBreak:
					if(_model.Config.RefreshOnBreakPause) {
						_model.RefreshData();
					}
					break;
			}
		}
	}
}
