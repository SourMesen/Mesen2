using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Config
{
	public class Configuration
	{
		private bool _needToSave = false;

		public string Version = "0.2.0";
		public VideoConfig Video;
		public AudioConfig Audio;
		public InputConfig Input;
		public EmulationConfig Emulation;
		public PreferencesConfig Preferences;
		public DebugInfo Debug;
		public RecentItems RecentFiles;
		public AviRecordConfig AviRecord;
		public MovieRecordConfig MovieRecord;
		public CheatWindowConfig Cheats;
		public NetplayConfig Netplay;
		public Point WindowLocation;
		public Size WindowSize;
		public bool NeedInputReinit = true;
		public DefaultKeyMappingType DefaultKeyMappings = DefaultKeyMappingType.None;

		public Configuration()
		{
			RecentFiles = new RecentItems();
			Debug = new DebugInfo();
			Video = new VideoConfig();
			Audio = new AudioConfig();
			Input = new InputConfig();
			Emulation = new EmulationConfig();
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
			if(_needToSave) {
				Serialize(ConfigManager.ConfigFile);
			}
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
			Preferences.ApplyConfig();
			Debug.Debugger.ApplyConfig();
		}

		public void InitializeDefaults()
		{
			Preferences.InitializeDefaultShortcuts();
			if(NeedInputReinit) {
				Input.InitializeDefaults(DefaultKeyMappings);
				NeedInputReinit = false;
			}
			ConfigManager.ApplyChanges();
		}

		public static Configuration Deserialize(string configFile)
		{
			Configuration config;

			try {
				XmlSerializer xmlSerializer = new XmlSerializer(typeof(Configuration));
				using(TextReader textReader = new StreamReader(configFile)) {
					config = (Configuration)xmlSerializer.Deserialize(textReader);
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
					XmlSerializer xmlSerializer = new XmlSerializer(typeof(Configuration));
					using(TextWriter textWriter = new StreamWriter(configFile)) {
						xmlSerializer.Serialize(textWriter, this);
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
			XmlSerializer xmlSerializer = new XmlSerializer(typeof(Configuration));
			StringWriter stringWriter = new StringWriter();
			xmlSerializer.Serialize(stringWriter, this);

			StringReader stringReader = new StringReader(stringWriter.ToString());
			Configuration config = (Configuration)xmlSerializer.Deserialize(stringReader);
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
