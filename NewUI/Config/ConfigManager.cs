using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Xml.Serialization;
using System.Text.RegularExpressions;
using System.Reflection;
using Mesen.Interop;
using System.Diagnostics;
using Mesen.Utilities;

namespace Mesen.Config
{
	public static class ConfigManager
	{
		private static Configuration? _config;
		private static string? _homeFolder = null;
		private static object _initLock = new object();

		public static string DefaultPortableFolder { get { return Path.GetDirectoryName(Program.ExePath) ?? "./"; } }
		public static string DefaultDocumentsFolder
		{
			get
			{
				return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "Mesen-SCore");
			}
		}

		public static string DefaultAviFolder { get { return Path.Combine(HomeFolder, "Avi"); } }
		public static string DefaultMovieFolder { get { return Path.Combine(HomeFolder, "Movies"); } }
		public static string DefaultSaveDataFolder { get { return Path.Combine(HomeFolder, "Saves"); } }
		public static string DefaultSaveStateFolder { get { return Path.Combine(HomeFolder, "SaveStates"); } }
		public static string DefaultScreenshotFolder { get { return Path.Combine(HomeFolder, "Screenshots"); } }
		public static string DefaultWaveFolder { get { return Path.Combine(HomeFolder, "Wave"); } }

		public static string GetConfigFile()
		{
			return Path.Combine(HomeFolder, "settings.json");
		}

		public static void CreateConfig(bool portable)
		{
			string homeFolder;
			if(portable) {
				homeFolder = DefaultPortableFolder;
			} else {
				homeFolder = DefaultDocumentsFolder;
			}
			Program.ExtractNativeDependencies(homeFolder);
			_homeFolder = homeFolder;
			LoadConfig();
		}
		
		public static void LoadConfig()
		{
			if(_config == null) {
				lock(_initLock) {
					if(_config == null) {
						if(File.Exists(ConfigFile)) {
							_config = Configuration.Deserialize(ConfigFile);
						} else {
							//Create new config file and save it to disk
							_config = new Configuration();
							_config.Save();
						}
						ConfigManager.ActiveTheme = _config.Preferences.Theme;
					}
				}
			}
		}

		public static MesenTheme ActiveTheme { get; private set; }

		private static void ApplySetting(Type type, object instance, string name, string value)
		{
			FieldInfo[] fields = type.GetFields();
			foreach(FieldInfo info in fields) {
				if(string.Compare(info.Name, name, true) == 0) {
					try {
						if(info.FieldType == typeof(int) || info.FieldType == typeof(uint) || info.FieldType == typeof(double)) {
							if(info.GetCustomAttribute<MinMaxAttribute>() is MinMaxAttribute minMaxAttribute) {
								if(info.FieldType == typeof(int)) {
									if(int.TryParse(value, out int result)) {
										if(result >= (int)minMaxAttribute.Min && result <= (int)minMaxAttribute.Max) {
											info.SetValue(instance, result);
										}
									}
								} else if(info.FieldType == typeof(uint)) {
									if(uint.TryParse(value, out uint result)) {
										if(result >= (uint)(int)minMaxAttribute.Min && result <= (uint)(int)minMaxAttribute.Max) {
											info.SetValue(instance, result);
										}
									}
								} else if(info.FieldType == typeof(double)) {
									if(double.TryParse(value, out double result)) {
										if(result >= (double)minMaxAttribute.Min && result <= (double)minMaxAttribute.Max) {
											info.SetValue(instance, result);
										}
									}
								}
							} else {
								if(info.GetCustomAttribute<ValidValuesAttribute>() is ValidValuesAttribute validValuesAttribute) {
									if(uint.TryParse(value, out uint result)) {
										if(validValuesAttribute.ValidValues.Contains(result)) {
											info.SetValue(instance, result);
										}
									}
								}
							}
						} else if(info.FieldType == typeof(bool)) {
							if(string.Compare(value, "false", true) == 0) {
								info.SetValue(instance, false);
							} else if(string.Compare(value, "true", true) == 0) {
								info.SetValue(instance, true);
							}
						} else if(info.FieldType.IsEnum) {
							int indexOf = Enum.GetNames(info.FieldType).Select((enumValue) => enumValue.ToLower()).ToList().IndexOf(value.ToLower());
							if(indexOf >= 0) {
								info.SetValue(instance, indexOf);
							}
						}
					} catch {
					}
					break;
				}
			}
		}

		public static void ProcessSwitches(List<string> switches)
		{
			Regex regex = new Regex("/([a-z0-9_A-Z.]+)=([a-z0-9_A-Z.\\-]+)");
			foreach(string param in switches) {
				Match match = regex.Match(param);
				if(match.Success) {
					string switchName = match.Groups[1].Value;
					string switchValue = match.Groups[2].Value;

					ApplySetting(typeof(VideoConfig), Config.Video, switchName, switchValue);
					ApplySetting(typeof(AudioConfig), Config.Audio, switchName, switchValue);
					ApplySetting(typeof(EmulationConfig), Config.Emulation, switchName, switchValue);
				}
			}
		}

		public static void ResetHomeFolder()
		{
			_homeFolder = null;
		}

		public static string HomeFolder {
			get
			{
				if(_homeFolder == null) {
					string portableFolder = DefaultPortableFolder;
					string documentsFolder = DefaultDocumentsFolder;

					string portableConfig = Path.Combine(portableFolder, "settings.json");
					if(File.Exists(portableConfig)) {
						_homeFolder = portableFolder;
					} else {
						_homeFolder = documentsFolder;
					}

					Directory.CreateDirectory(_homeFolder);
				}

				return _homeFolder;
			}
		}

		public static string GetFolder(string defaultFolderName, string? overrideFolder, bool useOverride)
		{
			string folder;
			if(useOverride && overrideFolder != null) {
				folder = overrideFolder;
			} else {
				folder = defaultFolderName;
			}

			try {
				if(!Directory.Exists(folder)) {
					Directory.CreateDirectory(folder);
				}
			} catch {
				//If the folder doesn't exist and we couldn't create it, use the default folder
				EmuApi.WriteLogEntry("[UI] Folder could not be created: " + folder);
				folder = defaultFolderName;
			}
			return folder;
		}

		public static string AviFolder { get { return GetFolder(DefaultAviFolder, Config.Preferences.AviFolder, Config.Preferences.OverrideAviFolder); } }
		public static string MovieFolder { get { return GetFolder(DefaultMovieFolder, Config.Preferences.MovieFolder, Config.Preferences.OverrideMovieFolder); } }
		public static string SaveFolder { get { return GetFolder(DefaultSaveDataFolder, Config.Preferences.SaveDataFolder, Config.Preferences.OverrideSaveDataFolder); } }
		public static string SaveStateFolder { get { return GetFolder(DefaultSaveStateFolder, Config.Preferences.SaveStateFolder, Config.Preferences.OverrideSaveStateFolder); } }
		public static string ScreenshotFolder { get { return GetFolder(DefaultScreenshotFolder, Config.Preferences.ScreenshotFolder, Config.Preferences.OverrideScreenshotFolder); } }
		public static string WaveFolder { get { return GetFolder(DefaultWaveFolder, Config.Preferences.WaveFolder, Config.Preferences.OverrideWaveFolder); } }

		public static string CheatFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "Cheats"), null, false); } }
		public static string GameConfigFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "GameConfig"), null, false); } }
		public static string SatellaviewFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "Satellaview"), null, false); } }

		public static string DebuggerFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "Debugger"), null, false); } }
		public static string FirmwareFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "Firmware"), null, false); } }
		public static string BackupFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "Backups"), null, false); } }
		public static string TestFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "Tests"), null, false); } }
		public static string HdPackFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "HdPacks"), null, false); } }
		public static string RecentGamesFolder { get { return GetFolder(Path.Combine(ConfigManager.HomeFolder, "RecentGames"), null, false); } }
		public static string FontFolder { get { return GetFolder(Environment.GetFolderPath(Environment.SpecialFolder.Fonts, Environment.SpecialFolderOption.Create), null, false); } }

		public static string ConfigFile
		{
			get
			{
				if(!Directory.Exists(HomeFolder)) {
					Directory.CreateDirectory(HomeFolder);
				}

				return Path.Combine(HomeFolder, "settings.json");
			}
		}

		public static Configuration Config
		{
			get 
			{
				LoadConfig();
				return _config!;
			}
		}

		public static void ResetSettings()
		{
			DefaultKeyMappingType defaultMappings = Config.DefaultKeyMappings;
			_config = new Configuration();
			Config.DefaultKeyMappings = defaultMappings;
			Config.InitializeDefaults();
			Config.Save();
			Config.ApplyConfig();
		}

		public static void RestartMesen()
		{
			ProcessModule? mainModule = Process.GetCurrentProcess().MainModule;
			if(mainModule?.FileName == null) {
				return;
			}
			SingleInstance.Instance.Dispose();
			Process.Start(mainModule.FileName);
		}
	}
}
