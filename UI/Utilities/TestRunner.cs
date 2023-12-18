using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	internal class TestRunner
	{
		internal static int Run(string[] args)
		{
			ConfigManager.DisableSaveSettings = true;
			CommandLineHelper commandLineHelper = new(args, true);

			if(commandLineHelper.FilesToLoad.Count != 1) {
				//No rom specified
				return -1;
			}

			EmuApi.InitDll();

			int timeout = commandLineHelper.TestRunnerTimeout;
			ConfigManager.Config.ApplyConfig();

			EmuApi.InitializeEmu(ConfigManager.HomeFolder, IntPtr.Zero, IntPtr.Zero, true, true, true, true);
			EmuApi.Pause();

			if(!EmuApi.LoadRom(commandLineHelper.FilesToLoad[0], string.Empty)) {
				return -1;
			}

			DebugWorkspaceManager.Load();

			foreach(string luaScript in commandLineHelper.LuaScriptsToLoad) {
				try {
					string script = File.ReadAllText(luaScript);
					DebugApi.LoadScript(luaScript, Path.GetDirectoryName(luaScript) ?? Program.OriginalFolder, script);
				} catch { }
			}

			ConfigApi.SetEmulationFlag(EmulationFlags.ConsoleMode, true);
			ConfigApi.SetEmulationFlag(EmulationFlags.MaximumSpeed, true);
			EmuApi.Resume();

			int result = -1;
			Stopwatch sw = Stopwatch.StartNew();
			while(sw.ElapsedMilliseconds < timeout * 1000) {
				System.Threading.Thread.Sleep(100);

				if(!EmuApi.IsRunning()) {
					result = EmuApi.GetStopCode();
					break;
				}
			}

			EmuApi.Stop();
			EmuApi.Release();
			return result;
		}
	}
}
