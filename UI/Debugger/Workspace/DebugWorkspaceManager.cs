using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Integration;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Forms;
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
		public delegate void SymbolProviderChangedHandler(ISymbolProvider symbolProvider);
		public static event SymbolProviderChangedHandler SymbolProviderChanged;
		private static DebugWorkspace _workspace;
		private static ISymbolProvider _symbolProvider;
		private static string _romName;
		private static object _lock = new object();

		public static void SaveWorkspace()
		{
			if(_workspace != null) {
				_workspace.WatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries);
				_workspace.SpcWatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.Spc).WatchEntries);
				_workspace.Sa1WatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.Sa1).WatchEntries);
				_workspace.GsuWatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.Gsu).WatchEntries);
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
			LabelManager.ResetLabels();
		}

		public static void ResetWorkspace()
		{
			if(_workspace != null) {
				_workspace.Breakpoints = new List<Breakpoint>();
				_workspace.WatchValues = new List<string>();
				_workspace.SpcWatchValues = new List<string>();
				_workspace.Sa1WatchValues = new List<string>();
				_workspace.GsuWatchValues = new List<string>();
				_workspace.CpuLabels = new List<CodeLabel>();
				_workspace.SpcLabels = new List<CodeLabel>();
				WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries = _workspace.WatchValues;
				WatchManager.GetWatchManager(CpuType.Spc).WatchEntries = _workspace.SpcWatchValues;
				WatchManager.GetWatchManager(CpuType.Sa1).WatchEntries = _workspace.Sa1WatchValues;
				WatchManager.GetWatchManager(CpuType.Gsu).WatchEntries = _workspace.GsuWatchValues;
				BreakpointManager.SetBreakpoints(_workspace.Breakpoints);
				LabelManager.SetDefaultLabels();
				LabelManager.RefreshLabels();
				_workspace.Save();
				Clear();
			}
		}

		public static void ResetLabels()
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
				WatchManager.GetWatchManager(CpuType.Sa1).WatchEntries = _workspace.Sa1WatchValues;
				WatchManager.GetWatchManager(CpuType.Gsu).WatchEntries = _workspace.GsuWatchValues;

				LabelManager.ResetLabels();
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
			_symbolProvider = null;

			if(ConfigManager.Config.Debug.DbgIntegration.AutoImport) {
				RomInfo romInfo = EmuApi.GetRomInfo();
				string dbgPath = Path.Combine(((ResourcePath)romInfo.RomPath).Folder, romInfo.GetRomName() + ".dbg");
				if(File.Exists(dbgPath)) {
					_symbolProvider = new DbgImporter();
					(_symbolProvider as DbgImporter).Import(dbgPath, true);
					SymbolProviderChanged?.Invoke(_symbolProvider);
					LabelManager.RefreshLabels();
				}
			}

			SymbolProviderChanged?.Invoke(_symbolProvider);
		}

		public static ISymbolProvider GetSymbolProvider()
		{
			return _symbolProvider;
		}
	}
}
