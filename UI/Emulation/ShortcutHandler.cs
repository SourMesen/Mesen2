using Mesen.GUI.Config;
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Emulation
{
	public class ShortcutHandler
	{
		private DisplayManager _displayManager;
		private Dictionary<EmulatorShortcut, Func<bool>> _actionEnabledFuncs = new Dictionary<EmulatorShortcut, Func<bool>>();
		private List<uint> _speedValues = new List<uint> { 1, 3, 6, 12, 25, 50, 75, 100, 150, 200, 250, 300, 350, 400, 450, 500, 750, 1000, 2000, 4000 };

		public ShortcutHandler(DisplayManager displayManager)
		{
			_displayManager = displayManager;
		}

		public void BindShortcut(ToolStripMenuItem item, EmulatorShortcut shortcut, Func<bool> isActionEnabled = null)
		{
			item.Click += (object sender, EventArgs e) => {
				if(isActionEnabled == null || isActionEnabled()) {
					ExecuteShortcut(shortcut);
				}
			};

			_actionEnabledFuncs[shortcut] = isActionEnabled;

			PreferencesConfig cfg = ConfigManager.Config.Preferences;
			if(item.OwnerItem is ToolStripMenuItem) {
				Action updateShortcut = () => {
					int keyIndex = cfg.ShortcutKeys1.FindIndex((ShortcutKeyInfo shortcutInfo) => shortcutInfo.Shortcut == shortcut);
					if(keyIndex >= 0) {
						item.ShortcutKeyDisplayString = cfg.ShortcutKeys1[keyIndex].KeyCombination.ToString();
					} else {
						keyIndex = cfg.ShortcutKeys2.FindIndex((ShortcutKeyInfo shortcutInfo) => shortcutInfo.Shortcut == shortcut);
						if(keyIndex >= 0) {
							item.ShortcutKeyDisplayString = cfg.ShortcutKeys2[keyIndex].KeyCombination.ToString();
						} else {
							item.ShortcutKeyDisplayString = "";
						}
					}
					item.Enabled = isActionEnabled == null || isActionEnabled();
				};

				updateShortcut();

				//Update item shortcut text when its parent opens
				((ToolStripMenuItem)item.OwnerItem).DropDownOpening += (object sender, EventArgs e) => { updateShortcut(); };
			}
		}

		public void ExecuteShortcut(EmulatorShortcut shortcut)
		{
			Func<bool> isActionEnabled;
			if(_actionEnabledFuncs.TryGetValue(shortcut, out isActionEnabled)) {
				isActionEnabled = _actionEnabledFuncs[shortcut];
				if(isActionEnabled != null && !isActionEnabled()) {
					//Action disabled
					return;
				}
			}

			bool restoreFullscreen = _displayManager.ExclusiveFullscreen;

			switch(shortcut) {
				case EmulatorShortcut.Pause: TogglePause(); break;
				case EmulatorShortcut.Reset: ResetEmu(); break;
				case EmulatorShortcut.PowerCycle: PowerCycleEmu(); break;
				case EmulatorShortcut.PowerOff: Task.Run(() => EmuApi.Stop()); restoreFullscreen = false; break;
				case EmulatorShortcut.Exit: Application.OpenForms[0].Close(); restoreFullscreen = false; break;

				case EmulatorShortcut.ToggleAudio: ToggleAudio(); break;
				case EmulatorShortcut.ToggleFps: ToggleFps(); break;
				case EmulatorShortcut.ToggleGameTimer: ToggleGameTimer(); break;
				case EmulatorShortcut.ToggleFrameCounter: ToggleFrameCounter(); break;
				case EmulatorShortcut.ToggleOsd: ToggleOsd(); break;
				case EmulatorShortcut.ToggleAlwaysOnTop: ToggleAlwaysOnTop(); break;
				case EmulatorShortcut.ToggleDebugInfo: ToggleDebugInfo(); break;
				case EmulatorShortcut.MaxSpeed: ToggleMaxSpeed(); break;
				case EmulatorShortcut.ToggleFullscreen: _displayManager.ToggleFullscreen(); restoreFullscreen = false; break;

				case EmulatorShortcut.OpenFile: OpenFile(); break;
				case EmulatorShortcut.IncreaseSpeed: IncreaseEmulationSpeed(); break;
				case EmulatorShortcut.DecreaseSpeed: DecreaseEmulationSpeed(); break;

				case EmulatorShortcut.SetScale1x: _displayManager.SetScale(1, true); break;
				case EmulatorShortcut.SetScale2x: _displayManager.SetScale(2, true); break;
				case EmulatorShortcut.SetScale3x: _displayManager.SetScale(3, true); break;
				case EmulatorShortcut.SetScale4x: _displayManager.SetScale(4, true); break;
				case EmulatorShortcut.SetScale5x: _displayManager.SetScale(5, true); break;
				case EmulatorShortcut.SetScale6x: _displayManager.SetScale(6, true); break;
		
				case EmulatorShortcut.TakeScreenshot: EmuApi.TakeScreenshot(); break;

				case EmulatorShortcut.LoadStateFromFile: SaveStateManager.LoadStateFromFile(); break;
				case EmulatorShortcut.SaveStateToFile: SaveStateManager.SaveStateToFile(); break;

				case EmulatorShortcut.SaveStateSlot1: SaveStateManager.SaveState(1); break;
				case EmulatorShortcut.SaveStateSlot2: SaveStateManager.SaveState(2); break;
				case EmulatorShortcut.SaveStateSlot3: SaveStateManager.SaveState(3); break;
				case EmulatorShortcut.SaveStateSlot4: SaveStateManager.SaveState(4); break;
				case EmulatorShortcut.SaveStateSlot5: SaveStateManager.SaveState(5); break;
				case EmulatorShortcut.SaveStateSlot6: SaveStateManager.SaveState(6); break;
				case EmulatorShortcut.SaveStateSlot7: SaveStateManager.SaveState(7); break;
				case EmulatorShortcut.SaveStateSlot8: SaveStateManager.SaveState(8); break;
				case EmulatorShortcut.SaveStateSlot9: SaveStateManager.SaveState(9); break;
				case EmulatorShortcut.SaveStateSlot10: SaveStateManager.SaveState(10); break;
				case EmulatorShortcut.LoadStateSlot1: SaveStateManager.LoadState(1); break;
				case EmulatorShortcut.LoadStateSlot2: SaveStateManager.LoadState(2); break;
				case EmulatorShortcut.LoadStateSlot3: SaveStateManager.LoadState(3); break;
				case EmulatorShortcut.LoadStateSlot4: SaveStateManager.LoadState(4); break;
				case EmulatorShortcut.LoadStateSlot5: SaveStateManager.LoadState(5); break;
				case EmulatorShortcut.LoadStateSlot6: SaveStateManager.LoadState(6); break;
				case EmulatorShortcut.LoadStateSlot7: SaveStateManager.LoadState(7); break;
				case EmulatorShortcut.LoadStateSlot8: SaveStateManager.LoadState(8); break;
				case EmulatorShortcut.LoadStateSlot9: SaveStateManager.LoadState(9); break;
				case EmulatorShortcut.LoadStateSlot10: SaveStateManager.LoadState(10); break;
				case EmulatorShortcut.LoadStateSlotAuto: SaveStateManager.LoadState(11); break;
			}

			if(restoreFullscreen && !_displayManager.ExclusiveFullscreen) {
				//Need to restore fullscreen mode after showing a dialog
				_displayManager.SetFullscreenState(true);
			}
		}
		
		private void OpenFile()
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.Filter = ResourceHelper.GetMessage("FilterRom");
				
				if(ConfigManager.Config.Preferences.OverrideGameFolder && Directory.Exists(ConfigManager.Config.Preferences.GameFolder)) {
					ofd.InitialDirectory = ConfigManager.Config.Preferences.GameFolder;
				} else if(ConfigManager.Config.RecentFiles.Items.Count > 0) {
					ofd.InitialDirectory = ConfigManager.Config.RecentFiles.Items[0].RomFile.Folder;
				}

				if(ofd.ShowDialog(Application.OpenForms[0]) == DialogResult.OK) {
					EmuRunner.LoadRom(ofd.FileName);
				}
			}
		}
				
		public void SetRegion(ConsoleRegion region)
		{
			ConfigManager.Config.Emulation.Region = region;
			ConfigManager.Config.Emulation.ApplyConfig();
			ConfigManager.ApplyChanges();
		}
		
		public void SetVideoFilter(VideoFilterType filter)
		{
			ConfigManager.Config.Video.VideoFilter = filter;
			ConfigManager.Config.Video.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		public void ToggleBilinearInterpolation()
		{
			ConfigManager.Config.Video.UseBilinearInterpolation = !ConfigManager.Config.Video.UseBilinearInterpolation;
			ConfigManager.Config.Video.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void SetEmulationSpeed(uint emulationSpeed)
		{
			ConfigManager.Config.Emulation.EmulationSpeed = emulationSpeed;
			ConfigManager.Config.Emulation.ApplyConfig();
			ConfigManager.ApplyChanges();

			if(emulationSpeed == 0) {
				EmuApi.DisplayMessage("EmulationSpeed", "EmulationMaximumSpeed");
			} else {
				EmuApi.DisplayMessage("EmulationSpeed", "EmulationSpeedPercent", emulationSpeed.ToString());
			}
		}

		private void IncreaseEmulationSpeed()
		{
			uint emulationSpeed = ConfigManager.Config.Emulation.EmulationSpeed;
			if(emulationSpeed == _speedValues[_speedValues.Count - 1]) {
				SetEmulationSpeed(0);
			} else if(emulationSpeed != 0) {
				for(int i = 0; i < _speedValues.Count; i++) {
					if(_speedValues[i] > emulationSpeed) {
						SetEmulationSpeed(_speedValues[i]);
						break;
					}
				}
			}
		}

		private void DecreaseEmulationSpeed()
		{
			uint emulationSpeed = ConfigManager.Config.Emulation.EmulationSpeed;
			if(emulationSpeed == 0) {
				SetEmulationSpeed(_speedValues[_speedValues.Count - 1]);
			} else if(emulationSpeed > _speedValues[0]) {
				for(int i = _speedValues.Count - 1; i >= 0; i--) {
					if(_speedValues[i] < emulationSpeed) {
						SetEmulationSpeed(_speedValues[i]);
						break;
					}
				}
			}
		}
		
		private void ToggleMaxSpeed()
		{
			if(ConfigManager.Config.Emulation.EmulationSpeed == 0) {
				SetEmulationSpeed(100);
			} else {
				SetEmulationSpeed(0);
			}
		}

		private void ToggleOsd()
		{
			ConfigManager.Config.Preferences.DisableOsd = !ConfigManager.Config.Preferences.DisableOsd;
			ConfigManager.Config.Preferences.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void ToggleFps()
		{
			ConfigManager.Config.Preferences.ShowFps = !ConfigManager.Config.Preferences.ShowFps;
			ConfigManager.Config.Preferences.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void ToggleAudio()
		{
			ConfigManager.Config.Audio.EnableAudio = !ConfigManager.Config.Audio.EnableAudio;
			ConfigManager.Config.Audio.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void ToggleFrameCounter()
		{
			ConfigManager.Config.Preferences.ShowFrameCounter = !ConfigManager.Config.Preferences.ShowFrameCounter;
			ConfigManager.Config.Preferences.ApplyConfig();
			ConfigManager.ApplyChanges();
		}
		
		private void ToggleGameTimer()
		{
			ConfigManager.Config.Preferences.ShowGameTimer = !ConfigManager.Config.Preferences.ShowGameTimer;
			ConfigManager.Config.Preferences.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void ToggleAlwaysOnTop()
		{
			ConfigManager.Config.Preferences.AlwaysOnTop = !ConfigManager.Config.Preferences.AlwaysOnTop;
			ConfigManager.Config.Preferences.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void ToggleDebugInfo()
		{
			ConfigManager.Config.Preferences.ShowDebugInfo = !ConfigManager.Config.Preferences.ShowDebugInfo;
			ConfigManager.Config.Preferences.ApplyConfig();
			ConfigManager.ApplyChanges();
		}

		private void TogglePause()
		{
			if(EmuApi.IsPaused()) {
				EmuApi.Resume();
			} else {
				EmuApi.Pause();
			}
		}

		private void ResetEmu()
		{
			//TODO
		}

		private void PowerCycleEmu()
		{
			//TODO
		}
	}
}
