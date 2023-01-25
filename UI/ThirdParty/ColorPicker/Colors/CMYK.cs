using System;

namespace ThemeEditor.Controls.ColorPicker.Colors
{
	public readonly struct CMYK
	{
		public double C { get; }

		public double M { get; }

		public double Y { get; }

		public double K { get; }

		public CMYK(double c, double m, double y, double k)
		{
			C = c;
			M = m;
			Y = y;
			K = k;
		}

		public CMYK(CMYK cmyk)
		{
			C = cmyk.C;
			M = cmyk.M;
			Y = cmyk.Y;
			K = cmyk.K;
		}

		public CMYK(RGB rgb)
		{
			CMYK cmyk = rgb.ToCMYK();
			C = cmyk.C;
			M = cmyk.M;
			Y = cmyk.Y;
			K = cmyk.K;
		}

		public CMYK(HSV hsv)
		{
			CMYK cmyk = hsv.ToCMYK();
			C = cmyk.C;
			M = cmyk.M;
			Y = cmyk.Y;
			K = cmyk.K;
		}

		public CMYK WithC(double c) => new CMYK(c, M, Y, K);

		public CMYK WithM(double m) => new CMYK(C, m, Y, K);

		public CMYK WithY(double y) => new CMYK(C, M, y, K);

		public CMYK WithK(double k) => new CMYK(C, M, Y, k);

		public RGB ToRGB() => ToRGB(C, M, Y, K);

		public HSV ToHSV() => ToHSV(C, M, Y, K);

		public static RGB ToRGB(double c, double m, double y, double k)
		{
			double R = default;
			double G = default;
			double B = default;

			double cc = c / 100.0;
			double mm = m / 100.0;
			double yy = y / 100.0;
			double kk = k / 100.0;

			R = (1.0 - cc) * (1.0 - kk);
			G = (1.0 - mm) * (1.0 - kk);
			B = (1.0 - yy) * (1.0 - kk);

			R = Math.Round(R * 255.0);
			G = Math.Round(G * 255.0);
			B = Math.Round(B * 255.0);

			return new RGB(R, G, B);
		}

		public static HSV ToHSV(double c, double m, double y, double k)
		{
			return ToRGB(c, m, y, k).ToHSV();
		}
	}
}
