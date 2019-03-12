using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Config
{
	public class PreferencesConfig
	{
		public Language DisplayLanguage = Language.SystemDefault;
		public bool AutomaticallyCheckForUpdates = true;
		public bool SingleInstance = true;
		public bool AutoLoadPatches = true;

		public bool AssociateRomFiles = false;
		public bool AssociateMsmFiles = false;
		public bool AssociateMssFiles = false;

		public bool AlwaysOnTop = false;

		public bool ShowFps = false;
		public bool ShowFrameCounter = false;
		public bool ShowGameTimer = false;
		public bool ShowTitleBarInfo = false;
		public bool ShowDebugInfo = false;
		public bool DisableOsd = false;

		public List<ShortcutKeyInfo> ShortcutKeys1;
		public List<ShortcutKeyInfo> ShortcutKeys2;

		public bool OverrideGameFolder = false;
		public bool OverrideAviFolder = false;
		public bool OverrideMovieFolder = false;
		public bool OverrideSaveDataFolder = false;
		public bool OverrideSaveStateFolder = false;
		public bool OverrideScreenshotFolder = false;
		public bool OverrideWaveFolder = false;

		public string GameFolder = "";
		public string AviFolder = "";
		public string MovieFolder = "";
		public string SaveDataFolder = "";
		public string SaveStateFolder = "";
		public string ScreenshotFolder = "";
		public string WaveFolder = "";

		public PreferencesConfig()
		{
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
			if(Program.IsMono) {
				FileAssociationHelper.ConfigureLinuxMimeTypes();
			} else {
				FileAssociationHelper.UpdateFileAssociation("sfc", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("smc", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("swc", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("fig", this.AssociateRomFiles);
				FileAssociationHelper.UpdateFileAssociation("msm", this.AssociateMsmFiles);
				FileAssociationHelper.UpdateFileAssociation("mss", this.AssociateMssFiles);
			}

			ConfigApi.SetPreferences(new InteropPreferencesConfig() {
				ShowFps = ShowFps,
				ShowFrameCounter = ShowFrameCounter,
				ShowGameTimer = ShowGameTimer,
				ShowDebugInfo = ShowDebugInfo,
				DisableOsd = DisableOsd,
				SaveFolderOverride = OverrideSaveDataFolder ? SaveDataFolder : "",
				SaveStateFolderOverride = OverrideSaveStateFolder ? SaveStateFolder : "",
				ScreenshotFolderOverride = OverrideScreenshotFolder ? ScreenshotFolder : ""
			});

			Application.OpenForms[0].TopMost = AlwaysOnTop;

			ShortcutKeyInfo[] shortcutKeys = new ShortcutKeyInfo[ShortcutKeys1.Count + ShortcutKeys2.Count];
			int i = 0;
			foreach(ShortcutKeyInfo shortcutInfo in ShortcutKeys1) {
				shortcutKeys[i++] = shortcutInfo;
			}
			foreach(ShortcutKeyInfo shortcutInfo in ShortcutKeys2) {
				shortcutKeys[i++] = shortcutInfo;
			}
			ConfigApi.SetShortcutKeys(shortcutKeys, (UInt32)shortcutKeys.Length);
		}
	}
	
	public struct InteropPreferencesConfig
	{
		[MarshalAs(UnmanagedType.I1)] public bool ShowFps;
		[MarshalAs(UnmanagedType.I1)] public bool ShowFrameCounter;
		[MarshalAs(UnmanagedType.I1)] public bool ShowGameTimer;
		[MarshalAs(UnmanagedType.I1)] public bool ShowDebugInfo;
		[MarshalAs(UnmanagedType.I1)] public bool DisableOsd;

		public string SaveFolderOverride;
		public string SaveStateFolderOverride;
		public string ScreenshotFolderOverride;
	}
}
