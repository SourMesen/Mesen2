using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Controls
{
	public class OptionSection : ItemsControl
	{
		public static readonly StyledProperty<string> HeaderProperty = AvaloniaProperty.Register<OptionSection, string>(nameof(Header));
		public static readonly StyledProperty<object> ButtonProperty = AvaloniaProperty.Register<OptionSection, object>(nameof(Button));

		public string Header
		{
			get { return GetValue(HeaderProperty); }
			set { SetValue(HeaderProperty, value); }
		}

		public object Button
		{
			get { return GetValue(ButtonProperty); }
			set { SetValue(ButtonProperty, value); }
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
