using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using System;

namespace Mesen.Utilities
{
	public static class ImageUtilities
	{
		public static Image FromAsset(string source)
		{
			return new Image() { Source = new Bitmap(AssetLoader.Open(new Uri("avares://Mesen/" + source))) };
		}

		public static Bitmap BitmapFromAsset(string source)
		{
			return new Bitmap(AssetLoader.Open(new Uri("avares://Mesen/" + source)));
		}
	}
}
