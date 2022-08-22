using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Platform;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using System;

namespace Mesen
{
	public class NativeRenderer : NativeControlHost
   {
      public NativeRenderer()
      {
      }

      public IntPtr Handle { get; private set; }

      protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
      {
         var handle = base.CreateNativeControlCore(parent);
         Handle = handle.Handle;
         return handle;
      }

		protected override Size MeasureOverride(Size availableSize)
		{
			return availableSize;
		}

		protected override Size ArrangeOverride(Size finalSize)
		{
			double aspectRatio = EmuApi.GetAspectRatio();

			double width = finalSize.Width;
			double height = width / aspectRatio;
			if(height > finalSize.Height) {
				height = finalSize.Height;
				width = height * aspectRatio;
			}

			if(DataContext is MainWindowViewModel model) {
				if(ConfigManager.Config.Video.FullscreenForceIntegerScale && VisualRoot is Window wnd && wnd.WindowState == WindowState.FullScreen) {
					FrameInfo baseSize = EmuApi.GetBaseScreenSize();
					double scale = (height * LayoutHelper.GetLayoutScale(this)) / baseSize.Height;
					if(scale != Math.Floor(scale)) {
						height = baseSize.Height * Math.Max(1, Math.Floor(scale));
						width = height * aspectRatio;
					}
				}

				width = Math.Round(width * LayoutHelper.GetLayoutScale(this));
				height = Math.Round(height * LayoutHelper.GetLayoutScale(this));

				EmuApi.SetRendererSize((uint)width, (uint)height);
				model.RendererSize = new Size(width, height);
			} else if(DataContext is HistoryViewerViewModel historyViewer) {
				width = Math.Round(width * LayoutHelper.GetLayoutScale(this));
				height = Math.Round(height * LayoutHelper.GetLayoutScale(this));
				historyViewer.RendererSize = new Size(width, height);
			}

			return new Size(width, height);
		}
	}
}
