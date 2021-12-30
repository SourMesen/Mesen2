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
using Mesen.Utilities;

namespace Mesen.Debugger.Windows
{
	public class EventViewerWindow : Window
	{
		private NotificationListener _listener;
		private EventViewerViewModel _model;
		private PixelPoint? _prevMousePos = null;

		[Obsolete("For designer only")]
		public EventViewerWindow() : this(CpuType.Cpu) { }

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

			_listener = new NotificationListener();
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

			UpdateConfig();
			ReactiveHelper.RegisterRecursiveObserver(_model.ConsoleConfig, Config_PropertyChanged);
			_listener.OnNotification += listener_OnNotification;
			_model.RefreshData(true);
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			UpdateConfig();
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

			_model.RefreshTab(true);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_model.Config.SaveWindowSettings(this);
			ReactiveHelper.UnregisterRecursiveObserver(_model.ConsoleConfig, Config_PropertyChanged);
			_listener.Dispose();
			_model.SaveConfig();
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void Viewer_PointerLeave(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				ToolTip.SetTip(viewer, null);
				ToolTip.SetIsOpen(viewer, false);
			}
			_prevMousePos = null;
		}

		private void Viewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				PixelPoint? point = viewer.GetGridPointFromMousePoint(e.GetCurrentPoint(viewer).Position);
				if(point == _prevMousePos) {
					return;
				}
				_prevMousePos = point;

				DebugEventInfo evt = new DebugEventInfo();
				if(point != null) {
					DebugApi.GetEventViewerEvent(_model.CpuType, ref evt, (ushort)point.Value.Y, (ushort)point.Value.X);
				}

				if(point != null && evt.ProgramCounter != UInt32.MaxValue) {
					//Force tooltip to update its position
					ToolTip.SetHorizontalOffset(viewer, 14);
					ToolTip.SetHorizontalOffset(viewer, 15);

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
					break;
			}

			string details = EventViewerViewModel.GetEventDetails(evt, false);
			if(details.Length > 0) {
				entries.Add(new("Details", details));
			}

			return entries;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.EventViewerRefresh:
					if(_model.Config.AutoRefresh) {
						_model.RefreshData(false);
					}
					break;

				case ConsoleNotificationType.CodeBreak:
					if(_model.Config.RefreshOnBreakPause) {
						_model.RefreshData(true);
					}
					break;
			}
		}
	}
}
