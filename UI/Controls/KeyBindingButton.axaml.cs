using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Avalonia.VisualTree;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Controls
{
	public class KeyBindingButton : Button, IStyleable
	{
		Type IStyleable.StyleKey => typeof(Button);

		public static readonly StyledProperty<UInt16> KeyBindingProperty = AvaloniaProperty.Register<KeyBindingButton, UInt16>(nameof(KeyBinding), 0, false, Avalonia.Data.BindingMode.TwoWay);

		public UInt16 KeyBinding
		{
			get { return GetValue(KeyBindingProperty); }
			set { SetValue(KeyBindingProperty, value); }
		}

		public KeyBindingButton()
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
			SetKeyName(this.KeyBinding);
		}

		protected override void OnKeyUp(KeyEventArgs e)
		{
			if(e.Key == Key.Space) {
				//Prevent using space to open up the configuration dialog
				//Otherwise the dialog opens up again when the user is trying
				//to bind the space key to a button
				e.Handled = true;
			} else {
				base.OnKeyUp(e);
			}
		}

		protected override async void OnClick()
		{
			GetKeyWindow wnd = new GetKeyWindow(true);
			wnd.SingleKeyMode = true;
			wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
			await wnd.ShowCenteredDialog(this.GetVisualRoot() as Visual);
			this.KeyBinding = wnd.ShortcutKey.Key1;
		}

		protected override void OnPointerReleased(PointerReleasedEventArgs e)
		{
			base.OnPointerReleased(e);

			//Allow using right mouse button to clear bindings
			if(e.InitialPressMouseButton == MouseButton.Right) {
				this.KeyBinding = 0;
			}
		}

		protected override void OnPropertyChanged(AvaloniaPropertyChangedEventArgs change)
		{
			base.OnPropertyChanged(change);

			if(change.Property == KeyBindingProperty) {
				UInt16 value = Convert.ToUInt16(change.NewValue);
				SetKeyName(value);
			}
		}

		private void SetKeyName(ushort value)
		{
			string keyname = InputApi.GetKeyName(value);
			this.Content = keyname;
			ToolTip.SetTip(this, string.IsNullOrWhiteSpace(keyname) ? null : keyname);
		}
	}
}
