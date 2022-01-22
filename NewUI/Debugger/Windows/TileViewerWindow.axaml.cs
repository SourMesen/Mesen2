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
	public class TileViewerWindow : Window, INotificationHandler
	{
		private TileViewerViewModel _model;

		[Obsolete("For designer only")]
		public TileViewerWindow() : this(CpuType.Snes) { }

		public TileViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PictureViewer picViewer = this.FindControl<PictureViewer>("picViewer");
			_model = new TileViewerViewModel(cpuType, picViewer, this);
			DataContext = _model;

			_model.Config.LoadWindowSettings(this);

			if(Design.IsDesignMode) {
				return;
			}

			//picViewer.PointerMoved += PicViewer_PointerMoved;
			//picViewer.PointerLeave += PicViewer_PointerLeave;
			//picViewer.PositionClicked += PicViewer_PositionClicked;
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
			_model.Dispose();
			DataContext = null;
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			ToolRefreshHelper.ProcessNotification(this, e, _model.Config.RefreshTiming, _model, _model.RefreshData);
		}
	}
}
