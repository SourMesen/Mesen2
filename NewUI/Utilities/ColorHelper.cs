using Avalonia.Media;
using Mesen.Config;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class ColorHelper
	{
		public static HslColor RgbToHsl(Color rgbColor)
		{
			double r = rgbColor.R / 255.0;
			double g = rgbColor.G / 255.0;
			double b = rgbColor.B / 255.0;

			double max = Math.Max(r, Math.Max(g, b));
			double min = Math.Min(r, Math.Min(g, b));

			double c = max - min; //Chroma

			HslColor hsl;
			hsl.L = (max + min) / 2;
			
			if(Math.Abs(c) < 0.00001) {
				hsl.S = 0;
				hsl.H = 0;
			} else {
				if(hsl.L < 0.0001 || hsl.L > 0.9999) {
					hsl.S = 0;
				} else {
					hsl.S = (max - hsl.L) / Math.Min(hsl.L, 1 - hsl.L);
				}

				if(r == max) {
					hsl.H = 0 + ((g - b) / c);
				} else if(g == max) {
					hsl.H = 2 + ((b - r) / c);
				} else {
					hsl.H = 4 + ((r - g) / c);
				}

				hsl.H *= 60;
				if(hsl.H < 0) {
					hsl.H += 360;
				}
			}

			return hsl;
		}

		public static Color HslToRgb(HslColor hsl)
		{
			double c = (1 - Math.Abs(2 * hsl.L - 1)) * hsl.S;
			double hp = hsl.H / 60.0;
			double x = c * (1 - Math.Abs((hp % 2) - 1));

			double r = 0;
			double g = 0;
			double b = 0;

			if(hp <= 1) {
				r = c; g = x; b = 0;
			} else if(hp <= 2) {
				r = x; g = c; b = 0;
			} else if(hp <= 3) {
				r = 0; g = c; b = x;
			} else if(hp <= 4) {
				r = 0; g = x; b = c;
			} else if(hp <= 5) {
				r = x; g = 0; b = c;
			} else if(hp <= 6) {
				r = c; g = 0; b = x;
			}

			double m = hsl.L - c * 0.5;
			return Color.FromRgb(
				(byte)Math.Round(255 * (r + m)),
				(byte)Math.Round(255 * (g + m)),
				(byte)Math.Round(255 * (b + m))
			);
		}

		public static Color InvertBrightness(Color color)
		{
			HslColor hsl = RgbToHsl(color);
			
			if(hsl.L >= 0.3 && hsl.L < 0.6) {
				hsl.L -= 0.2;
			}
			
			hsl.L = 1 - hsl.L;

			if(hsl.L < 0.1) {
				hsl.L += 0.05;
			} else if(hsl.L > 0.9) {
				hsl.L -= 0.05;
			}

			return HslToRgb(hsl);
		}

		public static SolidColorBrush GetBrush(Color color)
		{
			if(ConfigManager.Config.Preferences.Theme == MesenTheme.Dark) {
				return new SolidColorBrush(InvertBrightness(color));
			} else {
				return new SolidColorBrush(color);
			}
		}

		public static SolidColorBrush GetBrush(SolidColorBrush b)
		{
			if(ConfigManager.Config.Preferences.Theme == MesenTheme.Dark) {
				return new SolidColorBrush(InvertBrightness(b.Color));
			} else {
				return new SolidColorBrush(b.Color);
			}
		}

		public static Pen GetPen(Color color)
		{
			return new Pen(GetBrush(color));
		}

		public static Color GetColor(Color color)
		{
			if(ConfigManager.Config.Preferences.Theme == MesenTheme.Dark) {
				return InvertBrightness(color);
			} else {
				return color;
			}
		}
	}

	public struct HslColor
	{
		public double H;
		public double S;
		public double L;
	}
}
