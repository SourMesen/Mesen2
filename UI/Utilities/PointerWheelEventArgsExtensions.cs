using Avalonia.Input;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities;

static class PointerWheelEventArgsExtensions
{
	public static double GetDeltaY(this PointerWheelEventArgs e)
	{
		if(OperatingSystem.IsWindows() || OperatingSystem.IsMacOS()) {
			return e.Delta.Y;
		}

		if(Math.Abs(e.Delta.Y) > 8) {
			//Avalonia currently seems to have an issue with mouse wheel events on Linux
			//Alt-tabbing to another application, scrolling, alt-tabbing back, and then
			//trying to scroll will return a large delta value that includes the amount
			//of scrolling done in the other application. In this case, return 0.
			return 0;
		}
		return e.Delta.Y;
	}
}
