using System;

namespace ThemeEditor.Controls.ColorPicker.Colors
{
	public readonly struct HSV
	{
		public double H { get; }

		public double S { get; }

		public double V { get; }

		public HSV(double h, double s, double v)
		{
			H = h;
			S = s;
			V = v;
		}

		public HSV(HSV hsv)
		{
			H = hsv.H;
			S = hsv.S;
			V = hsv.V;
		}

		public HSV(RGB rgb)
		{
			HSV hsv = rgb.ToHSV();
			H = hsv.H;
			S = hsv.S;
			V = hsv.V;
		}

		public HSV(CMYK cmyk)
		{
			HSV hsv = cmyk.ToHSV();
			H = hsv.H;
			S = hsv.S;
			V = hsv.V;
		}

		public HSV WithH(double h) => new HSV(h, S, V);

		public HSV WithS(double s) => new HSV(H, s, V);

		public HSV WithV(double v) => new HSV(H, S, v);

		public RGB ToRGB() => ToRGB(H, S, V);

		public CMYK ToCMYK() => ToCMYK(H, S, V);

		public static RGB ToRGB(double h, double s, double v)
		{
			double R = default;
			double G = default;
			double B = default;

			if(s == 0) {
				R = G = B = Math.Round(v * 2.55);
				return new RGB(R, G, B);
			}

			double hh = h;
			double ss = s / 100.0;
			double vv = v / 100.0;
			if(hh >= 360.0)
				hh = 0.0;
			hh /= 60.0;

			long i = (long)hh;
			double ff = hh - i;
			double p = vv * (1.0 - ss);
			double q = vv * (1.0 - ss * ff);
			double t = vv * (1.0 - ss * (1.0 - ff));

			switch((int)i) {
				case 0:
					R = vv;
					G = t;
					B = p;
					break;
				case 1:
					R = q;
					G = vv;
					B = p;
					break;
				case 2:
					R = p;
					G = vv;
					B = t;
					break;
				case 3:
					R = p;
					G = q;
					B = vv;
					break;
				case 4:
					R = t;
					G = p;
					B = vv;
					break;
				default:
					R = vv;
					G = p;
					B = q;
					break;
			}

			R = Math.Round(R * 255.0);
			G = Math.Round(G * 255.0);
			B = Math.Round(B * 255.0);

			return new RGB(R, G, B);
		}

		public static CMYK ToCMYK(double h, double s, double v)
		{
			return ToRGB(h, s, v).ToCMYK();
		}
	}
}
