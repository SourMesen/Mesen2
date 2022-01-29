using Avalonia;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public partial class Configuration
	{
		private string _fileData = "";

		public string Version { get; set; } = "0.4.0";
		public VideoConfig Video { get; set; } = new();
		public AudioConfig Audio { get; set; } = new();
		public InputConfig Input { get; set; } = new();
		public EmulationConfig Emulation { get; set; } = new();
		public SnesConfig Snes { get; set; } = new();
		public NesConfig Nes { get; set; } = new();
		public GameboyConfig Gameboy { get; set; } = new();
		public PreferencesConfig Preferences { get; set; } = new();
		public AudioPlayerConfig AudioPlayer { get; set; } = new();
		public DebugConfig Debug { get; set; } = new();
		public RecentItems RecentFiles { get; set; } = new();
		public VideoRecordConfig VideoRecord { get; set; } = new();
		public MovieRecordConfig MovieRecord { get; set; } = new();
		public CheatWindowConfig Cheats { get; set; } = new();
		public NetplayConfig Netplay { get; set; } = new();
		public MainWindowConfig MainWindow { get; set; } = new();
		public bool FirstRun { get; set; } = true;
		public DefaultKeyMappingType DefaultKeyMappings { get; set; } = DefaultKeyMappingType.Xbox | DefaultKeyMappingType.ArrowKeys;

		public Configuration()
		{
		}

		~Configuration()
		{
			//Try to save before destruction if we were unable to save at a previous point in time
			Save();
		}

		public void Save()
		{
			Serialize(ConfigManager.ConfigFile);
		}

		public void ApplyConfig()
		{
			Video.ApplyConfig();
			Audio.ApplyConfig();
			Input.ApplyConfig();
			Emulation.ApplyConfig();
			Gameboy.ApplyConfig();
			Nes.ApplyConfig();
			Snes.ApplyConfig();
			Preferences.ApplyConfig();
			AudioPlayer.ApplyConfig();
			Debug.Debugger.ApplyConfig();
			Debug.ScriptWindow.ApplyConfig();
		}

		public void InitializeDefaults()
		{
			if(FirstRun) {
				Snes.InitializeDefaults(DefaultKeyMappings);
				Nes.InitializeDefaults(DefaultKeyMappings);
				Gameboy.InitializeDefaults(DefaultKeyMappings);
				FirstRun = false;
			}
			Preferences.InitializeDefaultShortcuts();
			ConfigManager.Config.Save();
		}

		public static Configuration Deserialize(string configFile)
		{
			Configuration config;

			try {
				string fileData = File.ReadAllText(configFile);
				config = JsonSerializer.Deserialize<Configuration>(fileData, JsonHelper.Options) ?? new Configuration();
				config._fileData = fileData;
			} catch {
				config = new Configuration();
			}

			return config;
		}

		public void Serialize(string configFile)
		{
			try {
				if(!ConfigManager.DoNotSaveSettings) {
					string cfgData = JsonSerializer.Serialize(this, typeof(Configuration), JsonHelper.Options);
					if(_fileData != cfgData) {
						File.WriteAllText(configFile, cfgData);
						_fileData = cfgData;
					}
				}
			} catch {
				//This can sometime fail due to the file being used by another Mesen instance, etc.
				//In this case, the _needToSave flag will still be set, and the config will be saved when the emulator is closed
			}
		}
	}

	[Flags]
	public enum DefaultKeyMappingType
	{
		None = 0,
		Xbox = 1,
		Ps4 = 2,
		WasdKeys = 4,
		ArrowKeys = 8
	}
}
