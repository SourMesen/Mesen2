using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.GUI;
using ReactiveUI;
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Avalonia.Platform;

namespace Mesen.Debugger.Windows
{
	public class TileViewerWindow : Window
	{
		private NotificationListener _listener;
		private TileViewerViewModel _model;
		private PictureViewer _picViewer;
		private WriteableBitmap _viewerBitmap;

		public TileViewerWindow()
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

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
			
			_viewerBitmap = new WriteableBitmap(new PixelSize(48 * 8, 86 * 8), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			
			_picViewer = this.FindControl<PictureViewer>("picViewer");
			_picViewer.Source = _viewerBitmap;
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			_model = this.DataContext as TileViewerViewModel;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.PpuFrameDone) {
				return;
			}

			byte[] cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
			byte[] source = DebugApi.GetMemoryState(_model.MemoryType);

			using(var framebuffer = _viewerBitmap.Lock()) {
				DebugApi.GetTileView(new GetTileViewOptions() {
					Format = _model.TileFormat,
					Width = 48,
					PageSize = 0x10000,
					Palette = 6,
					Layout = _model.TileLayout,
					Background = _model.TileBackground
				}, source, 0x10000, cgram, framebuffer.Address);
			}

			Dispatcher.UIThread.Post(() => {
				_picViewer.InvalidateVisual();
			});
		}
	}
}
