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

namespace Mesen.Windows
{
	public class GetKeyWindow : Window
	{
		private DispatcherTimer _timer; 
		
		private List<UInt32> _prevScanCodes = new List<UInt32>();
		private TextBlock lblCurrentKey;
		public bool SingleKeyMode { get; set; } = false;
		
		public DbgShortKeys DbgShortcutKey { get; set; } = new DbgShortKeys();
		public KeyCombination ShortcutKey { get; set; } = new KeyCombination();

		public GetKeyWindow()
		{
			InitializeComponent();
			
			lblCurrentKey = this.FindControl<TextBlock>("lblCurrentKey");
			
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
			InputApi.SetKeyState((int)e.Key, true);
			DbgShortcutKey = new DbgShortKeys(e.KeyModifiers, e.Key);
			e.Handled = true;
		}

		private void OnPreviewKeyUp(object? sender, KeyEventArgs e)
		{
			InputApi.SetKeyState((int)e.Key, false);
			e.Handled = true;
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

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			InputApi.DisableAllKeys(false);
			DataContext = null;
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
			List<UInt32> scanCodes = InputApi.GetPressedKeys();

			if(this.SingleKeyMode) {
				if(scanCodes.Count >= 1) {
					//Always use the largest scancode (when multiple buttons are pressed at once)
					scanCodes = new List<UInt32> { scanCodes.OrderBy(code => -code).First() };
					this.SelectKeyCombination(new KeyCombination(scanCodes));
				}
			} else {
				KeyCombination key = new KeyCombination(_prevScanCodes);
				this.FindControl<TextBlock>("lblCurrentKey").Text = key.ToString();

				if(scanCodes.Count < _prevScanCodes.Count) {
					//Confirm key selection when the user releases a key
					this.SelectKeyCombination(key);
				}

				_prevScanCodes = scanCodes;
			}
		}
	}
}
