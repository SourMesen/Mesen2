using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;

namespace Mesen.Config
{
	public partial class Configuration : ReactiveObject
	{
		private string _fileData = "";

		public string Version { get; set; } = "2.0.0";
		
		[Reactive] public VideoConfig Video { get; set; } = new();
		[Reactive] public AudioConfig Audio { get; set; } = new();
		[Reactive] public InputConfig Input { get; set; } = new();
		[Reactive] public EmulationConfig Emulation { get; set; } = new();
		[Reactive] public SnesConfig Snes { get; set; } = new();
		[Reactive] public NesConfig Nes { get; set; } = new();
		[Reactive] public GameboyConfig Gameboy { get; set; } = new();
		[Reactive] public PcEngineConfig PcEngine { get; set; } = new();
		[Reactive] public PreferencesConfig Preferences { get; set; } = new();
		[Reactive] public AudioPlayerConfig AudioPlayer { get; set; } = new();
		[Reactive] public DebugConfig Debug { get; set; } = new();
		[Reactive] public RecentItems RecentFiles { get; set; } = new();
		[Reactive] public VideoRecordConfig VideoRecord { get; set; } = new();
		[Reactive] public MovieRecordConfig MovieRecord { get; set; } = new();
		[Reactive] public HdPackBuilderConfig HdPackBuilder { get; set; } = new();
		[Reactive] public CheatWindowConfig Cheats { get; set; } = new();
		[Reactive] public NetplayConfig Netplay { get; set; } = new();
		[Reactive] public HistoryViewerConfig HistoryViewer { get; set; } = new();
		[Reactive] public MainWindowConfig MainWindow { get; set; } = new();
		
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
			if(ConfigManager.DisableSaveSettings) {
				//Don't save to disk if command line option to disable setting updates was set
				return;
			}

			Serialize(ConfigManager.ConfigFile);
		}

		public void ApplyConfig()
		{
			Video.ApplyConfig();
			Audio.ApplyConfig();
			Input.ApplyConfig();
			Emulation.ApplyConfig();
			Gameboy.ApplyConfig();
			PcEngine.ApplyConfig();
			Nes.ApplyConfig();
			Snes.ApplyConfig();
			Preferences.ApplyConfig();
			AudioPlayer.ApplyConfig();
			Debug.ApplyConfig();
		}

		public void InitializeFontDefaults()
		{
			if(FirstRun) {
				Preferences.InitializeFontDefaults();

				Debug.Fonts.DisassemblyFont = GetDefaultMonospaceFont();
				Debug.Fonts.MemoryViewerFont = GetDefaultMonospaceFont();
				Debug.Fonts.AssemblerFont = GetDefaultMonospaceFont();
				Debug.Fonts.ScriptWindowFont = GetDefaultMonospaceFont();
				Debug.Fonts.OtherMonoFont = GetDefaultMonospaceFont(true);
				Debug.Fonts.ApplyConfig();
			}
		}

		public void InitializeDefaults()
		{
			if(FirstRun) {
				Snes.InitializeDefaults(DefaultKeyMappings);
				Nes.InitializeDefaults(DefaultKeyMappings);
				Gameboy.InitializeDefaults(DefaultKeyMappings);
				PcEngine.InitializeDefaults(DefaultKeyMappings);
				FirstRun = false;
			}
			Preferences.InitializeDefaultShortcuts();
		}

		private static HashSet<string>? _installedFonts = null;

		private static string FindMatchingFont(string defaultFont, params string[] fontNames)
		{
			if(_installedFonts == null) {
				_installedFonts = new(FontManager.Current.GetInstalledFontFamilyNames());
			}

			foreach(string name in fontNames) {
				if(_installedFonts.Contains(name)) {
					return name;
				}
			}

			return defaultFont;
		}

		public static FontConfig GetDefaultFont()
		{
			if(OperatingSystem.IsWindows()) {
				return new FontConfig() { FontFamily = "Microsoft Sans Serif", FontSize = 11 };
			} else if(OperatingSystem.IsMacOS()) {
				return new FontConfig() { FontFamily = FindMatchingFont("sans-serif", "Microsoft Sans Serif"), FontSize = 11 };
			} else {
				return new FontConfig() { FontFamily = FindMatchingFont("sans-serif", "DejaVu Sans", "Noto Sans"), FontSize = 11 };
			}
		}

		public static FontConfig GetDefaultMenuFont()
		{
			if(OperatingSystem.IsWindows()) {
				return new FontConfig() { FontFamily = "Segoe UI", FontSize = 12 };
			} else if(OperatingSystem.IsMacOS()) {
				return new FontConfig() { FontFamily = FindMatchingFont("sans-serif", "Microsoft Sans Serif"), FontSize = 12 };
			} else {
				return new FontConfig() { FontFamily = FindMatchingFont("sans-serif", "DejaVu Sans", "Noto Sans"), FontSize = 12 };
			}
		}

		public static FontConfig GetDefaultMonospaceFont(bool useSmallFont = false)
		{
			if(OperatingSystem.IsWindows()) {
				return new FontConfig() { FontFamily = "Consolas", FontSize = useSmallFont ? 12 : 14 };
			} else if(OperatingSystem.IsMacOS()) {
				return new FontConfig() { FontFamily = FindMatchingFont("monospace", "PT Mono"), FontSize = useSmallFont ? 11 : 12 };
			} else {
				return new FontConfig() { FontFamily = FindMatchingFont("monospace", "DejaVu Sans Mono", "Noto Sans Mono"), FontSize = 12 };
			}
		}

		public static Configuration Deserialize(string configFile)
		{
			Configuration config;

			try {
				string fileData = File.ReadAllText(configFile);
				config = JsonSerializer.Deserialize<Configuration>(fileData, JsonHelper.Options) ?? new Configuration();
				config._fileData = fileData;
			} catch {
				try {
					//File exists but couldn't be loaded, make a backup of the old settings before we overwrite them
					string? folder = Path.GetDirectoryName(configFile);
					if(folder != null) {
						File.Copy(configFile, Path.Combine(folder, "settings." + DateTime.Now.ToString("yyyy-M-dd_HH-mm-ss") + ".bak"), true);
					}
				} catch { }

				config = new Configuration();
			}

			return config;
		}

		public void Serialize(string configFile)
		{
			try {
				string cfgData = JsonSerializer.Serialize(this, typeof(Configuration), JsonHelper.Options);
				if(_fileData != cfgData && !Design.IsDesignMode) {
					FileHelper.WriteAllText(configFile, cfgData);
					_fileData = cfgData;
				}
			} catch {
				//This can sometime fail due to the file being used by another Mesen instance, etc.
			}
		}

		public void RemoveObsoleteConfig()
		{
			//Clean up configuration to remove any obsolete values that existed in older versions
			for(int i = Preferences.ShortcutKeys.Count - 1; i >= 0; i--) {
				if(Preferences.ShortcutKeys[i].Shortcut >= EmulatorShortcut.LastValidValue) {
					Preferences.ShortcutKeys.RemoveAt(i);
				}
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
