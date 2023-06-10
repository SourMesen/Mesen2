using Avalonia;
using Avalonia.Animation;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Avalonia.VisualTree;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Debugger.Controls
{
	public class DebuggerKeyBindingButton : Button
	{
		protected override Type StyleKeyOverride => typeof(Button);

		public static readonly StyledProperty<DbgShortKeys> KeyBindingProperty = AvaloniaProperty.Register<DebuggerKeyBindingButton, DbgShortKeys>(nameof(KeyBinding), new DbgShortKeys(), false, Avalonia.Data.BindingMode.TwoWay);

		public DbgShortKeys KeyBinding
		{
			get { return GetValue(KeyBindingProperty); }
			set { SetValue(KeyBindingProperty, value); }
		}

		static DebuggerKeyBindingButton()
		{
			KeyBindingProperty.Changed.AddClassHandler<DebuggerKeyBindingButton>((x, e) => x.SetKeyName(x.KeyBinding.ToString()));
		}

		public DebuggerKeyBindingButton()
		{
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			SetKeyName(this.KeyBinding.ToString());
		}

		protected override async void OnClick()
		{
			GetKeyWindow wnd = new GetKeyWindow(true);
			wnd.SingleKeyMode = false;
			wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
			await wnd.ShowCenteredDialog(this.GetVisualRoot() as Visual);
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

		private void SetKeyName(string keyname)
		{
			this.Content = keyname;
			ToolTip.SetTip(this, string.IsNullOrWhiteSpace(keyname) ? null : keyname);
		}
	}
}
