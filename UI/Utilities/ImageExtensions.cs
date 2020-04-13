using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Utilities
{
	static class ImageExtensions
	{
		public static Image GetScaledImage(this Image img, double scale)
		{
			int newWidth = (int)(img.Width * scale);
			int newHeight = (int)(img.Height * scale);

			Bitmap scaledImg = new Bitmap(newWidth, newHeight);
			using(Graphics g = Graphics.FromImage(scaledImg)) {
				g.InterpolationMode = scale >= 2 ? InterpolationMode.NearestNeighbor : InterpolationMode.HighQualityBicubic;
				g.DrawImage(img, 0, 0, newWidth, newHeight);
			}

			return scaledImg;
		}
	}
}
