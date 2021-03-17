using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;
using Mesen.GUI;

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

		protected override void OnInitialized()
		{
			base.OnInitialized();

			if(Design.IsDesignMode) {
				return;
			}

			this.FindControl<ComboBox>("AudioDevice").Items = ConfigApi.GetAudioDevices();
		}
	}
}
