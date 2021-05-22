using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Mesen.Config.Shortcuts;
using Avalonia.LogicalTree;
using Mesen.Interop;
using Avalonia.Styling;

namespace Mesen.Controls
{
	public class ShortcutMenuItem : MenuItem, IStyleable
	{
		Type IStyleable.StyleKey => typeof(MenuItem);

		public static readonly StyledProperty<EmulatorShortcut> ShortcutProperty = AvaloniaProperty.Register<ShortcutMenuItem, EmulatorShortcut>(nameof(Shortcut));
		public static readonly StyledProperty<int> ShortcutParamProperty = AvaloniaProperty.Register<ShortcutMenuItem, int>(nameof(ShortcutParam));

		public EmulatorShortcut Shortcut
		{
			get { return GetValue(ShortcutProperty); }
			set { SetValue(ShortcutProperty, value); }
		}

		public int ShortcutParam
		{
			get { return GetValue(ShortcutParamProperty); }
			set { SetValue(ShortcutParamProperty, value); }
		}

		public ShortcutMenuItem()
		{
		}

		public static KeyCombination? GetShortcutKeys(EmulatorShortcut shortcut)
		{
			PreferencesConfig cfg = ConfigManager.Config.Preferences;
			int keyIndex = cfg.ShortcutKeys.FindIndex((ShortcutKeyInfo shortcutInfo) => shortcutInfo.Shortcut == shortcut);
			if(keyIndex >= 0) {
				if(!cfg.ShortcutKeys[keyIndex].KeyCombination.IsEmpty) {
					return cfg.ShortcutKeys[keyIndex].KeyCombination;
				} else if(!cfg.ShortcutKeys[keyIndex].KeyCombination.IsEmpty) {
					return cfg.ShortcutKeys[keyIndex].KeyCombination2;
				}
			}
			return null;
		}

		protected override void OnInitialized()
		{
			if(Parent is MenuItem parent) {
				Action updateShortcut = () => {
					Tag = GetShortcutKeys(Shortcut)?.ToString() ?? "";
					IsEnabled = EmuApi.IsShortcutAllowed(Shortcut);
				};

				updateShortcut();

				//Update item shortcut text when its parent opens
				parent.SubmenuOpened += (object? sender, RoutedEventArgs e) => {
					updateShortcut();
				};

				Click += (object? sender, RoutedEventArgs e) => {
					EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = Shortcut, Param = (uint)ShortcutParam });
				};
			}
		}
	}
}
