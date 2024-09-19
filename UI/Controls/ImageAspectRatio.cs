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
			if(Source == null) {
				return finalSize;
			}

			double ratio = Source.Size.Width / Source.Size.Height;
			if(finalSize.Width >= finalSize.Height * ratio) {
				return new Size(finalSize.Height * ratio, finalSize.Height);
			} else {
				return new Size(finalSize.Width, finalSize.Width / ratio);
			}
		}
	}
}
