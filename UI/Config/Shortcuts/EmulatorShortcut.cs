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

		IncreaseSpeed,
		DecreaseSpeed,
		MaxSpeed,

		Pause,
		Reset,
		PowerCycle,
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

		OpenFile,
		LoadRandomGame,
	}
}
