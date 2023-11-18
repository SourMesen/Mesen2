using Avalonia.Controls;
using Avalonia.VisualTree;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	static class ControlExtensions
	{
		public static bool IsParentWindowFocused(this Control ctrl)
		{
			return (ctrl.GetVisualRoot() as WindowBase)?.IsKeyboardFocusWithin == true;
		}
	}
}
