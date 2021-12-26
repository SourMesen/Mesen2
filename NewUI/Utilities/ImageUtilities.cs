using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using System;

namespace Mesen.Utilities
{
	public static class ImageUtilities
	{
		public static Image? FromAsset(string source)
		{
			IAssetLoader? assetLoader = AvaloniaLocator.Current.GetService<IAssetLoader>();
			if(assetLoader != null) {
				return new Image() { Source = new Bitmap(assetLoader.Open(new Uri("avares://Mesen/" + source))) };
			} else {
				return null;
			}
		}
	}
}
