using Avalonia.Controls;
using Avalonia.Interactivity;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Interop;
using Mesen.Localization;
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
				case EmulatorShortcut.ReloadRom: ReloadRom(); break;
				case EmulatorShortcut.PowerOff: PowerOff(); break;
				case EmulatorShortcut.Exit: _mainWindow.Close(); break;

				case EmulatorShortcut.ToggleAudio: ToggleAudio(); break;
				case EmulatorShortcut.IncreaseVolume: IncreaseVolume(); break;
				case EmulatorShortcut.DecreaseVolume: DecreaseVolume(); break;

				case EmulatorShortcut.PreviousTrack: GoToPreviousTrack(); break;
				case EmulatorShortcut.NextTrack: GoToNextTrack(); break;

				case EmulatorShortcut.ToggleFps: ToggleFps(); break;
				case EmulatorShortcut.ToggleGameTimer: ToggleGameTimer(); break;
				case EmulatorShortcut.ToggleFrameCounter: ToggleFrameCounter(); break;
				case EmulatorShortcut.ToggleLagCounter: ToggleLagCounter(); break;
				case EmulatorShortcut.ToggleOsd: ToggleOsd(); break;
				
				case EmulatorShortcut.ToggleAlwaysOnTop: ToggleAlwaysOnTop(); break;

				case EmulatorShortcut.ToggleDebugInfo: ToggleDebugInfo(); break;
				case EmulatorShortcut.ToggleCheats: ToggleCheats(); break;
				case EmulatorShortcut.MaxSpeed: ToggleMaxSpeed(); break;

				case EmulatorShortcut.ToggleFullscreen: _mainWindow.ToggleFullscreen(); break;

				case EmulatorShortcut.OpenFile: OpenFile(); break;
				case EmulatorShortcut.IncreaseSpeed: IncreaseEmulationSpeed(); break;
				case EmulatorShortcut.DecreaseSpeed: DecreaseEmulationSpeed(); break;

				case EmulatorShortcut.SetScale1x: _mainWindow.SetScale(1); break;
				case EmulatorShortcut.SetScale2x: _mainWindow.SetScale(2); break;
				case EmulatorShortcut.SetScale3x: _mainWindow.SetScale(3); break;
				case EmulatorShortcut.SetScale4x: _mainWindow.SetScale(4); break;
				case EmulatorShortcut.SetScale5x: _mainWindow.SetScale(5); break;
				case EmulatorShortcut.SetScale6x: _mainWindow.SetScale(6); break;
				case EmulatorShortcut.SetScale7x: _mainWindow.SetScale(7); break;
				case EmulatorShortcut.SetScale8x: _mainWindow.SetScale(8); break;
				case EmulatorShortcut.SetScale9x: _mainWindow.SetScale(9); break;
				case EmulatorShortcut.SetScale10x: _mainWindow.SetScale(10); break;

				case EmulatorShortcut.ToggleBgLayer1: ToggleVideoLayer(VideoLayer.Bg1); break;
				case EmulatorShortcut.ToggleBgLayer2: ToggleVideoLayer(VideoLayer.Bg2); break;
				case EmulatorShortcut.ToggleBgLayer3: ToggleVideoLayer(VideoLayer.Bg3); break;
				case EmulatorShortcut.ToggleBgLayer4: ToggleVideoLayer(VideoLayer.Bg4); break;
				case EmulatorShortcut.ToggleSprites1: ToggleVideoLayer(VideoLayer.Sprite1); break;
				case EmulatorShortcut.ToggleSprites2: ToggleVideoLayer(VideoLayer.Sprite2); break;
				case EmulatorShortcut.EnableAllLayers: EnableAllLayers(); break;

				case EmulatorShortcut.ResetLagCounter: InputApi.ResetLagCounter(); break;

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
					LoadRomHelper.LoadRecentGame(filename, true);
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
				RecordApi.AviRecord(filename, new RecordAviOptions() {
					Codec = ConfigManager.Config.VideoRecord.Codec,
					CompressionLevel = ConfigManager.Config.VideoRecord.CompressionLevel,
					RecordSystemHud = ConfigManager.Config.VideoRecord.RecordSystemHud,
					RecordInputHud = ConfigManager.Config.VideoRecord.RecordInputHud
				});
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
						GetOutputFilename(ConfigManager.MovieFolder, "." + FileDialogHelper.MesenMovieExt),
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
			string? initialFolder = null;
			if(ConfigManager.Config.Preferences.OverrideGameFolder && Directory.Exists(ConfigManager.Config.Preferences.GameFolder)) {
				initialFolder = ConfigManager.Config.Preferences.GameFolder;
			} else {
				initialFolder = ConfigManager.Config.RecentFiles.Items.Count > 0 ? ConfigManager.Config.RecentFiles.Items[0].RomFile.Folder : null;
			}

			string? filename = await FileDialogHelper.OpenFile(initialFolder, _mainWindow, FileDialogHelper.RomExt);
			if(filename != null) {
				LoadRomHelper.LoadFile(filename);
			}
		}

		enum VideoLayer
		{
			Bg1,
			Bg2,
			Bg3,
			Bg4,
			Sprite1,
			Sprite2,
		}

		private (Func<bool>? get, Action<bool>? set) GetFlagSetterGetter(VideoLayer layer)
		{
			switch(MainWindowViewModel.Instance.RomInfo.ConsoleType) {
				case ConsoleType.Snes:
					switch(layer) {
						case VideoLayer.Bg1: return (() => ConfigManager.Config.Snes.HideBgLayer1, (val) => ConfigManager.Config.Snes.HideBgLayer1 = val);
						case VideoLayer.Bg2: return (() => ConfigManager.Config.Snes.HideBgLayer2, (val) => ConfigManager.Config.Snes.HideBgLayer2 = val);
						case VideoLayer.Bg3: return (() => ConfigManager.Config.Snes.HideBgLayer3, (val) => ConfigManager.Config.Snes.HideBgLayer3 = val);
						case VideoLayer.Bg4: return (() => ConfigManager.Config.Snes.HideBgLayer4, (val) => ConfigManager.Config.Snes.HideBgLayer4 = val);
						case VideoLayer.Sprite1: return (() => ConfigManager.Config.Snes.HideSprites, (val) => ConfigManager.Config.Snes.HideSprites = val);
					}
					break;

				case ConsoleType.Nes:
					switch(layer) {
						case VideoLayer.Bg1: return (() => ConfigManager.Config.Nes.DisableBackground, (val) => ConfigManager.Config.Nes.DisableBackground = val);
						case VideoLayer.Sprite1: return (() => ConfigManager.Config.Nes.DisableSprites, (val) => ConfigManager.Config.Nes.DisableSprites = val);
					}
					break;

				case ConsoleType.Gameboy:
					switch(layer) {
						case VideoLayer.Bg1: return (() => ConfigManager.Config.Gameboy.DisableBackground, (val) => ConfigManager.Config.Gameboy.DisableBackground = val);
						case VideoLayer.Sprite1: return (() => ConfigManager.Config.Gameboy.DisableSprites, (val) => ConfigManager.Config.Gameboy.DisableSprites = val);
					}
					break;

				case ConsoleType.Gba:
					switch(layer) {
						case VideoLayer.Bg1: return (() => ConfigManager.Config.Gba.HideBgLayer1, (val) => ConfigManager.Config.Gba.HideBgLayer1 = val);
						case VideoLayer.Bg2: return (() => ConfigManager.Config.Gba.HideBgLayer2, (val) => ConfigManager.Config.Gba.HideBgLayer2 = val);
						case VideoLayer.Bg3: return (() => ConfigManager.Config.Gba.HideBgLayer3, (val) => ConfigManager.Config.Gba.HideBgLayer3 = val);
						case VideoLayer.Bg4: return (() => ConfigManager.Config.Gba.HideBgLayer4, (val) => ConfigManager.Config.Gba.HideBgLayer4 = val);
						case VideoLayer.Sprite1: return (() => ConfigManager.Config.Gba.DisableSprites, (val) => ConfigManager.Config.Gba.DisableSprites = val);
					}
					break;

				case ConsoleType.PcEngine:
					switch(layer) {
						case VideoLayer.Bg1: return (() => ConfigManager.Config.PcEngine.DisableBackground, (val) => ConfigManager.Config.PcEngine.DisableBackground = val);
						case VideoLayer.Bg2: return (() => ConfigManager.Config.PcEngine.DisableBackgroundVdc2, (val) => ConfigManager.Config.PcEngine.DisableBackgroundVdc2 = val);
						case VideoLayer.Sprite1: return (() => ConfigManager.Config.PcEngine.DisableSprites, (val) => ConfigManager.Config.PcEngine.DisableSprites = val);
						case VideoLayer.Sprite2: return (() => ConfigManager.Config.PcEngine.DisableSpritesVdc2, (val) => ConfigManager.Config.PcEngine.DisableSpritesVdc2 = val);
					}
					break;

				case ConsoleType.Sms:
					if(MainWindowViewModel.Instance.RomInfo.Format == RomFormat.ColecoVision) {
						switch(layer) {
							case VideoLayer.Bg1: return (() => ConfigManager.Config.Cv.DisableBackground, (val) => ConfigManager.Config.Cv.DisableBackground = val);
							case VideoLayer.Sprite1: return (() => ConfigManager.Config.Cv.DisableSprites, (val) => ConfigManager.Config.Cv.DisableSprites = val);
						}
					} else {
						switch(layer) {
							case VideoLayer.Bg1: return (() => ConfigManager.Config.Sms.DisableBackground, (val) => ConfigManager.Config.Sms.DisableBackground = val);
							case VideoLayer.Sprite1: return (() => ConfigManager.Config.Sms.DisableSprites, (val) => ConfigManager.Config.Sms.DisableSprites = val);
						}
					}
					break;

				case ConsoleType.Ws:
					switch(layer) {
						case VideoLayer.Bg1: return (() => ConfigManager.Config.Ws.HideBgLayer1, (val) => ConfigManager.Config.Ws.HideBgLayer1 = val);
						case VideoLayer.Bg2: return (() => ConfigManager.Config.Ws.HideBgLayer2, (val) => ConfigManager.Config.Ws.HideBgLayer2 = val);
						case VideoLayer.Sprite1: return (() => ConfigManager.Config.Ws.DisableSprites, (val) => ConfigManager.Config.Ws.DisableSprites = val);
					}
					break;
			}

			return (null, null);
		}

		private void ToggleVideoLayer(VideoLayer layer)
		{
			if(!EmuApi.IsRunning()) {
				return;
			}

			(Func<bool>? get, Action<bool>? set) = GetFlagSetterGetter(layer);
			if(get != null && set != null) {
				set(!get());
				DisplayMessageHelper.DisplayMessage("Debug", ResourceHelper.GetMessage(get() ? "VideoLayerDisabled" : "VideoLayerEnabled", ResourceHelper.GetEnumText(layer)));
				UpdateAllCoreConfig();
			}
		}

		private void EnableAllLayers()
		{
			ConfigManager.Config.Snes.HideBgLayer1 = false;
			ConfigManager.Config.Snes.HideBgLayer2 = false;
			ConfigManager.Config.Snes.HideBgLayer3 = false;
			ConfigManager.Config.Snes.HideBgLayer4 = false;
			ConfigManager.Config.Snes.HideSprites = false;
			ConfigManager.Config.Nes.DisableBackground = false;
			ConfigManager.Config.Nes.DisableSprites = false;
			ConfigManager.Config.Gameboy.DisableBackground = false;
			ConfigManager.Config.Gameboy.DisableSprites = false;
			ConfigManager.Config.Gba.HideBgLayer1 = false;
			ConfigManager.Config.Gba.HideBgLayer2 = false;
			ConfigManager.Config.Gba.HideBgLayer3 = false;
			ConfigManager.Config.Gba.HideBgLayer4 = false;
			ConfigManager.Config.Gba.DisableSprites = false;
			ConfigManager.Config.PcEngine.DisableBackground = false;
			ConfigManager.Config.PcEngine.DisableBackgroundVdc2 = false;
			ConfigManager.Config.PcEngine.DisableSprites = false;
			ConfigManager.Config.PcEngine.DisableSpritesVdc2 = false;
			ConfigManager.Config.Sms.DisableBackground = false;
			ConfigManager.Config.Sms.DisableSprites = false;
			ConfigManager.Config.Ws.HideBgLayer1 = false;
			ConfigManager.Config.Ws.HideBgLayer2 = false;
			ConfigManager.Config.Ws.DisableSprites = false;
			UpdateAllCoreConfig();
			DisplayMessageHelper.DisplayMessage("Debug", ResourceHelper.GetMessage("AllLayersEnabled"));
		}

		private void UpdateAllCoreConfig()
		{
			ConfigManager.Config.Snes.ApplyConfig();
			ConfigManager.Config.Nes.ApplyConfig();
			ConfigManager.Config.Gameboy.ApplyConfig();
			ConfigManager.Config.Gba.ApplyConfig();
			ConfigManager.Config.PcEngine.ApplyConfig();
			ConfigManager.Config.Sms.ApplyConfig();
			ConfigManager.Config.Cv.ApplyConfig();
			ConfigManager.Config.Ws.ApplyConfig();
		}

		private void SetEmulationSpeed(uint emulationSpeed)
		{
			ConfigManager.Config.Emulation.EmulationSpeed = emulationSpeed;
			ConfigManager.Config.Emulation.ApplyConfig();

			if(emulationSpeed == 0) {
				DisplayMessageHelper.DisplayMessage("EmulationSpeed", "EmulationMaximumSpeed");
			} else {
				DisplayMessageHelper.DisplayMessage("EmulationSpeed", "EmulationSpeedPercent", emulationSpeed.ToString());
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
			if(MainWindowModel.AudioPlayer == null) {
				ConfigManager.Config.Audio.MasterVolume = (uint)Math.Min(100, (int)ConfigManager.Config.Audio.MasterVolume + 5);
				ConfigManager.Config.Audio.ApplyConfig();
			} else {
				ConfigManager.Config.AudioPlayer.Volume = (uint)Math.Min(100, (int)ConfigManager.Config.AudioPlayer.Volume + 5);
				ConfigManager.Config.AudioPlayer.ApplyConfig();
			}
		}

		private void DecreaseVolume()
		{
			if(MainWindowModel.AudioPlayer == null) {
				ConfigManager.Config.Audio.MasterVolume = (uint)Math.Max(0, (int)ConfigManager.Config.Audio.MasterVolume - 5);
				ConfigManager.Config.Audio.ApplyConfig();
			} else {
				ConfigManager.Config.AudioPlayer.Volume = (uint)Math.Max(0, (int)ConfigManager.Config.AudioPlayer.Volume - 5);
				ConfigManager.Config.AudioPlayer.ApplyConfig();
			}
		}

		private void GoToPreviousTrack()
		{
			if(MainWindowModel.AudioPlayer != null) {
				EmuApi.ProcessAudioPlayerAction(new AudioPlayerActionParams() { Action = AudioPlayerAction.PrevTrack });
			}
		}

		private void GoToNextTrack()
		{
			if(MainWindowModel.AudioPlayer != null) {
				EmuApi.ProcessAudioPlayerAction(new AudioPlayerActionParams() { Action = AudioPlayerAction.NextTrack });
			}
		}

		private void ToggleFrameCounter()
		{
			ConfigManager.Config.Preferences.ShowFrameCounter = !ConfigManager.Config.Preferences.ShowFrameCounter;
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		private void ToggleLagCounter()
		{
			ConfigManager.Config.Preferences.ShowLagCounter = !ConfigManager.Config.Preferences.ShowLagCounter;
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

		public static async void ReloadRom()
		{
			if(!ConfigManager.Config.Preferences.ConfirmExitResetPower || await MesenMsgBox.Show(null, "ConfirmReloadRom", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
				EmuApi.ReloadRom();
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
