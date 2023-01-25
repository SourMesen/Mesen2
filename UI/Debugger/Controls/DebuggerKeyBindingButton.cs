using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Debugger.Controls
{
	public class DebuggerKeyBindingButton : Button, IStyleable
	{
		Type IStyleable.StyleKey => typeof(Button);

		public static readonly StyledProperty<DbgShortKeys> KeyBindingProperty = AvaloniaProperty.Register<DebuggerKeyBindingButton, DbgShortKeys>(nameof(KeyBinding), new DbgShortKeys(), false, Avalonia.Data.BindingMode.TwoWay);

		public DbgShortKeys KeyBinding
		{
			get { return GetValue(KeyBindingProperty); }
			set { SetValue(KeyBindingProperty, value); }
		}

		static DebuggerKeyBindingButton()
		{
			KeyBindingProperty.Changed.AddClassHandler<DebuggerKeyBindingButton>((x, e) => x.Content = x.KeyBinding.ToString());
		}

		public DebuggerKeyBindingButton()
		{
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			this.Content = this.KeyBinding.ToString();
		}

		protected override async void OnClick()
		{
			GetKeyWindow wnd = new GetKeyWindow(false);
			wnd.SingleKeyMode = false;
			wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
			await wnd.ShowCenteredDialog(VisualRoot);
			this.KeyBinding = wnd.DbgShortcutKey;
		}

		protected override void OnPointerReleased(PointerReleasedEventArgs e)
		{
			base.OnPointerReleased(e);

			//Allow using right mouse button to clear bindings
			if(e.InitialPressMouseButton == MouseButton.Right) {
				this.KeyBinding = new DbgShortKeys();
			}
		}
	}
}
