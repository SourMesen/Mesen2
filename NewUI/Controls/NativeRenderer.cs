using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Platform;
using Mesen.GUI;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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

			EmuApi.SetRendererSize((uint)width, (uint)height);
			return new Size(width, height);
		}
	}
}
