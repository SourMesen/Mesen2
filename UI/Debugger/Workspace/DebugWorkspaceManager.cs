using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Workspace
{
	public class DebugWorkspaceManager
	{
		private static DebugWorkspace _workspace;
		private static string _romName;
		private static object _lock = new object();

		public static void SaveWorkspace()
		{
			if(_workspace != null) {
				_workspace.WatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries);
				_workspace.SpcWatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.Spc).WatchEntries);
				_workspace.Breakpoints = new List<Breakpoint>(BreakpointManager.Breakpoints);
				_workspace.Save();
			}
		}

		public static void Clear()
		{
			_workspace = null;
			_romName = null;
		}

		public static void ResetWorkspace()
		{
			if(_workspace != null) {
				_workspace.Breakpoints = new List<Breakpoint>();
				_workspace.WatchValues = new List<string>();
				_workspace.SpcWatchValues = new List<string>();
				WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries = _workspace.WatchValues;
				WatchManager.GetWatchManager(CpuType.Spc).WatchEntries = _workspace.SpcWatchValues;
				BreakpointManager.SetBreakpoints(_workspace.Breakpoints);
				_workspace.Save();
				Clear();
			}
		}

		public static DebugWorkspace GetWorkspace()
		{
			string romName = EmuApi.GetRomInfo().GetRomName();
			if(_workspace == null || _romName != romName) {
				if(_workspace != null) {
					SaveWorkspace();
				}
				_romName = romName;
				_workspace = DebugWorkspace.GetWorkspace();

				//Load watch entries
				WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries = _workspace.WatchValues;
				WatchManager.GetWatchManager(CpuType.Spc).WatchEntries = _workspace.SpcWatchValues;

				//Load breakpoints
				BreakpointManager.SetBreakpoints(_workspace.Breakpoints);
			}
			return _workspace;
		}
	}
}
