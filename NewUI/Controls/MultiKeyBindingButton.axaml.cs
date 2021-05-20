using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Controls
{
	public class MultiKeyBindingButton : Button, IStyleable
	{
		Type IStyleable.StyleKey => typeof(Button);

		public static readonly StyledProperty<KeyCombination> KeyBindingProperty = AvaloniaProperty.Register<KeyBindingButton, KeyCombination>(nameof(KeyBinding), new KeyCombination(), false, Avalonia.Data.BindingMode.TwoWay);

		public KeyCombination KeyBinding
		{
			get { return GetValue(KeyBindingProperty); }
			set { SetValue(KeyBindingProperty, value); }
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
			this.Content = this.KeyBinding.ToString();
		}

		protected override async void OnClick()
		{
			GetKeyWindow wnd = new GetKeyWindow();
			wnd.SingleKeyMode = false;
			wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
			await wnd.ShowCenteredDialog((Window)this.VisualRoot);
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

		protected override void OnPropertyChanged<T>(AvaloniaPropertyChangedEventArgs<T> change)
		{
			base.OnPropertyChanged(change);

			if(change.Property == KeyBindingProperty) {
				this.Content = change.NewValue.Value.ToString();
			}
		}
	}
}
