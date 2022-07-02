using Avalonia.Controls;
using Avalonia.Interactivity;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Interop;
using Mesen.ViewModels;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class ShortcutHandler
	{
		private MainWindow _mainWindow;

		private List<uint> _speedValues = new List<uint> { 1, 3, 6, 12, 25, 50, 75, 100, 150, 200, 250, 300, 350, 400, 450, 500, 750, 1000, 2000, 4000 };

		public ShortcutHandler(MainWindow mainWindow)
		{
			_mainWindow = mainWindow;
		}

		private MainWindowViewModel MainWindowModel => (MainWindowViewModel)_mainWindow.DataContext!;

		public void ExecuteShortcut(EmulatorShortcut shortcut)
		{
			if(!EmuApi.IsShortcutAllowed(shortcut, 0)) {
				return;
			}

			bool isFullscreen = _mainWindow.WindowState == WindowState.FullScreen && ConfigManager.Config.Video.UseExclusiveFullscreen;

			switch(shortcut) {
				case EmulatorShortcut.Reset: Reset(); break;
				case EmulatorShortcut.PowerCycle: PowerCycle(); break;
				case EmulatorShortcut.PowerOff: PowerOff(); break;
				case EmulatorShortcut.Exit: _mainWindow.Close(); break;

				case EmulatorShortcut.ToggleAudio: ToggleAudio(); break;
				case EmulatorShortcut.IncreaseVolume: IncreaseVolume(); break;
				case EmulatorShortcut.DecreaseVolume: DecreaseVolume(); break;

				case EmulatorShortcut.ToggleFps: ToggleFps(); break;
				case EmulatorShortcut.ToggleGameTimer: ToggleGameTimer(); break;
				case EmulatorShortcut.ToggleFrameCounter: ToggleFrameCounter(); break;
				case EmulatorShortcut.ToggleOsd: ToggleOsd(); break;
				
				//TODO
				case EmulatorShortcut.ToggleAlwaysOnTop: ToggleAlwaysOnTop(); break;

				case EmulatorShortcut.ToggleDebugInfo: ToggleDebugInfo(); break;
				case EmulatorShortcut.ToggleCheats: ToggleCheats(); break;
				case EmulatorShortcut.MaxSpeed: ToggleMaxSpeed(); break;

				case EmulatorShortcut.ToggleFullscreen: _mainWindow.ToggleFullscreen(); break;

				case EmulatorShortcut.OpenFile: OpenFile(); break;
				case EmulatorShortcut.IncreaseSpeed: IncreaseEmulationSpeed(); break;
				case EmulatorShortcut.DecreaseSpeed: DecreaseEmulationSpeed(); break;

				//TODO
				//case EmulatorShortcut.LoadRandomGame: RandomGameHelper.LoadRandomGame(); break;

				case EmulatorShortcut.SetScale1x: _mainWindow.SetScale(1); break;
				case EmulatorShortcut.SetScale2x: _mainWindow.SetScale(2); break;
				case EmulatorShortcut.SetScale3x: _mainWindow.SetScale(3); break;
				case EmulatorShortcut.SetScale4x: _mainWindow.SetScale(4); break;
				case EmulatorShortcut.SetScale5x: _mainWindow.SetScale(5); break;
				case EmulatorShortcut.SetScale6x: _mainWindow.SetScale(6); break;

				//TODO
				/*case EmulatorShortcut.ToggleBgLayer0: ToggleBgLayer0(); break;
				case EmulatorShortcut.ToggleBgLayer1: ToggleBgLayer1(); break;
				case EmulatorShortcut.ToggleBgLayer2: ToggleBgLayer2(); break;
				case EmulatorShortcut.ToggleBgLayer3: ToggleBgLayer3(); break;
				case EmulatorShortcut.ToggleSprites: ToggleSprites(); break;
				case EmulatorShortcut.EnableAllLayers: EnableAllLayers(); break;*/

				case EmulatorShortcut.ToggleRecordVideo: ToggleRecordVideo(); break;
				case EmulatorShortcut.ToggleRecordAudio: ToggleRecordAudio(); break;
				case EmulatorShortcut.ToggleRecordMovie: ToggleRecordMovie(); break;

				case EmulatorShortcut.TakeScreenshot: EmuApi.TakeScreenshot(); break;
				
				case EmulatorShortcut.InputBarcode: InputBarcode(); break;
				case EmulatorShortcut.LoadTape: LoadTape(); break;
				case EmulatorShortcut.RecordTape: RecordTape(); break;
				case EmulatorShortcut.StopRecordTape: EmuApi.ProcessTapeRecorderAction(TapeRecorderAction.StopRecord); break;

				case EmulatorShortcut.LoadStateFromFile: LoadStateFromFile(); break;
				case EmulatorShortcut.SaveStateToFile: SaveStateToFile(); break;

				case EmulatorShortcut.LoadLastSession:
					string filename = Path.Combine(ConfigManager.RecentGamesFolder, MainWindowModel.RomInfo.GetRomName() + ".rgd");
					Task.Run(() => {
						//Run in another thread to prevent deadlocks etc. when emulator notifications are processed UI-side
						if(File.Exists(filename)) {
							EmuApi.LoadRecentGame(filename, false);
						}
					});
					break;

				case EmulatorShortcut.LoadStateDialog:
					if(isFullscreen) {
						_mainWindow.ToggleFullscreen();
					}
					MainWindowModel.RecentGames.Init(GameScreenMode.LoadState);
					break;

				case EmulatorShortcut.SaveStateDialog:
					if(isFullscreen) {
						_mainWindow.ToggleFullscreen();
					}
					MainWindowModel.RecentGames.Init(GameScreenMode.SaveState);
					break;
			}
		}

		private async void InputBarcode()
		{
			string? barcode = await new InputBarcodeWindow().ShowCenteredDialog<string?>(_mainWindow);
			if(barcode != null && UInt64.TryParse(barcode, out UInt64 value)) {
				EmuApi.InputBarcode(value, (UInt32)(barcode.Length > 8 ? 13 : 8));
			}
		}

		private async void LoadTape()
		{
			string? filename = await FileDialogHelper.OpenFile(null, _mainWindow, FileDialogHelper.BinExt);
			if(filename != null) {
				EmuApi.ProcessTapeRecorderAction(TapeRecorderAction.Play, filename);
			}
		}

		private async void RecordTape()
		{
			string? filename = await FileDialogHelper.SaveFile(null, null, _mainWindow, FileDialogHelper.BinExt);
			if(filename != null && filename.Length > 0) {
				EmuApi.ProcessTapeRecorderAction(TapeRecorderAction.StartRecord, filename);
			}
		}

		private async void LoadStateFromFile()
		{
			string? filename = await FileDialogHelper.OpenFile(ConfigManager.SaveStateFolder, _mainWindow, FileDialogHelper.MesenSaveStateExt);
			if(filename != null) {
				EmuApi.LoadStateFile(filename);
			}
		}

		private async void SaveStateToFile()
		{
			string? filename = await FileDialogHelper.SaveFile(ConfigManager.SaveStateFolder, null, _mainWindow, FileDialogHelper.MesenSaveStateExt);
			if(filename != null && filename.Length > 0) {
				EmuApi.SaveStateFile(filename);
			}
		}

		private static void ToggleRecordVideo()
		{
			if(!EmuApi.IsRunning()) {
				return;
			}

			if(RecordApi.AviIsRecording()) {
				RecordApi.AviStop();
			} else {
				string filename = GetOutputFilename(ConfigManager.AviFolder, ConfigManager.Config.VideoRecord.Codec == VideoCodec.GIF ? ".gif" : ".avi");
				RecordApi.AviRecord(filename, ConfigManager.Config.VideoRecord.Codec, ConfigManager.Config.VideoRecord.CompressionLevel);
			}
		}

		private static void ToggleRecordAudio()
		{
			if(!EmuApi.IsRunning()) {
				return;
			}

			if(RecordApi.WaveIsRecording()) {
				RecordApi.WaveStop();
			} else {
				string filename = GetOutputFilename(ConfigManager.WaveFolder, ".wav");
				RecordApi.WaveRecord(filename);
			}
		}

		private static void ToggleRecordMovie()
		{
			if(!EmuApi.IsRunning()) {
				return;
			}

			if(!RecordApi.MoviePlaying() && !NetplayApi.IsConnected()) {
				if(RecordApi.MovieRecording()) {
					RecordApi.MovieStop();
				} else {
					RecordMovieOptions options = new RecordMovieOptions(
						GetOutputFilename(ConfigManager.MovieFolder, ".msm"),
						ConfigManager.Config.MovieRecord.Author,
						ConfigManager.Config.MovieRecord.Description,
						ConfigManager.Config.MovieRecord.RecordFrom
					);
					RecordApi.MovieRecord(options);
				}
			}
		}

		private static string GetOutputFilename(string folder, string ext)
		{
			DateTime now = DateTime.Now;
			string baseName = EmuApi.GetRomInfo().GetRomName();
			string dateTime = " " + now.ToShortDateString() + " " + now.ToLongTimeString();
			string filename = baseName + dateTime + ext;

			//Replace any illegal chars with _
			filename = string.Join("_", filename.Split(Path.GetInvalidFileNameChars()));

			return Path.Combine(folder, filename);
		}

		private async void OpenFile()
		{
			string? filename = await FileDialogHelper.OpenFile(null, _mainWindow, FileDialogHelper.RomExt);
			if(filename != null) {
				LoadRomHelper.LoadFile(filename);
			}
		}

		//TODO
		/*public void SetRegion(ConsoleRegion region)
		{
			/*ConfigManager.Config.Emulation.Region = region;
			ConfigManager.Config.Emulation.ApplyConfig();
			ConfigManager.SaveConfig();
		}*/

		//TODO
		/*public void ToggleBilinearInterpolation()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.UseBilinearInterpolation);
		}

		public void ToggleBlendHighResolutionModes()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.BlendHighResolutionModes);
		}

		private void ToggleBgLayer0()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.HideBgLayer0);
			EmuApi.DisplayMessage("Debug", ResourceHelper.GetMessage(ConfigManager.Config.Video.HideBgLayer0 ? "BgLayerDisabled" : "BgLayerEnabled", "1"));
		}

		private void ToggleBgLayer1()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.HideBgLayer1);
			EmuApi.DisplayMessage("Debug", ResourceHelper.GetMessage(ConfigManager.Config.Video.HideBgLayer1 ? "BgLayerDisabled" : "BgLayerEnabled", "2"));
		}

		private void ToggleBgLayer2()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.HideBgLayer2);
			EmuApi.DisplayMessage("Debug", ResourceHelper.GetMessage(ConfigManager.Config.Video.HideBgLayer2 ? "BgLayerDisabled" : "BgLayerEnabled", "3"));
		}

		private void ToggleBgLayer3()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.HideBgLayer3);
			EmuApi.DisplayMessage("Debug", ResourceHelper.GetMessage(ConfigManager.Config.Video.HideBgLayer3 ? "BgLayerDisabled" : "BgLayerEnabled", "4"));
		}

		private void ToggleSprites()
		{
			InvertConfigFlag(ref ConfigManager.Config.Video.HideSprites);
			EmuApi.DisplayMessage("Debug", ResourceHelper.GetMessage(ConfigManager.Config.Video.HideSprites ? "SpriteLayerDisabled" : "SpriteLayerEnabled"));
		}
		
		private void EnableAllLayers()
		{
			ConfigManager.Config.Video.HideBgLayer0 = false;
			ConfigManager.Config.Video.HideBgLayer1 = false;
			ConfigManager.Config.Video.HideBgLayer2 = false;
			ConfigManager.Config.Video.HideBgLayer3 = false;
			ConfigManager.Config.Video.HideSprites = false;
			ConfigManager.Config.ApplyConfig();
			ConfigManager.ApplyChanges();

			EmuApi.DisplayMessage("Debug", ResourceHelper.GetMessage("AllLayersEnabled"));
		}*/

		private void SetEmulationSpeed(uint emulationSpeed)
		{
			ConfigManager.Config.Emulation.EmulationSpeed = emulationSpeed;
			ConfigManager.Config.Emulation.ApplyConfig();

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

		private void ToggleCheats()
		{
			ConfigManager.Config.Cheats.DisableAllCheats = !ConfigManager.Config.Cheats.DisableAllCheats;
			CheatCodes.ApplyCheats();
		}
		
		private void ToggleOsd()
		{
			ConfigManager.Config.Preferences.DisableOsd = !ConfigManager.Config.Preferences.DisableOsd;
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		private void ToggleFps()
		{
			ConfigManager.Config.Preferences.ShowFps = !ConfigManager.Config.Preferences.ShowFps;
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		private void ToggleAudio()
		{
			ConfigManager.Config.Audio.EnableAudio = !ConfigManager.Config.Audio.EnableAudio;
			ConfigManager.Config.Audio.ApplyConfig();
		}

		private void IncreaseVolume()
		{
			ConfigManager.Config.Audio.MasterVolume = (uint)Math.Min(100, (int)ConfigManager.Config.Audio.MasterVolume + 5);
			ConfigManager.Config.Audio.ApplyConfig();
		}

		private void DecreaseVolume()
		{
			ConfigManager.Config.Audio.MasterVolume = (uint)Math.Max(0, (int)ConfigManager.Config.Audio.MasterVolume - 5);
			ConfigManager.Config.Audio.ApplyConfig();
		}

		private void ToggleFrameCounter()
		{
			ConfigManager.Config.Preferences.ShowFrameCounter = !ConfigManager.Config.Preferences.ShowFrameCounter;
			ConfigManager.Config.Preferences.ApplyConfig();
		}
		
		private void ToggleGameTimer()
		{
			ConfigManager.Config.Preferences.ShowGameTimer = !ConfigManager.Config.Preferences.ShowGameTimer;
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		private void ToggleAlwaysOnTop()
		{
			ConfigManager.Config.Preferences.AlwaysOnTop = !ConfigManager.Config.Preferences.AlwaysOnTop;
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		private void ToggleDebugInfo()
		{
			ConfigManager.Config.Preferences.ShowDebugInfo = !ConfigManager.Config.Preferences.ShowDebugInfo;
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		public static async void Reset()
		{
			if(!ConfigManager.Config.Preferences.ConfirmExitResetPower || await MesenMsgBox.Show(null, "ConfirmReset", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
				EmuApi.Reset();
			}
		}

		public static async void PowerCycle()
		{
			if(!ConfigManager.Config.Preferences.ConfirmExitResetPower || await MesenMsgBox.Show(null, "ConfirmPowerCycle", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
				EmuApi.PowerCycle();
			}
		}

		public static async void PowerOff()
		{
			if(!ConfigManager.Config.Preferences.ConfirmExitResetPower || await MesenMsgBox.Show(null, "ConfirmPowerOff", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
				EmuApi.PowerOff();
			}
		}
	}
}
