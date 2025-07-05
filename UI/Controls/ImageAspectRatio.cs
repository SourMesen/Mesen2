using Avalonia;
using Avalonia.Controls;
using System;

namespace Mesen.Controls
{
	public class ImageAspectRatio : Image
	{
		protected override Type StyleKeyOverride => typeof(Image);
		
		public static readonly StyledProperty<double> AspectRatioProperty = AvaloniaProperty.Register<StateGridEntry, double>(nameof(AspectRatio));

		public ImageAspectRatio()
		{
		}

		public double AspectRatio
		{
			get { return GetValue(AspectRatioProperty); }
			set { SetValue(AspectRatioProperty, value); }
		}

		protected override Size ArrangeOverride(Size finalSize)
		{
			finalSize = base.ArrangeOverride(finalSize);
			if(Source == null) {
				return finalSize;
			}

			double ratio = AspectRatio;
			if(ratio == 0) {
				ratio = Source.Size.Width / Source.Size.Height;
			}

			if(finalSize.Width >= finalSize.Height * ratio) {
				return new Size(finalSize.Height * ratio, finalSize.Height);
			} else {
				return new Size(finalSize.Width, finalSize.Width / ratio);
			}
		}
	}
}
