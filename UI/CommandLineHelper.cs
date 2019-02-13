using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI
{
	class CommandLineHelper
	{
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
