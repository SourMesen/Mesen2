using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Controls
{
	public class OptionSection : ItemsControl
	{
		public static readonly StyledProperty<string> HeaderProperty = AvaloniaProperty.Register<OptionSection, string>(nameof(Header));

		public string Header
		{
			get { return GetValue(HeaderProperty); }
			set { SetValue(HeaderProperty, value); }
		}

		public OptionSection()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
