using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Avalonia.VisualTree;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Controls
{
	public class MultiKeyBindingButton : Button
	{
		protected override Type StyleKeyOverride => typeof(Button);

		public static readonly StyledProperty<KeyCombination> KeyBindingProperty = AvaloniaProperty.Register<KeyBindingButton, KeyCombination>(nameof(KeyBinding), new KeyCombination(), false, Avalonia.Data.BindingMode.TwoWay);

		public KeyCombination KeyBinding
		{
			get { return GetValue(KeyBindingProperty); }
			set { SetValue(KeyBindingProperty, value); }
		}

		static MultiKeyBindingButton()
		{
			KeyBindingProperty.Changed.AddClassHandler<MultiKeyBindingButton>((x, e) => {
				x.SetKeyName();
			});
		}

		public MultiKeyBindingButton()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			SetKeyName();
		}

		protected override async void OnClick()
		{
			GetKeyWindow wnd = new GetKeyWindow(false);
			wnd.SingleKeyMode = false;
			wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
			await wnd.ShowCenteredDialog(this.GetVisualRoot() as Visual);
			this.KeyBinding = wnd.ShortcutKey;
		}

		protected override void OnPointerReleased(PointerReleasedEventArgs e)
		{
			base.OnPointerReleased(e);

			//Allow using right mouse button to clear bindings
			if(e.InitialPressMouseButton == MouseButton.Right) {
				this.KeyBinding = new KeyCombination();
			}
		}

		private void SetKeyName()
		{
			string keyname = KeyBinding.ToString();
			this.Content = keyname;
			KeyBindingButton.SetBindingButtonTooltip(this, keyname);
		}
	}
}
