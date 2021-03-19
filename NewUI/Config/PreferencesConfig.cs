using Mesen.Localization;
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using ReactiveUI.Fody.Helpers;

namespace Mesen.GUI.Config
{
	public class PreferencesConfig
	{
		[Reactive] public Language DisplayLanguage { get; set; } = Language.English;
		[Reactive] public bool AutomaticallyCheckForUpdates { get; set; } = true;
		[Reactive] public bool SingleInstance { get; set; } = true;
		[Reactive] public bool AutoLoadPatches { get; set; } = true;

		[Reactive] public bool PauseWhenInBackground { get; set; } = false;
		[Reactive] public bool PauseWhenInMenusAndConfig { get; set; } = false;
		[Reactive] public bool PauseWhenInDebuggingTools { get; set; } = false;
		[Reactive] public bool AllowBackgroundInput { get; set; } = false;
		[Reactive] public bool PauseOnMovieEnd { get; set; } = true;

		[Reactive] public bool AssociateRomFiles { get; set; } = false;
		[Reactive] public bool AssociateSpcFiles { get; set; } = false;
		[Reactive] public bool AssociateBsFiles { get; set; } = false;
		[Reactive] public bool AssociateMsmFiles { get; set; } = false;
		[Reactive] public bool AssociateMssFiles { get; set; } = false;

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

		public List<ShortcutKeyInfo> ShortcutKeys1;
		public List<ShortcutKeyInfo> ShortcutKeys2;

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
			ShortcutKeys1 = new List<ShortcutKeyInfo>();
			ShortcutKeys2 = new List<ShortcutKeyInfo>();
		}

		public PreferencesConfig Clone()
		{
			PreferencesConfig copy = (PreferencesConfig)this.MemberwiseClone();
			copy.ShortcutKeys1 = new List<ShortcutKeyInfo>(copy.ShortcutKeys1);
			copy.ShortcutKeys2 = new List<ShortcutKeyInfo>(copy.ShortcutKeys2);
			return copy;
		}

		public void InitializeDefaultShortcuts()
		{
			if(ShortcutKeys1 != null && ShortcutKeys2 != null) {
				return;
			}

			ShortcutKeys1 = new List<ShortcutKeyInfo>();
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.FastForward, new KeyCombination() { Key1 = InputApi.GetKeyCode("Tab") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.Rewind, new KeyCombination() { Key1 = InputApi.GetKeyCode("Backspace") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.IncreaseSpeed, new KeyCombination() { Key1 = InputApi.GetKeyCode("=") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.DecreaseSpeed, new KeyCombination() { Key1 = InputApi.GetKeyCode("-") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.MaxSpeed, new KeyCombination() { Key1 = InputApi.GetKeyCode("F9") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.IncreaseVolume, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("=") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.DecreaseVolume, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("-") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.ToggleFps, new KeyCombination() { Key1 = InputApi.GetKeyCode("F10") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.ToggleFullscreen, new KeyCombination() { Key1 = InputApi.GetKeyCode("F11") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.TakeScreenshot, new KeyCombination() { Key1 = InputApi.GetKeyCode("F12") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.Reset, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("R") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.PowerCycle, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("T") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.Pause, new KeyCombination() { Key1 = InputApi.GetKeyCode("Esc") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SetScale1x, new KeyCombination() { Key1 = InputApi.GetKeyCode("Alt"), Key2 = InputApi.GetKeyCode("1") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SetScale2x, new KeyCombination() { Key1 = InputApi.GetKeyCode("Alt"), Key2 = InputApi.GetKeyCode("2") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SetScale3x, new KeyCombination() { Key1 = InputApi.GetKeyCode("Alt"), Key2 = InputApi.GetKeyCode("3") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SetScale4x, new KeyCombination() { Key1 = InputApi.GetKeyCode("Alt"), Key2 = InputApi.GetKeyCode("4") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SetScale5x, new KeyCombination() { Key1 = InputApi.GetKeyCode("Alt"), Key2 = InputApi.GetKeyCode("5") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SetScale6x, new KeyCombination() { Key1 = InputApi.GetKeyCode("Alt"), Key2 = InputApi.GetKeyCode("6") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.OpenFile, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("O") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot1, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F1") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot2, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F2") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot3, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F3") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot4, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F4") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot5, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F5") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot6, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F6") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateSlot7, new KeyCombination() { Key1 = InputApi.GetKeyCode("Shift"), Key2 = InputApi.GetKeyCode("F7") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.SaveStateToFile, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("S") }));

			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot1, new KeyCombination() { Key1 = InputApi.GetKeyCode("F1") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot2, new KeyCombination() { Key1 = InputApi.GetKeyCode("F2") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot3, new KeyCombination() { Key1 = InputApi.GetKeyCode("F3") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot4, new KeyCombination() { Key1 = InputApi.GetKeyCode("F4") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot5, new KeyCombination() { Key1 = InputApi.GetKeyCode("F5") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot6, new KeyCombination() { Key1 = InputApi.GetKeyCode("F6") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlot7, new KeyCombination() { Key1 = InputApi.GetKeyCode("F7") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateSlotAuto, new KeyCombination() { Key1 = InputApi.GetKeyCode("F8") }));
			ShortcutKeys1.Add(new ShortcutKeyInfo(EmulatorShortcut.LoadStateFromFile, new KeyCombination() { Key1 = InputApi.GetKeyCode("Ctrl"), Key2 = InputApi.GetKeyCode("L") }));

			ShortcutKeys2 = new List<ShortcutKeyInfo>();
			ShortcutKeys2.Add(new ShortcutKeyInfo(EmulatorShortcut.FastForward, new KeyCombination() { Key1 = InputApi.GetKeyCode("Pad1 R2") }));
			ShortcutKeys2.Add(new ShortcutKeyInfo(EmulatorShortcut.Rewind, new KeyCombination() { Key1 = InputApi.GetKeyCode("Pad1 L2") }));
		}

		public void ApplyConfig()
		{
			/*if(Program.IsMono) {
				FileAssociationHelper.ConfigureLinuxMimeTypes();
			} else {
				FileAssociationHelper.UpdateFileAssociation("sfc", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("smc", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("swc", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("fig", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("msm", this.AssociateMsmFiles);
				FileAssociationHelper.UpdateFileAssociation("mss", this.AssociateMssFiles);
				FileAssociationHelper.UpdateFileAssociation("spc", this.AssociateSpcFiles);
				FileAssociationHelper.UpdateFileAssociation("bs", this.AssociateBsFiles);
			}

			frmMain.Instance.TopMost = AlwaysOnTop;

			ShortcutKeyInfo[] shortcutKeys = new ShortcutKeyInfo[ShortcutKeys1.Count + ShortcutKeys2.Count];
			int i = 0;
			foreach(ShortcutKeyInfo shortcutInfo in ShortcutKeys1) {
				shortcutKeys[i++] = shortcutInfo;
			}
			foreach(ShortcutKeyInfo shortcutInfo in ShortcutKeys2) {
				shortcutKeys[i++] = shortcutInfo;
			}
			ConfigApi.SetShortcutKeys(shortcutKeys, (UInt32)shortcutKeys.Length);

			ConfigApi.SetPreferences(new InteropPreferencesConfig() {
				ShowFps = ShowFps,
				ShowFrameCounter = ShowFrameCounter,
				ShowGameTimer = ShowGameTimer,
				ShowDebugInfo = ShowDebugInfo,
				DisableOsd = DisableOsd,
				AllowBackgroundInput = AllowBackgroundInput,
				PauseOnMovieEnd = PauseOnMovieEnd,
				DisableGameSelectionScreen = DisableGameSelectionScreen,
				SaveFolderOverride = OverrideSaveDataFolder ? SaveDataFolder : "",
				SaveStateFolderOverride = OverrideSaveStateFolder ? SaveStateFolder : "",
				ScreenshotFolderOverride = OverrideScreenshotFolder ? ScreenshotFolder : "",
				RewindBufferSize = RewindBufferSize
			});*/
		}
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
		[MarshalAs(UnmanagedType.I1)] public bool DisableGameSelectionScreen;
		
		public UInt32 RewindBufferSize;

		public string SaveFolderOverride;
		public string SaveStateFolderOverride;
		public string ScreenshotFolderOverride;
	}
}
