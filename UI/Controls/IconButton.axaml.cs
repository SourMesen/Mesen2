using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Controls
{
	public class IconButton : Button
	{
		protected override Type StyleKeyOverride => typeof(Button);

		public static readonly StyledProperty<string> IconProperty = AvaloniaProperty.Register<KeyBindingButton, string>(nameof(Icon), "");

		public string Icon
		{
			get { return GetValue(IconProperty); }
			set { SetValue(IconProperty, value); }
		}

		static IconButton()
		{
			IconProperty.Changed.AddClassHandler<IconButton>((x, e) => {
				x.GetControl<Image>("IconImage").Source = ImageUtilities.BitmapFromAsset(x.Icon);
			});
		}

		public IconButton()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnClick()
		{
			base.OnClick();
			ToolTip.SetIsOpen(this, false);
		}
	}
}
