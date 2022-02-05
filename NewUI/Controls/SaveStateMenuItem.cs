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
using System.IO;

namespace Mesen.Controls
{
	public class SaveStateMenuItem : MenuItem, IStyleable
	{
		Type IStyleable.StyleKey => typeof(MenuItem);

		public static readonly StyledProperty<int> SlotNumberProperty = AvaloniaProperty.Register<SaveStateMenuItem, int>(nameof(SlotNumber));
		public static readonly StyledProperty<bool> SaveModeProperty = AvaloniaProperty.Register<SaveStateMenuItem, bool>(nameof(SaveMode));

		public int SlotNumber
		{
			get { return GetValue(SlotNumberProperty); }
			set { SetValue(SlotNumberProperty, value); }
		}

		public bool SaveMode
		{
			get { return GetValue(SaveModeProperty); }
			set { SetValue(SaveModeProperty, value); }
		}

		public SaveStateMenuItem()
		{
		}

		protected override void OnInitialized()
		{
			if(Parent is MenuItem parent) {
				Action updateShortcut = () => {
					EmulatorShortcut shortcut = (EmulatorShortcut)((int)(SaveMode ? EmulatorShortcut.SaveStateSlot1 : EmulatorShortcut.LoadStateSlot1) + SlotNumber - 1);

					string statePath = Path.Combine(ConfigManager.SaveStateFolder, EmuApi.GetRomInfo().GetRomName() + "_" + SlotNumber + ".mss");
					bool isAutoSaveSlot = SlotNumber == 11;
					string slotName = isAutoSaveSlot ? "Auto" : SlotNumber.ToString();

					if(!File.Exists(statePath)) {
						Header = slotName + ". " + ResourceHelper.GetMessage("EmptyState");
					} else {
						DateTime dateTime = new FileInfo(statePath).LastWriteTime;
						Header = slotName + ". " + dateTime.ToShortDateString() + " " + dateTime.ToShortTimeString();
					}

					Tag = ShortcutMenuItem.GetShortcutKeys(shortcut)?.ToString() ?? "";
					IsEnabled = EmuApi.IsShortcutAllowed(shortcut, 0);
				};

				updateShortcut();

				//Update item shortcut text when its parent opens
				parent.SubmenuOpened += (object? sender, RoutedEventArgs e) => {
					updateShortcut();
				};

				Click += (object? sender, RoutedEventArgs e) => {
					if(SaveMode) {
						EmuApi.SaveState((uint)SlotNumber);
					} else {
						EmuApi.LoadState((uint)SlotNumber);
					}
				};
			}
		}
	}
}
