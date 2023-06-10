using Avalonia;
using Avalonia.Controls;
using System;
using Avalonia.Interactivity;
using Avalonia.Styling;
using Avalonia.Input;
using Mesen.Debugger.Utilities;
using System.Globalization;

namespace Mesen.Controls
{
	public class ImageAspectRatio : Image
	{
		protected override Type StyleKeyOverride => typeof(Image);

		public ImageAspectRatio()
		{
		}

		protected override Size ArrangeOverride(Size finalSize)
		{
			finalSize = base.ArrangeOverride(finalSize);
			if(finalSize.Width >= finalSize.Height * 256 / 240) {
				return new Size(finalSize.Height * 256 / 240, finalSize.Height);
			} else {
				return new Size(finalSize.Width, finalSize.Width * 240 / 256);
			}
		}
	}
}
