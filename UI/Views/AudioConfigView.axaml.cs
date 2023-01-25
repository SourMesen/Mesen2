using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Interop;

namespace Mesen.Views
{
	public class AudioConfigView : UserControl
	{
		public AudioConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
