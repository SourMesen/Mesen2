using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Controls
{
	public class SectionHeader : UserControl
	{
		public SectionHeader()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
