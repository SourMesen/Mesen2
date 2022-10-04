using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Utilities;

public class CommandLineHelper
{
	public bool NoVideo { get; private set; }
	public bool NoAudio { get; private set; }
	public bool NoInput { get; private set; }
	public bool Fullscreen { get; private set; }
	public bool LoadLastSessionRequested { get; private set; }
	public string? MovieToRecord { get; private set; } = null;
	public List<string> LuaScriptsToLoad { get; private set; } = new();
	public List<string> FilesToLoad { get; private set; } = new();

	public CommandLineHelper(string[] args, bool forStartup)
	{
		ProcessCommandLineArgs(args, forStartup);
	}

	private void ProcessCommandLineArgs(string[] args, bool forStartup)
	{
		foreach(string arg in args) {
			string absPath;
			if(Path.IsPathRooted(arg)) {
				absPath = arg;
			} else {
				absPath = Path.GetFullPath(arg, Program.OriginalFolder);
			}

			if(File.Exists(absPath)) {
				switch(Path.GetExtension(absPath).ToLowerInvariant()) {
					case ".lua": LuaScriptsToLoad.Add(absPath); break;
					default: FilesToLoad.Add(absPath); break;
				}
			} else {
				if(!forStartup) {
					continue;
				}

				string switchArg = ConvertArg(arg).ToLowerInvariant();
				switch(switchArg) {
					case "novideo": NoVideo = true; break;
					case "noaudio": NoAudio = true; break;
					case "noinput": NoInput = true; break;
					case "fullscreen": Fullscreen = true; break;
					case "donotsavesettings": ConfigManager.DisableSaveSettings = true; break;
					case "loadlastsession": LoadLastSessionRequested = true; break;
					default:
						if(switchArg.StartsWith("recordmovie=")) {
							string[] values = switchArg.Split('=');
							if(values.Length <= 1) {
								//invalid
								continue;
							}
							string moviePath = values[1];
							string? folder = Path.GetDirectoryName(moviePath);
							if(string.IsNullOrWhiteSpace(folder)) {
								moviePath = Path.Combine(ConfigManager.MovieFolder, moviePath);
							} else if(!Path.IsPathRooted(moviePath)) {
								moviePath = Path.Combine(Program.OriginalFolder, moviePath);
							}
							if(!moviePath.ToLower().EndsWith("." + FileDialogHelper.MesenMovieExt)) {
								moviePath += "." + FileDialogHelper.MesenMovieExt;
							}
							MovieToRecord = moviePath;
						} else {
							ConfigManager.ProcessSwitch(switchArg);
						}
						break;
				}
			}
		}
	}

	public void OnAfterInit(MainWindow wnd)
	{
		if(Fullscreen && FilesToLoad.Count == 0) {
			wnd.ToggleFullscreen();
			Fullscreen = false;
		}
	}

	private static string ConvertArg(string arg)
	{
		arg = arg.Trim();
		if(arg.StartsWith("--")) {
			arg = arg.Substring(2);
		} else if(arg.StartsWith("-") || arg.StartsWith("/")) {
			arg = arg.Substring(1);
		}
		return arg;
	}

	public void ProcessPostLoadCommandSwitches(MainWindow wnd)
	{
		if(LuaScriptsToLoad.Count > 0) {
			foreach(string luaScript in LuaScriptsToLoad) {
				ScriptWindowViewModel model = new();
				model.LoadScript(luaScript);
				DebugWindowManager.OpenDebugWindow(() => new ScriptWindow(model));
			}
		}

		if(MovieToRecord != null) {
			if(RecordApi.MovieRecording()) {
				RecordApi.MovieStop();
			}
			RecordMovieOptions options = new RecordMovieOptions(MovieToRecord, "", "", RecordMovieFrom.StartWithSaveData);
			RecordApi.MovieRecord(options);
		}

		if(Fullscreen) {
			wnd.ToggleFullscreen();
		}

		if(LoadLastSessionRequested) {
			Task.Run(() => {
				EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = Config.Shortcuts.EmulatorShortcut.LoadLastSession });
			});
		}
	}

	public void LoadFiles()
	{
		foreach(string file in FilesToLoad) {
			LoadRomHelper.LoadFile(file);
		}
	}
}
