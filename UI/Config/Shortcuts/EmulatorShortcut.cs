using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config.Shortcuts
{
	public enum EmulatorShortcut
	{
		FastForward,
		Rewind,
		RewindTenSecs,
		RewindOneMin,

		SelectSaveSlot1,
		SelectSaveSlot2,
		SelectSaveSlot3,
		SelectSaveSlot4,
		SelectSaveSlot5,
		SelectSaveSlot6,
		SelectSaveSlot7,
		SelectSaveSlot8,
		SelectSaveSlot9,
		SelectSaveSlot10,
		MoveToNextStateSlot,
		MoveToPreviousStateSlot,
		SaveState,
		LoadState,

		ToggleCheats,
		ToggleFastForward,
		ToggleRewind,

		RunSingleFrame,

		TakeScreenshot,

		ToggleRecordVideo,
		ToggleRecordAudio,
		ToggleRecordMovie,

		IncreaseSpeed,
		DecreaseSpeed,
		MaxSpeed,

		Pause,
		Reset,
		PowerCycle,
		ReloadRom,
		PowerOff,
		Exit,

		ExecReset,
		ExecPowerCycle,
		ExecReloadRom,
		ExecPowerOff,

		SetScale1x,
		SetScale2x,
		SetScale3x,
		SetScale4x,
		SetScale5x,
		SetScale6x,
		SetScale7x,
		SetScale8x,
		SetScale9x,
		SetScale10x,
		ToggleFullscreen,
		ToggleFps,
		ToggleGameTimer,
		ToggleFrameCounter,
		ToggleLagCounter,
		ToggleOsd,
		ToggleAlwaysOnTop,
		ToggleDebugInfo,

		ToggleAudio,
		IncreaseVolume,
		DecreaseVolume,

		PreviousTrack,
		NextTrack,

		ToggleBgLayer1,
		ToggleBgLayer2,
		ToggleBgLayer3,
		ToggleBgLayer4,
		ToggleSprites1,
		ToggleSprites2,
		EnableAllLayers,

		ResetLagCounter,

		SaveStateSlot1,
		SaveStateSlot2,
		SaveStateSlot3,
		SaveStateSlot4,
		SaveStateSlot5,
		SaveStateSlot6,
		SaveStateSlot7,
		SaveStateSlot8,
		SaveStateSlot9,
		SaveStateSlot10,
		SaveStateToFile,
		SaveStateDialog,

		LoadStateSlot1,
		LoadStateSlot2,
		LoadStateSlot3,
		LoadStateSlot4,
		LoadStateSlot5,
		LoadStateSlot6,
		LoadStateSlot7,
		LoadStateSlot8,
		LoadStateSlot9,
		LoadStateSlot10,
		LoadStateSlotAuto,
		LoadStateFromFile,
		LoadStateDialog,
		LoadLastSession,

		OpenFile,

		InputBarcode,
		LoadTape,
		RecordTape,
		StopRecordTape,

		//NES
		FdsSwitchDiskSide,
		FdsEjectDisk,
		FdsInsertDiskNumber,
		FdsInsertNextDisk,
		VsServiceButton,
		VsServiceButton2,
		VsInsertCoin1,
		VsInsertCoin2,
		VsInsertCoin3,
		VsInsertCoin4,
		StartRecordHdPack,
		StopRecordHdPack,

		LastValidValue,
		[Obsolete] LoadRandomGame,
	}

	public static class EmulatorShortcutExtensions
	{
		public static KeyCombination? GetShortcutKeys(this EmulatorShortcut shortcut)
		{
			PreferencesConfig cfg = ConfigManager.Config.Preferences;
			int keyIndex = cfg.ShortcutKeys.FindIndex((ShortcutKeyInfo shortcutInfo) => shortcutInfo.Shortcut == shortcut);
			if(keyIndex >= 0) {
				if(!cfg.ShortcutKeys[keyIndex].KeyCombination.IsEmpty) {
					return cfg.ShortcutKeys[keyIndex].KeyCombination;
				} else if(!cfg.ShortcutKeys[keyIndex].KeyCombination2.IsEmpty) {
					return cfg.ShortcutKeys[keyIndex].KeyCombination2;
				}
			}
			return null;
		}
	}
}
