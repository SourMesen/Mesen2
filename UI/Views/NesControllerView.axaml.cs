using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Views
{
	public class NesControllerView : UserControl
	{
		public bool ShowMicrophoneButton { get; }

		public NesControllerView() : this(false)
		{
		}

		public NesControllerView(bool showMicrophoneButton)
		{
			ShowMicrophoneButton = showMicrophoneButton;

			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
