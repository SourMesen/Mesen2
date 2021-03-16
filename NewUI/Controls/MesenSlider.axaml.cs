using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Controls
{
	public class MesenSlider : UserControl
	{
		public static readonly StyledProperty<int> MinimumProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(Minimum));
		public static readonly StyledProperty<int> MaximumProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(Maximum));
		public static readonly StyledProperty<int> ValueProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(Value));
		public static readonly StyledProperty<string> TextProperty = AvaloniaProperty.Register<MesenSlider, string>(nameof(Text));

		public int Minimum
		{
			get { return GetValue(MinimumProperty); }
			set { SetValue(MinimumProperty, value); }
		}

		public int Maximum
		{
			get { return GetValue(MaximumProperty); }
			set { SetValue(MaximumProperty, value); }
		}

		public int Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}
		
		public string Text
		{
			get { return GetValue(TextProperty); }
			set { SetValue(TextProperty, value); }
		}

		public MesenSlider()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
