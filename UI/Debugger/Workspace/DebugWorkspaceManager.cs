using Mesen.GUI.Debugger.Integration;
using Mesen.GUI.Debugger.Labels;
using System;
using System.Collections.Generic;
using System.IO;
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
				_workspace.CpuLabels = new List<CodeLabel>(LabelManager.GetLabels(CpuType.Cpu));
				_workspace.SpcLabels = new List<CodeLabel>(LabelManager.GetLabels(CpuType.Spc));
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
				_workspace.CpuLabels = new List<CodeLabel>();
				_workspace.SpcLabels = new List<CodeLabel>();
				WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries = _workspace.WatchValues;
				WatchManager.GetWatchManager(CpuType.Spc).WatchEntries = _workspace.SpcWatchValues;
				BreakpointManager.SetBreakpoints(_workspace.Breakpoints);
				LabelManager.SetDefaultLabels();
				LabelManager.RefreshLabels();
				_workspace.Save();
				Clear();
			}
		}

		private static void ResetLabels()
		{
			if(_workspace != null) {
				_workspace.CpuLabels = new List<CodeLabel>();
				_workspace.SpcLabels = new List<CodeLabel>();
				LabelManager.ResetLabels();
				LabelManager.SetDefaultLabels();
				LabelManager.RefreshLabels();
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
				LabelManager.SetLabels(_workspace.CpuLabels);
				LabelManager.SetLabels(_workspace.SpcLabels);
				LabelManager.SetDefaultLabels();

				ImportDbgFile();
				LabelManager.RefreshLabels();

				//Load breakpoints
				BreakpointManager.SetBreakpoints(_workspace.Breakpoints);
			}
			return _workspace;
		}

		public static void ImportDbgFile()
		{
			RomInfo romInfo = EmuApi.GetRomInfo();
			string dbgPath = Path.Combine(Path.GetDirectoryName(romInfo.RomPath), romInfo.GetRomName() + ".dbg");
			if(File.Exists(dbgPath)) {
				DbgImporter import = new DbgImporter();
				ResetLabels();
				import.Import(dbgPath, true);
				LabelManager.RefreshLabels();
			}
		}
	}
}
