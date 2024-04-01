using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Debugger.Views
{
	public class GbaEventViewerConfigView : UserControl
	{
		public GbaEventViewerConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
