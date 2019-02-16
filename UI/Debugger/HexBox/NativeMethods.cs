using System;
using System.Drawing;
using System.Runtime.InteropServices;

namespace Be.Windows.Forms
{
	internal static class NativeMethods
	{
		// Key definitions
		public const int WM_KEYDOWN = 0x100;
		public const int WM_KEYUP = 0x101;
		public const int WM_CHAR = 0x102;
	}
}
