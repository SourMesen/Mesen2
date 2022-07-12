using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media;

namespace Mesen.Controls
{
	public class CheckBoxWarning : UserControl
	{
		public static readonly StyledProperty<bool> IsCheckedProperty = AvaloniaProperty.Register<CheckBoxWarning, bool>(nameof(IsChecked), false, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<string> TextProperty = AvaloniaProperty.Register<CheckBoxWarning, string>(nameof(Text));

		public bool IsChecked
		{
			get { return GetValue(IsCheckedProperty); }
			set { SetValue(IsCheckedProperty, value); }
		}

		public string Text
		{
			get { return GetValue(TextProperty); }
			set { SetValue(TextProperty, value); }
		}

		public CheckBoxWarning()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
