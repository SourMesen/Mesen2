using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Avalonia.Interactivity;
using Mesen.Debugger.Utilities;
using Avalonia.Input;

namespace Mesen.Debugger.Windows
{
	public class PaletteViewerWindow : Window, INotificationHandler
	{
		private PaletteViewerViewModel _model;

		[Obsolete("For designer only")]
		public PaletteViewerWindow() : this(CpuType.Snes) { }

		public PaletteViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PaletteSelector palSelector = this.GetControl<PaletteSelector>("palSelector");
			_model = new PaletteViewerViewModel(cpuType);
			_model.InitActions(this, palSelector);
			DataContext = _model;

			_model.Config.LoadWindowSettings(this);

			if(Design.IsDesignMode) {
				return;
			}

			palSelector.PointerMoved += PalSelector_PointerMoved;
			palSelector.PointerExited += PalSelector_PointerExited;
			PointerWheelChanged += Window_PointerWheelChanged;
		}

		private void Window_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
		{
			if(e.KeyModifiers == KeyModifiers.Control) {
				if(e.Delta.Y > 0) {
					_model.ZoomIn();
				} else {
					_model.ZoomOut();
				}
				e.Handled = true;
			}
		}

		private void PalSelector_PointerExited(object? sender, Avalonia.Input.PointerEventArgs e)
		{
			if(sender is PaletteSelector viewer) {
				ToolTip.SetTip(viewer, null);
				ToolTip.SetIsOpen(viewer, false);
			}
			_model.ViewerTooltip = null;
			_model.ViewerMouseOverPalette = -1;
		}

		private void PalSelector_PointerMoved(object? sender, Avalonia.Input.PointerEventArgs e)
		{
			if(sender is PaletteSelector viewer) {
				int index = viewer.GetPaletteIndexFromPoint(e.GetCurrentPoint(viewer).Position);
				if(_model.ViewerMouseOverPalette == index) {
					return;
				}

				_model.ViewerTooltip = _model.GetPreviewPanel(index, _model.ViewerTooltip);
				_model.ViewerMouseOverPalette = index;

				if(_model.ViewerTooltip != null) {
					ToolTip.SetTip(viewer, _model.ViewerTooltip);
					
					//Force tooltip to update its position
					ToolTip.SetHorizontalOffset(viewer, 14);
					ToolTip.SetHorizontalOffset(viewer, 15);
					ToolTip.SetIsOpen(viewer, true);
				} else {
					ToolTip.SetTip(viewer, null);
					ToolTip.SetIsOpen(viewer, false);
				}
			}
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

			_model.RefreshData();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			ToolRefreshHelper.ProcessNotification(this, e, _model.RefreshTiming, _model, _model.RefreshData);
		}
	}
}
