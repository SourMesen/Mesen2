using Mesen.GUI.Config;
using Mesen.GUI.Config.Shortcuts;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class PreferencesConfigViewModel : ViewModelBase
	{
		[Reactive] public PreferencesConfig Config { get; set; }

		public List<ShortcutKeyInfo> ShortcutKeys { get; set; }

		public PreferencesConfigViewModel()
		{
			Config = ConfigManager.Config.Preferences.Clone();

			EmulatorShortcut[] displayOrder = new EmulatorShortcut[] {
				EmulatorShortcut.FastForward,
				EmulatorShortcut.ToggleFastForward,
				EmulatorShortcut.Rewind,
				EmulatorShortcut.ToggleRewind,
				EmulatorShortcut.RewindTenSecs,
				EmulatorShortcut.RewindOneMin,

				EmulatorShortcut.Pause,
				EmulatorShortcut.Reset,
				EmulatorShortcut.PowerCycle,
				EmulatorShortcut.ReloadRom,
				EmulatorShortcut.PowerOff,
				EmulatorShortcut.Exit,

				EmulatorShortcut.ToggleRecordVideo,
				EmulatorShortcut.ToggleRecordAudio,
				EmulatorShortcut.ToggleRecordMovie,

				EmulatorShortcut.TakeScreenshot,
				EmulatorShortcut.RunSingleFrame,

				EmulatorShortcut.SetScale1x,
				EmulatorShortcut.SetScale2x,
				EmulatorShortcut.SetScale3x,
				EmulatorShortcut.SetScale4x,
				EmulatorShortcut.SetScale5x,
				EmulatorShortcut.SetScale6x,
				EmulatorShortcut.ToggleFullscreen,

				EmulatorShortcut.ToggleDebugInfo,
				EmulatorShortcut.ToggleFps,
				EmulatorShortcut.ToggleGameTimer,
				EmulatorShortcut.ToggleFrameCounter,
				EmulatorShortcut.ToggleOsd,
				EmulatorShortcut.ToggleAlwaysOnTop,
				EmulatorShortcut.ToggleCheats,

				EmulatorShortcut.ToggleBgLayer0,
				EmulatorShortcut.ToggleBgLayer1,
				EmulatorShortcut.ToggleBgLayer2,
				EmulatorShortcut.ToggleBgLayer3,
				EmulatorShortcut.ToggleSprites,
				EmulatorShortcut.EnableAllLayers,

				EmulatorShortcut.ToggleAudio,
				EmulatorShortcut.IncreaseVolume,
				EmulatorShortcut.DecreaseVolume,

				EmulatorShortcut.MaxSpeed,
				EmulatorShortcut.IncreaseSpeed,
				EmulatorShortcut.DecreaseSpeed,

				EmulatorShortcut.OpenFile,
				EmulatorShortcut.LoadRandomGame,

				EmulatorShortcut.MoveToNextStateSlot,
				EmulatorShortcut.MoveToPreviousStateSlot,
				EmulatorShortcut.SaveState,
				EmulatorShortcut.LoadState,

				EmulatorShortcut.SaveStateSlot1,
				EmulatorShortcut.SaveStateSlot2,
				EmulatorShortcut.SaveStateSlot3,
				EmulatorShortcut.SaveStateSlot4,
				EmulatorShortcut.SaveStateSlot5,
				EmulatorShortcut.SaveStateSlot6,
				EmulatorShortcut.SaveStateSlot7,
				EmulatorShortcut.SaveStateSlot8,
				EmulatorShortcut.SaveStateSlot9,
				EmulatorShortcut.SaveStateSlot10,
				EmulatorShortcut.SaveStateToFile,
				EmulatorShortcut.SaveStateDialog,

				EmulatorShortcut.LoadStateSlot1,
				EmulatorShortcut.LoadStateSlot2,
				EmulatorShortcut.LoadStateSlot3,
				EmulatorShortcut.LoadStateSlot4,
				EmulatorShortcut.LoadStateSlot5,
				EmulatorShortcut.LoadStateSlot6,
				EmulatorShortcut.LoadStateSlot7,
				EmulatorShortcut.LoadStateSlot8,
				EmulatorShortcut.LoadStateSlot9,
				EmulatorShortcut.LoadStateSlot10,
				EmulatorShortcut.LoadStateFromFile,
				EmulatorShortcut.LoadStateDialog,

				EmulatorShortcut.SelectSaveSlot1,
				EmulatorShortcut.SelectSaveSlot2,
				EmulatorShortcut.SelectSaveSlot3,
				EmulatorShortcut.SelectSaveSlot4,
				EmulatorShortcut.SelectSaveSlot5,
				EmulatorShortcut.SelectSaveSlot6,
				EmulatorShortcut.SelectSaveSlot7,
				EmulatorShortcut.SelectSaveSlot8,
				EmulatorShortcut.SelectSaveSlot9,
				EmulatorShortcut.SelectSaveSlot10
			};

			Dictionary<EmulatorShortcut, int> order = new Dictionary<EmulatorShortcut, int>();
			for(int i = 0; i < displayOrder.Length; i++) {
				order[displayOrder[i]] = i;
			}

			ShortcutKeys = new List<ShortcutKeyInfo>(Config.ShortcutKeys);
			ShortcutKeys.Sort((a, b) => order[a.Shortcut] - order[b.Shortcut]);
		}
   }
}
