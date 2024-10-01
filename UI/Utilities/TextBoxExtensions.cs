using Avalonia.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities;

public static class TextBoxExtensions
{
	public static void FocusAndSelectAll(this TextBox txt)
	{
		txt.Focus();
		txt.SelectAll();
	}
}
