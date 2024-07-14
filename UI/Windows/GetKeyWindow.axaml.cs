using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Interactivity;
using System;
using Mesen.Config.Shortcuts;
using System.Collections.Generic;
using System.Linq;
using Avalonia.Input;
using System.ComponentModel;
using Mesen.Interop;
using Avalonia.Threading;
using Avalonia;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.Localization;
using Avalonia.Layout;
using System.Diagnostics;

namespace Mesen.Windows
{
	public class GetKeyWindow : MesenWindow
	{
		private DispatcherTimer _timer; 
		
		private List<UInt16> _prevScanCodes = new List<UInt16>();
		private TextBlock lblCurrentKey;
		private bool _allowKeyboardOnly;

		private Stopwatch _stopWatch = Stopwatch.StartNew();
		private Dictionary<Key, long> _keyPressedStamp = new();

		public string HintLabel { get; }
		public bool SingleKeyMode { get; set; } = false;
		
		public DbgShortKeys DbgShortcutKey { get; set; } = new DbgShortKeys();
		public KeyCombination ShortcutKey { get; set; } = new KeyCombination();

		[Obsolete("For designer only")]
		public GetKeyWindow() : this(false) { }

		public GetKeyWindow(bool allowKeyboardOnly)
		{
			_allowKeyboardOnly = allowKeyboardOnly;
			HintLabel = ResourceHelper.GetMessage(_allowKeyboardOnly ? "SetKeyHint" : "SetKeyMouseHint");

			//Required for keyboard input to work properly in Linux/macOS
			this.Focusable = true;

			InitializeComponent();

			lblCurrentKey = this.GetControl<TextBlock>("lblCurrentKey");
			
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(25), DispatcherPriority.Normal, (s, e) => UpdateKeyDisplay());
			_timer.Start();

			//Allow us to catch LeftAlt/RightAlt key presses
			this.AddHandler(InputElement.KeyDownEvent, OnPreviewKeyDown, RoutingStrategies.Tunnel, true);
			this.AddHandler(InputElement.KeyUpEvent, OnPreviewKeyUp, RoutingStrategies.Tunnel, true);
		}

		protected override void OnClosed(EventArgs e)
		{
			_timer?.Stop();
			base.OnClosed(e);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnPreviewKeyDown(object? sender, KeyEventArgs e)
		{
			InputApi.SetKeyState((UInt16)e.Key, true);
			DbgShortcutKey = new DbgShortKeys(e.KeyModifiers, e.Key);
			_keyPressedStamp[e.Key] = _stopWatch.ElapsedTicks;
			e.Handled = true;
		}

		private void OnPreviewKeyUp(object? sender, KeyEventArgs e)
		{
			e.Handled = true;

			if(e.Key.IsSpecialKey() && !_allowKeyboardOnly && (!_keyPressedStamp.TryGetValue(e.Key, out long stamp) || ((_stopWatch.ElapsedTicks - stamp) * 1000 / Stopwatch.Frequency) < 10)) {
				//Key up received without key down, or key pressed for less than 10 ms, pretend the key was pressed for 50ms
				//Some special keys can behave this way (e.g printscreen)
				DbgShortcutKey = new DbgShortKeys(e.KeyModifiers, e.Key);
				InputApi.SetKeyState((UInt16)e.Key, true);
				UpdateKeyDisplay();
				DispatcherTimer.RunOnce(() => InputApi.SetKeyState((UInt16)e.Key, false), TimeSpan.FromMilliseconds(50), DispatcherPriority.MaxValue);
				_keyPressedStamp.Remove(e.Key);
				return;
			}

			_keyPressedStamp.Remove(e.Key);

			InputApi.SetKeyState((UInt16)e.Key, false);
			if(_allowKeyboardOnly) {
				this.Close();
			}
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			lblCurrentKey.IsVisible = !this.SingleKeyMode;
			lblCurrentKey.Height = this.SingleKeyMode ? 0 : 40;

			ShortcutKey = new KeyCombination();
			InputApi.UpdateInputDevices();
			InputApi.ResetKeyState();

			//Prevent other keybindings from interfering/activating
			InputApi.DisableAllKeys(true);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			InputApi.DisableAllKeys(false);
		}

		private void SelectKeyCombination(KeyCombination key)
		{
			if(!string.IsNullOrWhiteSpace(key.ToString())) {
				ShortcutKey = key;
				this.Close();
			}
		}

		private void UpdateKeyDisplay()
		{
			if(!_allowKeyboardOnly) {
				SystemMouseState mouseState = InputApi.GetSystemMouseState(IntPtr.Zero);
				PixelPoint mousePos = new PixelPoint(mouseState.XPosition, mouseState.YPosition);
				PixelRect clientBounds = new PixelRect(this.PointToScreen(new Point(0, 0)), PixelSize.FromSize(Bounds.Size, LayoutHelper.GetLayoutScale(this) / InputApi.GetPixelScale()));
				bool mouseInsideWindow = clientBounds.Contains(mousePos);
				InputApi.SetKeyState(MouseManager.LeftMouseButtonKeyCode, mouseInsideWindow && mouseState.LeftButton);
				InputApi.SetKeyState(MouseManager.RightMouseButtonKeyCode, mouseInsideWindow && mouseState.RightButton);
				InputApi.SetKeyState(MouseManager.MiddleMouseButtonKeyCode, mouseInsideWindow && mouseState.MiddleButton);
				InputApi.SetKeyState(MouseManager.MouseButton4KeyCode, mouseInsideWindow && mouseState.Button4);
				InputApi.SetKeyState(MouseManager.MouseButton5KeyCode, mouseInsideWindow && mouseState.Button5);

				List<UInt16> scanCodes = InputApi.GetPressedKeys();

				if(this.SingleKeyMode) {
					if(scanCodes.Count >= 1) {
						//Always use the largest scancode (when multiple buttons are pressed at once)
						scanCodes = new List<UInt16> { scanCodes.OrderBy(code => -code).First() };
						this.SelectKeyCombination(new KeyCombination(scanCodes));
					}
				} else {
					KeyCombination key = new KeyCombination(_prevScanCodes);
					this.GetControl<TextBlock>("lblCurrentKey").Text = key.ToString();

					if(scanCodes.Count < _prevScanCodes.Count) {
						//Confirm key selection when the user releases a key
						this.SelectKeyCombination(key);
					}

					_prevScanCodes = scanCodes;
				}
			} else {
				this.GetControl<TextBlock>("lblCurrentKey").Text = DbgShortcutKey.ToString();
			}
		}
	}
}
