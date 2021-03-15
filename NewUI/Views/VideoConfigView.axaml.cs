using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;

namespace Mesen.Views
{
	public class VideoConfigView : UserControl
	{
		public VideoConfigView()
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

			DataBindingHelper.InitializeComboBox(this.FindControl<ComboBox>("cboAspectRatio"), typeof(VideoAspectRatio));
		}
	}
}
