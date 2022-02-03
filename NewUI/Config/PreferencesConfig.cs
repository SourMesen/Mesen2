using Mesen.Localization;
using Mesen.Config.Shortcuts;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using ReactiveUI.Fody.Helpers;
using Avalonia.Styling;
using Avalonia;
using Avalonia.Markup.Xaml.Styling;
using Avalonia.Themes.Fluent;
using Mesen.Interop;

namespace Mesen.Config
{
	public class PreferencesConfig : BaseConfig<PreferencesConfig>
	{
		[Reactive] public MesenTheme Theme { get; set; } = MesenTheme.Light;
		[Reactive] public Language DisplayLanguage { get; set; } = Language.English;
		[Reactive] public bool AutomaticallyCheckForUpdates { get; set; } = true;
		[Reactive] public bool SingleInstance { get; set; } = true;
		[Reactive] public bool AutoLoadPatches { get; set; } = true;

		[Reactive] public bool PauseWhenInBackground { get; set; } = false;
		[Reactive] public bool PauseWhenInMenusAndConfig { get; set; } = false;
		[Reactive] public bool AllowBackgroundInput { get; set; } = false;
		[Reactive] public bool PauseOnMovieEnd { get; set; } = true;
		[Reactive] public bool ShowMovieIcons { get; set; } = true;

		[Reactive] public bool AssociateSnesRomFiles { get; set; } = false;
		[Reactive] public bool AssociateSnesMusicFiles { get; set; } = false;
		[Reactive] public bool AssociateNesRomFiles { get; set; } = false;
		[Reactive] public bool AssociateNesMusicFiles { get; set; } = false;
		[Reactive] public bool AssociateGbRomFiles { get; set; } = false;
		[Reactive] public bool AssociateGbMusicFiles { get; set; } = false;
		[Reactive] public bool AssociateMovieFiles { get; set; } = false;
		[Reactive] public bool AssociateSaveStateFiles { get; set; } = false;

		[Reactive] public bool EnableAutoSaveState { get; set; } = true;
		[Reactive] public UInt32 AutoSaveStateDelay { get; set; } = 5;

		[Reactive] public bool EnableRewind { get; set; } = true;
		[Reactive] public UInt32 RewindBufferSize { get; set; } = 30;

		[Reactive] public bool AlwaysOnTop { get; set; } = false;
		[Reactive] public bool AutoHideMenu { get; set; } = false;

		[Reactive] public bool ShowFps { get; set; } = false;
		[Reactive] public bool ShowFrameCounter { get; set; } = false;
		[Reactive] public bool ShowGameTimer { get; set; } = false;
		[Reactive] public bool ShowTitleBarInfo { get; set; } = false;
		[Reactive] public bool ShowDebugInfo { get; set; } = false;
		[Reactive] public bool DisableOsd { get; set; } = false;
		[Reactive] public bool DisableGameSelectionScreen { get; set; } = false;

		[Reactive] public List<ShortcutKeyInfo> ShortcutKeys { get; set; } = new List<ShortcutKeyInfo>();

		[Reactive] public bool OverrideGameFolder { get; set; } = false;
		[Reactive] public bool OverrideAviFolder { get; set; } = false;
		[Reactive] public bool OverrideMovieFolder { get; set; } = false;
		[Reactive] public bool OverrideSaveDataFolder { get; set; } = false;
		[Reactive] public bool OverrideSaveStateFolder { get; set; } = false;
		[Reactive] public bool OverrideScreenshotFolder { get; set; } = false;
		[Reactive] public bool OverrideWaveFolder { get; set; } = false;

		[Reactive] public string GameFolder { get; set; } = "";
		[Reactive] public string AviFolder { get; set; } = "";
		[Reactive] public string MovieFolder { get; set; } = "";
		[Reactive] public string SaveDataFolder { get; set; } = "";
		[Reactive] public string SaveStateFolder { get; set; } = "";
		[Reactive] public string ScreenshotFolder { get; set; } = "";
		[Reactive] public string WaveFolder { get; set; } = "";

		public PreferencesConfig()
		{
		}

		private void AddShortcut(ShortcutKeyInfo shortcut)
		{
			if(!ShortcutKeys.Exists(a => a.Shortcut == shortcut.Shortcut)) {
				ShortcutKeys.Add(shortcut);
			}
		}

		public void InitializeDefaultShortcuts()
		{
			uint ctrl = InputApi.GetKeyCode("Left Ctrl");
			uint alt = InputApi.GetKeyCode("Left Alt");
			uint shift = InputApi.GetKeyCode("Left Shift");

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.FastForward, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("Tab") }, KeyCombination2 = new KeyCombination() { Key1 = InputApi.GetKeyCode("Pad1 R2") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.Rewind, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("Backspace") }, KeyCombination2 = new KeyCombination() { Key1 = InputApi.GetKeyCode("Pad1 L2") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.IncreaseSpeed, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("=") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.DecreaseSpeed, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("-") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.MaxSpeed, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F9") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.IncreaseVolume, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("=") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.DecreaseVolume, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("-") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.ToggleFps, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F10") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.ToggleFullscreen, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F11") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.TakeScreenshot, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F12") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.Reset, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("R") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.PowerCycle, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("T") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.Pause, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("Esc") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SetScale1x, KeyCombination = new KeyCombination() { Key1 = alt, Key2 = InputApi.GetKeyCode("1") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SetScale2x, KeyCombination = new KeyCombination() { Key1 = alt, Key2 = InputApi.GetKeyCode("2") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SetScale3x, KeyCombination = new KeyCombination() { Key1 = alt, Key2 = InputApi.GetKeyCode("3") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SetScale4x, KeyCombination = new KeyCombination() { Key1 = alt, Key2 = InputApi.GetKeyCode("4") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SetScale5x, KeyCombination = new KeyCombination() { Key1 = alt, Key2 = InputApi.GetKeyCode("5") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SetScale6x, KeyCombination = new KeyCombination() { Key1 = alt, Key2 = InputApi.GetKeyCode("6") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.OpenFile, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("O") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot1, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F1") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot2, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F2") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot3, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F3") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot4, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F4") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot5, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F5") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot6, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F6") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateSlot7, KeyCombination = new KeyCombination() { Key1 = shift, Key2 = InputApi.GetKeyCode("F7") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.SaveStateToFile, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("S") } });

			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot1, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F1") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot2, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F2") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot3, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F3") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot4, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F4") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot5, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F5") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot6, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F6") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlot7, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F7") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateSlotAuto, KeyCombination = new KeyCombination() { Key1 = InputApi.GetKeyCode("F8") } });
			AddShortcut(new ShortcutKeyInfo { Shortcut = EmulatorShortcut.LoadStateFromFile, KeyCombination = new KeyCombination() { Key1 = ctrl, Key2 = InputApi.GetKeyCode("L") } });

			foreach(EmulatorShortcut value in Enum.GetValues<EmulatorShortcut>()) {
				AddShortcut(new ShortcutKeyInfo { Shortcut = value });
			}
		}

		public void UpdateFileAssociations()
		{
			FileAssociationHelper.UpdateFileAssociation("sfc", AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("smc", AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("swc", AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("fig", AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("bs", AssociateSnesRomFiles);

			FileAssociationHelper.UpdateFileAssociation("spc", AssociateSnesMusicFiles);

			FileAssociationHelper.UpdateFileAssociation("nes", AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("fds", AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("unf", AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("studybox", AssociateNesRomFiles);

			FileAssociationHelper.UpdateFileAssociation("nsf", AssociateNesMusicFiles);
			FileAssociationHelper.UpdateFileAssociation("nsfe", AssociateNesMusicFiles);

			FileAssociationHelper.UpdateFileAssociation("gb", AssociateGbRomFiles);
			FileAssociationHelper.UpdateFileAssociation("gbc", AssociateGbRomFiles);
			FileAssociationHelper.UpdateFileAssociation("gbs", AssociateGbMusicFiles);

			FileAssociationHelper.UpdateFileAssociation("msm", AssociateMovieFiles);
			FileAssociationHelper.UpdateFileAssociation("mss", AssociateSaveStateFiles);
		}

		public void ApplyConfig()
		{
			//TODO
			//frmMain.Instance.TopMost = AlwaysOnTop;

			List<InteropShortcutKeyInfo> shortcutKeys = new List<InteropShortcutKeyInfo>();
			foreach(ShortcutKeyInfo shortcutInfo in ShortcutKeys) {
				if(!shortcutInfo.KeyCombination.IsEmpty) {
					shortcutKeys.Add(new InteropShortcutKeyInfo(shortcutInfo.Shortcut, shortcutInfo.KeyCombination.ToInterop()));
				}
				if(!shortcutInfo.KeyCombination2.IsEmpty) {
					shortcutKeys.Add(new InteropShortcutKeyInfo(shortcutInfo.Shortcut, shortcutInfo.KeyCombination2.ToInterop()));
				}
			}
			ConfigApi.SetShortcutKeys(shortcutKeys.ToArray(), (UInt32)shortcutKeys.Count);

			ConfigApi.SetPreferences(new InteropPreferencesConfig() {
				ShowFps = ShowFps,
				ShowFrameCounter = ShowFrameCounter,
				ShowGameTimer = ShowGameTimer,
				ShowDebugInfo = ShowDebugInfo,
				DisableOsd = DisableOsd,
				AllowBackgroundInput = AllowBackgroundInput,
				PauseOnMovieEnd = PauseOnMovieEnd,
				ShowMovieIcons = ShowMovieIcons,
				DisableGameSelectionScreen = DisableGameSelectionScreen,
				SaveFolderOverride = OverrideSaveDataFolder ? SaveDataFolder : "",
				SaveStateFolderOverride = OverrideSaveStateFolder ? SaveStateFolder : "",
				ScreenshotFolderOverride = OverrideScreenshotFolder ? ScreenshotFolder : "",
				RewindBufferSize = EnableRewind ? RewindBufferSize : 0,
				AutoSaveStateDelay = EnableAutoSaveState ? AutoSaveStateDelay : 0
			});
		}
	}

	public enum MesenTheme
	{
		Light = 0,
		Dark = 1
	}

	public struct InteropPreferencesConfig
	{
		[MarshalAs(UnmanagedType.I1)] public bool ShowFps;
		[MarshalAs(UnmanagedType.I1)] public bool ShowFrameCounter;
		[MarshalAs(UnmanagedType.I1)] public bool ShowGameTimer;
		[MarshalAs(UnmanagedType.I1)] public bool ShowDebugInfo;
		[MarshalAs(UnmanagedType.I1)] public bool DisableOsd;
		[MarshalAs(UnmanagedType.I1)] public bool AllowBackgroundInput;
		[MarshalAs(UnmanagedType.I1)] public bool PauseOnMovieEnd;
		[MarshalAs(UnmanagedType.I1)] public bool ShowMovieIcons;
		[MarshalAs(UnmanagedType.I1)] public bool DisableGameSelectionScreen;

		public UInt32 AutoSaveStateDelay;
		public UInt32 RewindBufferSize;

		public string SaveFolderOverride;
		public string SaveStateFolderOverride;
		public string ScreenshotFolderOverride;
	}
}
