using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Mesen.Interop;
using Mesen.Windows;
using System;

namespace Mesen.Controls
{
	public class KeyBindingButton : Button, IStyleable
	{
		Type IStyleable.StyleKey => typeof(Button);

		public static readonly StyledProperty<UInt32> KeyBindingProperty = AvaloniaProperty.Register<KeyBindingButton, UInt32>(nameof(KeyBinding), 0, false, Avalonia.Data.BindingMode.TwoWay);

		public UInt32 KeyBinding
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
			this.Content = InputApi.GetKeyName(this.KeyBinding);
		}

		protected override async void OnClick()
		{
			GetKeyWindow wnd = new GetKeyWindow();
			wnd.SingleKeyMode = true;
			wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
			await wnd.ShowDialog(this.VisualRoot as Window);
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

		protected override void OnPropertyChanged<T>(AvaloniaPropertyChangedEventArgs<T> change)
		{
			base.OnPropertyChanged(change);

			if(change.Property == KeyBindingProperty) {
				UInt32 value = Convert.ToUInt32(change.NewValue.Value);
				this.Content = InputApi.GetKeyName(value);
			}
		}
	}
}
