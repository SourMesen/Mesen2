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
		public static readonly StyledProperty<IBrush> HintColorProperty = AvaloniaProperty.Register<CheckBoxWarning, IBrush>(nameof(HintColor), Brushes.DarkGray);

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

		public IBrush HintColor
		{
			get { return GetValue(HintColorProperty); }
			set { SetValue(HintColorProperty, value); }
		}

		public CheckBoxWarning()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnPropertyChanged<T>(AvaloniaPropertyChangedEventArgs<T> change)
		{
			base.OnPropertyChanged(change);

			if(change.Property == IsCheckedProperty) {
				this.HintColor = this.IsChecked ? Brushes.Red : Brushes.DarkGray;
			}
		}
	}
}
