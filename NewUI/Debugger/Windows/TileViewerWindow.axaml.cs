#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

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

namespace Mesen.Debugger.Windows
{
	public class TileViewerWindow : Window
	{
		private NotificationListener _listener;
		private TileViewerViewModel _model;

		[Obsolete("For designer only")]
		public TileViewerWindow() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public TileViewerWindow(CpuType cpuType, ConsoleType consoleType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PictureViewer picViewer = this.FindControl<PictureViewer>("picViewer");
			_model = new TileViewerViewModel(cpuType, consoleType, picViewer, this);
			DataContext = _model;

			_model.Config.LoadWindowSettings(this);
			_listener = new NotificationListener();

			if(Design.IsDesignMode) {
				return;
			}

			//picViewer.PointerMoved += PicViewer_PointerMoved;
			//picViewer.PointerLeave += PicViewer_PointerLeave;
			//picViewer.PositionClicked += PicViewer_PositionClicked;
			_listener.OnNotification += listener_OnNotification;
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

			_listener.OnNotification += listener_OnNotification;
			_model.RefreshData();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_listener.Dispose();
			_model.Config.SaveWindowSettings(this);
			_model.Dispose();
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			ToolRefreshHelper.ProcessNotification(this, e, _model.Config.RefreshTiming, _model.CpuType, _model.RefreshData);
		}
	}
}
