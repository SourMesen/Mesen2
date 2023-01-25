using Avalonia;
using Avalonia.Controls;
using Avalonia.Data.Converters;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Interop;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Debugger.Views
{
	public class DebuggerOptionsView : UserControl
	{
		public DebuggerOptionsView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
