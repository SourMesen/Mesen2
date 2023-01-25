using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Debugger.Controls
{
	public class SpritePreviewPanel : UserControl
	{
		public SpritePreviewPanel()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
