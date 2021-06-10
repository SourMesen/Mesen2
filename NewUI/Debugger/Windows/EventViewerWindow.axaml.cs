#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Avalonia.Platform;
using Mesen.Interop;
using System.ComponentModel;
using Mesen.Config;

namespace Mesen.Debugger.Windows
{
	public class EventViewerWindow : Window
	{
		private NotificationListener _listener;
		private EventViewerViewModel _model;
		private PictureViewer _picViewer;
		private WriteableBitmap _viewerBitmap;
		private DispatcherTimer _timer;

		public EventViewerWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			//Renderer.DrawFps = true;
			_picViewer = this.FindControl<PictureViewer>("picViewer");
			_picViewer.Source = _viewerBitmap;
			InitBitmap();

			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateConfig());
			_timer.Start();

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
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
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_timer?.Stop();
			_listener?.Dispose();
			_model.SaveConfig();
			base.OnClosing(e);
		}

		private void InitBitmap()
		{
			FrameInfo size = DebugApi.GetEventViewerDisplaySize(_model.CpuType);
			if(_viewerBitmap == null || _viewerBitmap.Size.Width != size.Width || _viewerBitmap.Size.Height != size.Height) {
				_viewerBitmap = new WriteableBitmap(new PixelSize((int)size.Width, (int)size.Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(this.DataContext is EventViewerViewModel model) {
				_model = model;
			} else {
				throw new Exception("Unexception model");
			}
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.EventViewerRefresh) {
				return;
			}

			DebugApi.TakeEventSnapshot(_model.CpuType);

			Dispatcher.UIThread.Post(() => {
				InitBitmap();
				using(var framebuffer = _viewerBitmap.Lock()) {
					DebugApi.GetEventViewerOutput(_model.CpuType, framebuffer.Address, (uint)(_viewerBitmap.Size.Width * _viewerBitmap.Size.Height * sizeof(UInt32)));
				}

				_picViewer.Source = _viewerBitmap;
				_picViewer.InvalidateVisual();
			});
		}
	}
}
