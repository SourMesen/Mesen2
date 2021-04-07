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
using Mesen.Debugger.ViewModels;
using Mesen.GUI.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;

namespace Mesen.Debugger.Windows
{
	public class DebuggerWindow : Window
	{
		public DebuggerWindow()
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

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			_context = this.DataContext as DebuggerWindowViewModel;
			_context.Disassembly.StyleProvider = new BaseStyleProvider();
		}

		DebuggerWindowViewModel _context;
		NotificationListener _listener;
		Avalonia.Controls.Image _picDebug;
		Avalonia.Controls.Image _picDebug2;
		Avalonia.Controls.Image _picDebug3;
		Avalonia.Controls.Image _picDebug4;
		Avalonia.Controls.Image _picDebug5;
		Avalonia.Controls.Image _picDebug6;
		Avalonia.Controls.Image _picDebug7;
		Avalonia.Controls.Image _picDebug8;
		Avalonia.Controls.Image _picDebug9;
		Avalonia.Controls.Image _picDebug10;
		WriteableBitmap _bmp;
		WriteableBitmap _bmp2;
		WriteableBitmap _bmp3;
		WriteableBitmap _bmp4;
		WriteableBitmap _bmp5;
		WriteableBitmap _bmp6;
		WriteableBitmap _bmp7;
		WriteableBitmap _bmp8;
		WriteableBitmap _bmp9;
		WriteableBitmap _bmp10;
		DisassemblyViewer _disView;
		EventViewerDisplayOptions evtOpts = new EventViewerConfig().GetInteropOptions();

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			//ConfigApi.SetEmulationFlag(EmulationFlags.MaximumSpeed, true);

			//Renderer.DrawFps = true;

			_disView = this.FindControl<DisassemblyViewer>("DisassemblyView");

			/*_picDebug = this.FindControl<Avalonia.Controls.Image>("picDebug");
			_picDebug2 = this.FindControl<Avalonia.Controls.Image>("picDebug2");
			_picDebug3 = this.FindControl<Avalonia.Controls.Image>("picDebug3");
			_picDebug4 = this.FindControl<Avalonia.Controls.Image>("picDebug4");
			_picDebug5 = this.FindControl<Avalonia.Controls.Image>("picDebug5");
			
			_picDebug6 = this.FindControl<Avalonia.Controls.Image>("picDebug6");
			_picDebug7 = this.FindControl<Avalonia.Controls.Image>("picDebug7");
			_picDebug8 = this.FindControl<Avalonia.Controls.Image>("picDebug8");
			_picDebug9 = this.FindControl<Avalonia.Controls.Image>("picDebug9");
			_picDebug10 = this.FindControl<Avalonia.Controls.Image>("picDebug10");

			_bmp = new WriteableBitmap(new PixelSize(1364 / 2, 262*2), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp2 = new WriteableBitmap(new PixelSize(1364 / 2, 262*2), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp3 = new WriteableBitmap(new PixelSize(1364 / 2, 262*2), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp4 = new WriteableBitmap(new PixelSize(1364 / 2, 262*2), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp5 = new WriteableBitmap(new PixelSize(1364 / 2, 262*2), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			
			_bmp6 = new WriteableBitmap(new PixelSize(1024, 1024), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp7 = new WriteableBitmap(new PixelSize(1024, 1024), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp8 = new WriteableBitmap(new PixelSize(256, 256), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp9 = new WriteableBitmap(new PixelSize(48*8, 48 * 8), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);
			_bmp10 = new WriteableBitmap(new PixelSize(48 * 8, 48 * 8), new Vector(96, 96), Avalonia.Platform.PixelFormat.Bgra8888, Avalonia.Platform.AlphaFormat.Premul);

			_picDebug.Source = _bmp;
			_picDebug2.Source = _bmp2;
			_picDebug3.Source = _bmp3;
			_picDebug4.Source = _bmp4;
			_picDebug5.Source = _bmp5;

			_picDebug6.Source = _bmp6;
			_picDebug7.Source = _bmp7;
			_picDebug8.Source = _bmp8;
			_picDebug9.Source = _bmp9;
			_picDebug10.Source = _bmp10;

			Task.Factory.StartNew(() => {

				while(waitHandle.WaitOne()) {
					using(var framebuffer = _bmp.Lock()) {
						DebugApi.GetEventViewerOutputWrapper(CpuType.Cpu, framebuffer.Address, 1364 / 2 * 4 * 262 * 2, evtOpts);
					}
					Dispatcher.UIThread.Post(() => {
						_picDebug.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);

			Task.Factory.StartNew(() => {

				while(waitHandle2.WaitOne()) {
					using(var framebuffer = _bmp2.Lock()) {
						DebugApi.GetEventViewerOutputWrapper(CpuType.Cpu, framebuffer.Address, 1364 / 2 * 4 * 262 * 2, evtOpts);
					}

					Dispatcher.UIThread.Post(() => {

						_picDebug2.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);


			Task.Factory.StartNew(() => {

				while(waitHandle3.WaitOne()) {
					using(var framebuffer = _bmp3.Lock()) {
						DebugApi.GetEventViewerOutputWrapper(CpuType.Cpu, framebuffer.Address, 1364 / 2 * 4 * 262 * 2, evtOpts);
					}

					Dispatcher.UIThread.Post(() => {

						_picDebug3.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);


			Task.Factory.StartNew(() => {

				while(waitHandle4.WaitOne()) {
					using(var framebuffer = _bmp4.Lock()) {
						DebugApi.GetEventViewerOutputWrapper(CpuType.Cpu, framebuffer.Address, 1364 / 2 * 4 * 262 * 2, evtOpts);
					}
					Dispatcher.UIThread.Post(() => {

						_picDebug4.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);


			Task.Factory.StartNew(() => {

				while(waitHandle5.WaitOne()) {
					using(var framebuffer = _bmp5.Lock()) {
						DebugApi.GetEventViewerOutputWrapper(CpuType.Cpu, framebuffer.Address, 1364 / 2 * 4 * 262 * 2, evtOpts);
					}
					Dispatcher.UIThread.Post(() => {

						_picDebug5.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);


			Task.Factory.StartNew(() => {

				while(waitHandle6.WaitOne()) {
					using(var framebuffer = _bmp6.Lock()) {
						DebugApi.GetTilemap(new GetTilemapOptions() { Layer = 0 }, state.Ppu, vram, cgram, framebuffer.Address);
					}
					Dispatcher.UIThread.Post(() => {

						_picDebug6.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);



			Task.Factory.StartNew(() => {

				while(waitHandle7.WaitOne()) {
					using(var framebuffer = _bmp7.Lock()) {
						DebugApi.GetTilemap(new GetTilemapOptions() { Layer = 1 }, state.Ppu, vram, cgram, framebuffer.Address);
					}
					Dispatcher.UIThread.Post(() => {

						_picDebug7.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);


			Task.Factory.StartNew(() => {

				while(waitHandle8.WaitOne()) {
					using(var framebuffer = _bmp8.Lock()) {
						DebugApi.GetSpritePreview(new GetSpritePreviewOptions() { SelectedSprite = -1 }, state.Ppu, vram, oamram, cgram, framebuffer.Address);
					}
					Dispatcher.UIThread.Post(() => {

						_picDebug8.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);


			Task.Factory.StartNew(() => {

				while(waitHandle9.WaitOne()) {
					using(var framebuffer = _bmp9.Lock()) {
						DebugApi.GetTileView(new GetTileViewOptions() { Format = TileFormat.Bpp4, Width = 48, PageSize = 0x10000, Palette = 6 }, vram, 0x10000, cgram, framebuffer.Address);
					}
					Dispatcher.UIThread.Post(() => {

						_picDebug9.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);

			Task.Factory.StartNew(() => {
				while(waitHandle10.WaitOne()) {
					using(var framebuffer = _bmp10.Lock()) {
						DebugApi.GetTileView(new GetTileViewOptions() { Format = TileFormat.Bpp4, Width = 48, PageSize = 0x10000, Palette = 4 }, vram, 0x10000, cgram, framebuffer.Address);
					}

					Dispatcher.UIThread.Post(() => {
						_picDebug10.InvalidateVisual();
					});
				}
			}, TaskCreationOptions.LongRunning);

			*/
			_listener = new NotificationListener();
			_listener.OnNotification += _listener_OnNotification;
		}

		AutoResetEvent waitHandle = new AutoResetEvent(false);
		AutoResetEvent waitHandle2 = new AutoResetEvent(false);
		AutoResetEvent waitHandle3 = new AutoResetEvent(false);
		AutoResetEvent waitHandle4 = new AutoResetEvent(false);
		AutoResetEvent waitHandle5 = new AutoResetEvent(false);
		AutoResetEvent waitHandle6 = new AutoResetEvent(false);
		AutoResetEvent waitHandle7 = new AutoResetEvent(false);
		AutoResetEvent waitHandle8 = new AutoResetEvent(false);
		AutoResetEvent waitHandle9 = new AutoResetEvent(false);
		AutoResetEvent waitHandle10 = new AutoResetEvent(false);

		DebugState state;
		byte[] oamram;
		byte[] cgram;
		byte[] vram;
		int frmCnt = 0;
		private void _listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.PpuFrameDone) {
				return;
			}
			frmCnt++;

			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesDebuggerEnabled, true);

			if(frmCnt % 200 == 0) {
				DebugApi.RefreshDisassembly(CpuType.Nes);
			}

			_context.Disassembly.DataProvider = new CodeDataProvider(CpuType.Nes);
			_context.Disassembly.UpdateMaxScroll();

			/*ConfigApi.SetDebuggerFlag(DebuggerFlags.CpuDebuggerEnabled, true);

			state = DebugApi.GetState();

			cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
			vram = DebugApi.GetMemoryState(SnesMemoryType.VideoRam);
			oamram = DebugApi.GetMemoryState(SnesMemoryType.SpriteRam);

			DebugApi.TakeEventSnapshot(CpuType.Cpu, evtOpts);

			if(frmCnt == 200) {
				DebugApi.RefreshDisassembly(CpuType.Cpu);
			}

			_context.Disassembly.DataProvider = new CodeDataProvider(CpuType.Cpu);
			_context.Disassembly.UpdateMaxScroll();
			_context.SnesCpu.State = state.Cpu;
			_context.SnesPpu.State = state.Ppu;*/

			/*waitHandle.Set();
			waitHandle2.Set();
			waitHandle6.Set();
			waitHandle7.Set();
			waitHandle8.Set();
			waitHandle9.Set();
			waitHandle10.Set();*/
		}
	}
}
