using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config.Shortcuts
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

		// Everything below this is handled UI-side
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

		SetScale1x,
		SetScale2x,
		SetScale3x,
		SetScale4x,
		SetScale5x,
		SetScale6x,
		ToggleFullscreen,
		ToggleFps,
		ToggleGameTimer,
		ToggleFrameCounter,
		ToggleOsd,
		ToggleAlwaysOnTop,
		ToggleDebugInfo,
		
		ToggleAudio,
		IncreaseVolume,
		DecreaseVolume,

		ToggleBgLayer0,
		ToggleBgLayer1,
		ToggleBgLayer2,
		ToggleBgLayer3,
		ToggleSprites,
		EnableAllLayers,

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
		//LoadStateSlotAuto,
		LoadStateFromFile,
		LoadStateDialog,

		OpenFile,
		LoadRandomGame,

		//NES
		SwitchDiskSide,
		EjectDisk,
		InsertCoin1,
		InsertCoin2,
		InsertCoin3,
		InsertCoin4,
		InputBarcode,
	}
}
