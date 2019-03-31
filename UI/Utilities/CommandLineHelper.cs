using Mesen.GUI.Config;
using Mesen.GUI.Emulation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.GUI.Utilities
{
	class CommandLineHelper
	{
		private string[] _args;
		private bool _noVideo;
		private bool _noAudio;
		private bool _noInput;
		//private bool _loadLastSessionRequested;
		private string _movieToRecord;
		private List<string> _luaScriptsToLoad = new List<string>();

		public CommandLineHelper(string[] args)
		{
			_args = args;
		}

		public void ProcessCommandLineArguments(List<string> switches, bool forStartup)
		{
			if(forStartup) {
				_noVideo = switches.Contains("/novideo");
				_noAudio = switches.Contains("/noaudio");
				_noInput = switches.Contains("/noinput");
			}

			if(switches.Contains("/donotsavesettings")) {
				ConfigManager.DoNotSaveSettings = true;
			}

			/*if(switches.Contains("/loadlastsession")) {
				_loadLastSessionRequested = true;
			}*/

			Regex recordMovieCommand = new Regex("/recordmovie=([^\"]+)");
			foreach(string command in switches) {
				Match match = recordMovieCommand.Match(command);
				if(match.Success) {
					string moviePath = match.Groups[1].Value;
					string folder = Path.GetDirectoryName(moviePath);
					if(string.IsNullOrWhiteSpace(folder)) {
						moviePath = Path.Combine(ConfigManager.MovieFolder, moviePath);
					} else if(!Path.IsPathRooted(moviePath)) {
						moviePath = Path.Combine(Program.OriginalFolder, moviePath);
					}
					if(!moviePath.ToLower().EndsWith(".mmo")) {
						moviePath += ".mmo";
					}
					_movieToRecord = moviePath;
					break;
				}
			}
			
			ConfigManager.ProcessSwitches(switches);
		}

		private void ProcessFullscreenSwitch(List<string> switches)
		{
			//TODO
			/*if(switches.Contains("/fullscreen")) {
				double scale = ConfigManager.Config.Video.VideoScale;
				if(!ConfigManager.Config.Video.UseExclusiveFullscreen) {
					//Go into fullscreen mode right away
					SetFullscreenState(true);
				}

				_fullscreenRequested = true;
				foreach(string option in switches) {
					if(option.StartsWith("/videoscale=")) {
						_switchOptionScale = scale;
					}
				}
			}*/
		}

		public void LoadGameFromCommandLine()
		{
			List<string> switches = CommandLineHelper.PreprocessCommandLineArguments(_args, false);

			string romPath;
			CommandLineHelper.GetRomPathFromCommandLine(switches, out romPath, out _luaScriptsToLoad);

			if(romPath != null) {
				EmuRunner.LoadFile(romPath);
			} else {
				if(!EmuRunner.IsRunning()) {
					//When no ROM is loaded, only process Lua scripts if a ROM was specified as a command line param
					_luaScriptsToLoad.Clear();
					_movieToRecord = null;
					//_loadLastSessionRequested = false;
				} else {
					//No game was specified, but a game is running already, load the scripts right away
					ProcessPostLoadCommandSwitches();
				}
			}
		}

		private void ProcessPostLoadCommandSwitches()
		{
			//TODO
			/*if(_luaScriptsToLoad.Count > 0) {
				foreach(string luaScript in _luaScriptsToLoad) {
					frmScript scriptWindow = DebugWindowManager.OpenScriptWindow(true);
					scriptWindow.LoadScriptFile(luaScript);
				}
				_luaScriptsToLoad.Clear();
			}

			if(_movieToRecord != null) {
				if(EmuApi.MovieRecording()) {
					EmuApi.MovieStop();
				}
				RecordMovieOptions options = new RecordMovieOptions(_movieToRecord, "", "", RecordMovieFrom.StartWithSaveData);
				EmuApi.MovieRecord(ref options);
				_movieToRecord = null;
			}

			if(_loadLastSessionRequested) {
				_loadLastSessionRequested = false;
				EmuRunner.LoadLastSession();
			}*/
		}

		public static List<string> PreprocessCommandLineArguments(string[] args, bool toLower)
		{
			var switches = new List<string>();
			for(int i = 0; i < args.Length; i++) {
				if(args[i] != null) {
					string arg = args[i].Trim();
					if(arg.StartsWith("--")) {
						arg = "/" + arg.Substring(2);
					} else if(arg.StartsWith("-")) {
						arg = "/" + arg.Substring(1);
					}

					if(toLower) {
						arg = arg.ToLowerInvariant();
					}
					switches.Add(arg);
				}
			}
			return switches;
		}

		public static void GetRomPathFromCommandLine(List<string> switches, out string romPath, out List<string> luaScriptsToLoad)
		{
			Func<string, bool, string> getValidPath = (string path, bool forLua) => {
				path = path.Trim();
				if(path.ToLower().EndsWith(".lua") == forLua) {
					try {
						if(!File.Exists(path)) {
							//Try loading file as a relative path to the folder Mesen was started from
							path = Path.Combine(Program.OriginalFolder, path);
						}
						if(File.Exists(path)) {
							return path;
						}
					} catch { }
				}
				return null;
			};

			//Check if any Lua scripts were specified
			luaScriptsToLoad = new List<string>();
			foreach(string arg in switches) {
				string path = getValidPath(arg, true);
				if(path != null) {
					luaScriptsToLoad.Add(path);
				}
			}

			romPath = null;
			foreach(string arg in switches) {
				string path = getValidPath(arg, false);
				if(path != null) {
					romPath = path;
					break;
				}
			}
		}
	}
}
