using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config.Shortcuts;
using System.Collections.Generic;

namespace Mesen.Views
{
	public class ShortcutKeysTabView : UserControl
	{
		public static readonly StyledProperty<Thickness> HeaderMarginProperty = AvaloniaProperty.Register<ShortcutKeysTabView, Thickness>(nameof(HeaderMargin), new Thickness(5, 5, 16, 5));
		public static readonly StyledProperty<List<ShortcutKeyInfo>> ShortcutKeysProperty = AvaloniaProperty.Register<ShortcutKeysTabView, List<ShortcutKeyInfo>>(nameof(ShortcutKeys), new List<ShortcutKeyInfo>());

		public Thickness HeaderMargin
		{
			get { return GetValue(HeaderMarginProperty); }
			set { SetValue(HeaderMarginProperty, value); }
		}

		public List<ShortcutKeyInfo> ShortcutKeys
		{
			get { return GetValue(ShortcutKeysProperty); }
			set { SetValue(ShortcutKeysProperty, value); }
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
