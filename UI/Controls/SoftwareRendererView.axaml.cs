using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Avalonia.Data;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.Interop;
using Splat.ModeDetection;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using Avalonia.Media;

namespace Mesen.Controls
{
	public class SoftwareRendererView : UserControl
	{
		private SimpleImageViewer _frame;
		private SimpleImageViewer _emuHud;
		private SimpleImageViewer _scriptHud;
		private SoftwareRendererViewModel _model = new();

		public SoftwareRendererView()
		{
			InitializeComponent();

			_frame = this.GetControl<SimpleImageViewer>("Frame");
			_emuHud = this.GetControl<SimpleImageViewer>("EmuHud");
			_scriptHud = this.GetControl<SimpleImageViewer>("ScriptHud");

			RenderOptions.SetBitmapInterpolationMode(_emuHud, BitmapInterpolationMode.None);
			RenderOptions.SetBitmapInterpolationMode(_scriptHud, BitmapInterpolationMode.None);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			if(DataContext is SoftwareRendererViewModel model) {
				_model = model;
			}
		}

		private unsafe void UpdateSurface(SoftwareRendererSurface frame, WriteableBitmap? surface, Action<WriteableBitmap> updateSurfaceRef)
		{
			PixelSize frameSize = new PixelSize((int)frame.Width, (int)frame.Height);
			if(surface?.PixelSize != frameSize) {
				surface = new WriteableBitmap(frameSize, new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
				updateSurfaceRef(surface);
			}

			int size = (int)frame.Width * (int)frame.Height * sizeof(UInt32);
			using(var bitmapLock = surface.Lock()) {
				var srcSpan = new Span<byte>((byte*)frame.FrameBuffer, size);
				var dstSpan = new Span<byte>((byte*)bitmapLock.Address, size);
				srcSpan.CopyTo(dstSpan);
			}
		}

		public unsafe void UpdateSoftwareRenderer(SoftwareRendererFrame frameInfo)
		{
			UpdateSurface(frameInfo.Frame, _model.FrameSurface, s => _model.FrameSurface = s);
			if(frameInfo.EmuHud.IsDirty) {
				UpdateSurface(frameInfo.EmuHud, _model.EmuHudSurface, s => _model.EmuHudSurface = s);
			}
			if(frameInfo.ScriptHud.IsDirty) {
				UpdateSurface(frameInfo.ScriptHud, _model.ScriptHudSurface, s => _model.ScriptHudSurface = s);
			}

			Dispatcher.UIThread.Post(() => {
				RenderOptions.SetBitmapInterpolationMode(_frame, ConfigManager.Config.Video.UseBilinearInterpolation ? BitmapInterpolationMode.LowQuality : BitmapInterpolationMode.None);
				_frame.InvalidateVisual();
				_emuHud.InvalidateVisual();
				_scriptHud.InvalidateVisual();
			}, DispatcherPriority.MaxValue);
		}
	}

	public class SoftwareRendererViewModel : ViewModelBase
	{
		[Reactive] public WriteableBitmap? FrameSurface { get; set; }
		[Reactive] public WriteableBitmap? EmuHudSurface { get; set; }
		[Reactive] public WriteableBitmap? ScriptHudSurface { get; set; }
		[Reactive] public double Width { get; set; }
		[Reactive] public double Height { get; set; }
	}
}
