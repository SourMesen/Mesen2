using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public partial class Configuration
	{
		private bool _needToSave = false;

		public string Version { get; set; } = "0.4.0";
		public VideoConfig Video { get; set; }
		public AudioConfig Audio { get; set; }
		public InputConfig Input { get; set; }
		public EmulationConfig Emulation { get; set; }
		public GameboyConfig Gameboy { get; set; }
		public PreferencesConfig Preferences { get; set; }
		//public DebugInfo Debug;
		public RecentItems RecentFiles { get; set; }
		public AviRecordConfig AviRecord { get; set; }
		public MovieRecordConfig MovieRecord { get; set; }
		public CheatWindowConfig Cheats { get; set; }
		public NetplayConfig Netplay { get; set; }
		public Point WindowLocation { get; set; }
		public Size WindowSize { get; set; }
		public bool NeedInputReinit2 { get; set; } = true;
		public DefaultKeyMappingType DefaultKeyMappings { get; set; } = DefaultKeyMappingType.Xbox | DefaultKeyMappingType.ArrowKeys;

		public Configuration()
		{
			RecentFiles = new RecentItems();
			//Debug = new DebugInfo();
			Video = new VideoConfig();
			Audio = new AudioConfig();
			Input = new InputConfig();
			Emulation = new EmulationConfig();
			Gameboy = new GameboyConfig();
			Preferences = new PreferencesConfig();
			AviRecord = new AviRecordConfig();
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
			Preferences.ApplyConfig();
			//Debug.Debugger.ApplyConfig();
		}

		public void InitializeDefaults()
		{
			//if(NeedInputReinit2) {
				Input.Controllers = new ControllerConfig[5];
				Preferences.ShortcutKeys1 = null;
				Preferences.ShortcutKeys2 = null;

				Input.InitializeDefaults(DefaultKeyMappings);
				NeedInputReinit2 = false;
			//}
			Preferences.InitializeDefaultShortcuts();
			ConfigManager.SaveConfig();
		}

		public static Configuration Deserialize(string configFile)
		{
			Configuration config;
			JsonSerializerOptions options = new JsonSerializerOptions { Converters = { new TimeSpanConverter(), new JsonStringEnumConverter() }, IgnoreReadOnlyProperties = true };

			try {
				using(StreamReader reader = new StreamReader(configFile)) {
					config = JsonSerializer.Deserialize<Configuration>(reader.ReadToEnd(), options) ?? new Configuration();
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
						JsonSerializerOptions options = new JsonSerializerOptions { Converters = { new TimeSpanConverter(), new JsonStringEnumConverter() }, WriteIndented = true, IgnoreReadOnlyProperties = true };
						writer.Write(JsonSerializer.Serialize(this, typeof(Configuration), options));
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
