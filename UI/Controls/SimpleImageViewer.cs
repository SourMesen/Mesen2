using Avalonia;
using Avalonia.Controls;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

namespace Mesen.Controls
{
	public class SimpleImageViewer : Control
	{
		public static readonly StyledProperty<IImage> SourceProperty = AvaloniaProperty.Register<SimpleImageViewer, IImage>(nameof(Source));
		public static readonly StyledProperty<BitmapInterpolationMode> InterpolationModeProperty = AvaloniaProperty.Register<SimpleImageViewer, BitmapInterpolationMode>(nameof(InterpolationMode), BitmapInterpolationMode.Default);

		public IImage Source
		{
			get { return GetValue(SourceProperty); }
			set { SetValue(SourceProperty, value); }
		}

		public BitmapInterpolationMode InterpolationMode
		{
			get { return GetValue(InterpolationModeProperty); }
			set { SetValue(InterpolationModeProperty, value); }
		}

		static SimpleImageViewer()
		{
			AffectsRender<SimpleImageViewer>(SourceProperty, InterpolationModeProperty);
		}

		public SimpleImageViewer()
		{
		}

		public override void Render(DrawingContext context)
		{
			if(Source == null) {
				return;
			}

			context.DrawImage(
				Source,
				new Rect(0, 0, (int)Source.Size.Width, (int)Source.Size.Height),
				new Rect(0, 0, Bounds.Width, Bounds.Height),
				InterpolationMode
			);
		}
	}
}
