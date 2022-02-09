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
	public class ButtonWithIcon : Button, IStyleable
	{
		Type IStyleable.StyleKey => typeof(Button);

		public static readonly StyledProperty<string> TextProperty = AvaloniaProperty.Register<KeyBindingButton, string>(nameof(Text), "");
		public static readonly StyledProperty<string> IconProperty = AvaloniaProperty.Register<KeyBindingButton, string>(nameof(Icon), "");

		public string Text
		{
			get { return GetValue(TextProperty); }
			set { SetValue(TextProperty, value); }
		}

		public string Icon
		{
			get { return GetValue(IconProperty); }
			set { SetValue(IconProperty, value); }
		}

		static ButtonWithIcon()
		{
			IconProperty.Changed.AddClassHandler<ButtonWithIcon>((x, e) => {
				x.FindControl<Image>("IconImage").Source = ImageUtilities.BitmapFromAsset(x.Icon);
			});
		}

		public ButtonWithIcon()
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
