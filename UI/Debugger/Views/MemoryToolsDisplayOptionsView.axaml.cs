using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Debugger.Views
{
	public class MemoryToolsDisplayOptionsView : UserControl
	{
		public MemoryToolsDisplayOptionsView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
