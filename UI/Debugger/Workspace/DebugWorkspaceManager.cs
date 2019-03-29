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
				_workspace.WatchValues = new List<string>(WatchManager.WatchEntries);
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
				WatchManager.WatchEntries = _workspace.WatchValues;
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
				WatchManager.WatchEntries = _workspace.WatchValues;

				//Load breakpoints
				BreakpointManager.SetBreakpoints(_workspace.Breakpoints);
			}
			return _workspace;
		}
	}
}
