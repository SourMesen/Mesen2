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
		public int[] AvailableWidths => new int[] { 4, 8, 16, 32, 48, 64, 80, 96, 112, 128 };

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
