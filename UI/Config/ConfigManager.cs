using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;
using System.Reflection;
using Mesen.Interop;
using System.Diagnostics;
using Mesen.Utilities;
using Avalonia.Controls;

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
				Environment.SpecialFolder folder = OperatingSystem.IsWindows() ? Environment.SpecialFolder.MyDocuments : Environment.SpecialFolder.ApplicationData;
				return Path.Combine(Environment.GetFolderPath(folder, Environment.SpecialFolderOption.Create), "Mesen2");
			}
		}

		public static string DefaultAviFolder { get { return Path.Combine(HomeFolder, "Avi"); } }
		public static string DefaultMovieFolder { get { return Path.Combine(HomeFolder, "Movies"); } }
		public static string DefaultSaveDataFolder { get { return Path.Combine(HomeFolder, "Saves"); } }
		public static string DefaultSaveStateFolder { get { return Path.Combine(HomeFolder, "SaveStates"); } }
		public static string DefaultScreenshotFolder { get { return Path.Combine(HomeFolder, "Screenshots"); } }
		public static string DefaultWaveFolder { get { return Path.Combine(HomeFolder, "Wave"); } }

		public static bool DisableSaveSettings { get; internal set; }

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
			DependencyHelper.ExtractNativeDependencies(homeFolder);
			_homeFolder = homeFolder;
			Config.Save();
		}
		
		public static void LoadConfig()
		{
			if(_config == null) {
				lock(_initLock) {
					if(_config == null) {
						if(File.Exists(ConfigFile) && !Design.IsDesignMode) {
							_config = Configuration.Deserialize(ConfigFile);
						} else {
							_config = Configuration.CreateConfig();
						}
						ConfigManager.ActiveTheme = _config.Preferences.Theme;
					}
				}
			}
		}

		public static MesenTheme ActiveTheme { get; set; }

		private static bool ApplySetting(object instance, PropertyInfo property, string value)
		{
			Type t = property.PropertyType;
			try {
				if(!property.CanWrite) {
					return false;
				}

				if(t == typeof(int) || t == typeof(uint) || t == typeof(double)) {
					if(property.GetCustomAttribute<MinMaxAttribute>() is MinMaxAttribute minMaxAttribute) {
						if(t == typeof(int)) {
							if(int.TryParse(value, out int result)) {
								if(result >= (int)minMaxAttribute.Min && result <= (int)minMaxAttribute.Max) {
									property.SetValue(instance, result);
								} else {
									return false;
								}
							}
						} else if(t == typeof(uint)) {
							if(uint.TryParse(value, out uint result)) {
								if(result >= (uint)(int)minMaxAttribute.Min && result <= (uint)(int)minMaxAttribute.Max) {
									property.SetValue(instance, result);
								} else {
									return false;
								}
							}
						} else if(t == typeof(double)) {
							if(double.TryParse(value, out double result)) {
								if(result >= (double)minMaxAttribute.Min && result <= (double)minMaxAttribute.Max) {
									property.SetValue(instance, result);
								} else {
									return false;
								}
							}
						}
					}
				} else if(t == typeof(bool)) {
					if(bool.TryParse(value, out bool boolValue)) {
						property.SetValue(instance, boolValue);
					} else {
						return false;
					}
				} else if(t.IsEnum) {
					if(Enum.TryParse(t, value, true, out object? enumValue)) {
						if(property.GetCustomAttribute<ValidValuesAttribute>() is ValidValuesAttribute validValuesAttribute) {
							if(validValuesAttribute.ValidValues.Contains(enumValue)) {
								property.SetValue(instance, enumValue);
							} else {
								return false;
							}
						} else {
							property.SetValue(instance, enumValue);
						}
					} else {
						return false;
					}
				}
			} catch {
				return false;
			}
			return true;
		}

		public static bool ProcessSwitch(string switchArg)
		{
			Regex regex = new Regex("([a-z0-9_A-Z.]+)=([a-z0-9_A-Z.\\-]+)");
			Match match = regex.Match(switchArg);
			if(match.Success) {
				string[] switchPath = match.Groups[1].Value.Split(".");
				string switchValue = match.Groups[2].Value;

				object? cfg = ConfigManager.Config;
				PropertyInfo? property;
				for(int i = 0; i < switchPath.Length; i++) {
					property = cfg.GetType().GetProperty(switchPath[i], BindingFlags.Public | BindingFlags.Instance | BindingFlags.IgnoreCase);
					if(property == null) {
						//Invalid switch name
						return false;
					}

					if(i < switchPath.Length - 1) {
						cfg = property.GetValue(cfg);
						if(cfg == null) {
							//Invalid
							return false;
						}
					} else {
						return ApplySetting(cfg, property, switchValue);
					}
				}
			}

			return false;
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

		public static void ResetSettings(bool initDefaults = true)
		{
			DefaultKeyMappingType defaultMappings = Config.DefaultKeyMappings;
			if(defaultMappings == DefaultKeyMappingType.None) {
				defaultMappings = DefaultKeyMappingType.Xbox | DefaultKeyMappingType.ArrowKeys;
			}

			_config = Configuration.CreateConfig();
			Config.DefaultKeyMappings = defaultMappings;
			if(initDefaults) {
				Config.InitializeDefaults();
				Config.ConfigUpgrade = (int)ConfigUpgradeHint.NextValue - 1;
			}
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
