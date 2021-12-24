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
		private bool _needToSave = false;

		public string Version { get; set; } = "0.4.0";
		public VideoConfig Video { get; set; }
		public AudioConfig Audio { get; set; }
		public InputConfig Input { get; set; }
		public EmulationConfig Emulation { get; set; }
		public SnesConfig Snes { get; set; }
		public NesConfig Nes { get; set; }
		public GameboyConfig Gameboy { get; set; }
		public PreferencesConfig Preferences { get; set; }
		public AudioPlayerConfig AudioPlayer { get; set; }
		public DebugConfig Debug { get; set; }
		public RecentItems RecentFiles { get; set; }
		public VideoRecordConfig VideoRecord { get; set; }
		public MovieRecordConfig MovieRecord { get; set; }
		public CheatWindowConfig Cheats { get; set; }
		public NetplayConfig Netplay { get; set; }
		public Point WindowLocation { get; set; }
		public Size WindowSize { get; set; }
		public DefaultKeyMappingType DefaultKeyMappings { get; set; } = DefaultKeyMappingType.Xbox | DefaultKeyMappingType.ArrowKeys;

		public Configuration()
		{
			RecentFiles = new RecentItems();
			Debug = new DebugConfig();
			Video = new VideoConfig();
			Audio = new AudioConfig();
			Input = new InputConfig();
			Emulation = new EmulationConfig();
			Gameboy = new GameboyConfig();
			Snes = new SnesConfig();
			Nes = new NesConfig();
			Preferences = new PreferencesConfig();
			AudioPlayer = new AudioPlayerConfig();
			VideoRecord = new VideoRecordConfig();
			MovieRecord = new MovieRecordConfig();
			Cheats = new CheatWindowConfig();
			Netplay = new NetplayConfig();
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

		public bool NeedToSave
		{
			set
			{
				_needToSave = value;
			}
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
			//if(NeedInputReinit2) {
				//Input.Controllers = new ControllerConfig[5];
				//Preferences.ShortcutKeys = null;

				Snes.InitializeDefaults(DefaultKeyMappings);
				Nes.InitializeDefaults(DefaultKeyMappings);
			//}
			Preferences.InitializeDefaultShortcuts();
			ConfigManager.SaveConfig();
		}

		public static Configuration Deserialize(string configFile)
		{
			Configuration config;

			try {
				using(StreamReader reader = new StreamReader(configFile)) {
					config = JsonSerializer.Deserialize<Configuration>(reader.ReadToEnd(), JsonHelper.Options) ?? new Configuration();
				}
			} catch {
				config = new Configuration();
			}

			return config;
		}

		public void Serialize(string configFile)
		{
			try {
				if(!ConfigManager.DoNotSaveSettings) {
					using(StreamWriter writer = new StreamWriter(configFile)) {
						writer.Write(JsonSerializer.Serialize(this, typeof(Configuration), JsonHelper.Options));
					}
				}
				_needToSave = false;
			} catch {
				//This can sometime fail due to the file being used by another Mesen instance, etc.
				//In this case, the _needToSave flag will still be set, and the config will be saved when the emulator is closed
			}
		}

		public Configuration Clone()
		{
			Configuration config = JsonSerializer.Deserialize<Configuration>(JsonSerializer.Serialize(this, typeof(Configuration))) ?? new Configuration();
			config.NeedToSave = false;
			return config;
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
