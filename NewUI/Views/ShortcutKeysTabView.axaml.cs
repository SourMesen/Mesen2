using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Config;

namespace Mesen.Views
{
	public class ShortcutKeysTabView : UserControl
	{
		public static readonly StyledProperty<Thickness> HeaderMarginProperty = AvaloniaProperty.Register<ShortcutKeysTabView, Thickness>(nameof(HeaderMargin), new Thickness(5, 5, 16, 5));
	
		public Thickness HeaderMargin
		{
			get { return GetValue(HeaderMarginProperty); }
			set { SetValue(HeaderMarginProperty, value); }
		}

		public ShortcutKeysTabView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
