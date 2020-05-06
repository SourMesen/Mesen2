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
				_workspace.NecDspWatchValues = new List<string>(WatchManager.GetWatchManager(CpuType.NecDsp).WatchEntries);
				_workspace.Breakpoints = new List<Breakpoint>(BreakpointManager.Breakpoints);
				_workspace.CpuLabels = new List<CodeLabel>(LabelManager.GetLabels(CpuType.Cpu));
				_workspace.SpcLabels = new List<CodeLabel>(LabelManager.GetLabels(CpuType.Spc));
				_workspace.NecDspLabels = new List<CodeLabel>(LabelManager.GetLabels(CpuType.NecDsp));
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
				_workspace.NecDspWatchValues = new List<string>();
				_workspace.CpuLabels = new List<CodeLabel>();
				_workspace.SpcLabels = new List<CodeLabel>();
				_workspace.NecDspLabels = new List<CodeLabel>();
				WatchManager.GetWatchManager(CpuType.Cpu).WatchEntries = _workspace.WatchValues;
				WatchManager.GetWatchManager(CpuType.Spc).WatchEntries = _workspace.SpcWatchValues;
				WatchManager.GetWatchManager(CpuType.Sa1).WatchEntries = _workspace.Sa1WatchValues;
				WatchManager.GetWatchManager(CpuType.Gsu).WatchEntries = _workspace.GsuWatchValues;
				WatchManager.GetWatchManager(CpuType.NecDsp).WatchEntries = _workspace.NecDspWatchValues;
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
				_workspace.NecDspLabels = new List<CodeLabel>();
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
				WatchManager.GetWatchManager(CpuType.NecDsp).WatchEntries = _workspace.NecDspWatchValues;

				LabelManager.ResetLabels();
				LabelManager.SetLabels(_workspace.CpuLabels);
				LabelManager.SetLabels(_workspace.SpcLabels);
				LabelManager.SetLabels(_workspace.NecDspLabels);
				LabelManager.SetDefaultLabels();

				ImportDbgFile();
			}
			
			//Send breakpoints & labels to emulation core (even if the same game is running)
			LabelManager.RefreshLabels();
			BreakpointManager.SetBreakpoints(_workspace.Breakpoints);

			return _workspace;
		}

		public static void ImportMslFile(string mslPath, bool silent = false)
		{
			if(ConfigManager.Config.Debug.DbgIntegration.ResetLabelsOnImport) {
				ResetLabels();
			}
			MslLabelFile.Import(mslPath, silent);
			LabelManager.RefreshLabels();
		}

		public static void ImportSymFile(string symPath, bool silent = false)
		{
			if(ConfigManager.Config.Debug.DbgIntegration.ResetLabelsOnImport) {
				ResetLabels();
			}
			new WlaDxImporter().Import(symPath, silent);
			LabelManager.RefreshLabels();
		}

		public static void ImportDbgFile(string dbgPath = null)
		{
			_symbolProvider = null;

			if(dbgPath != null || ConfigManager.Config.Debug.DbgIntegration.AutoImport) {
				bool silent = dbgPath == null;
				if(dbgPath == null) {
					RomInfo romInfo = EmuApi.GetRomInfo();
					dbgPath = Path.Combine(((ResourcePath)romInfo.RomPath).Folder, romInfo.GetRomName() + ".dbg");
				}

				if(File.Exists(dbgPath)) {
					_symbolProvider = new DbgImporter();
					(_symbolProvider as DbgImporter).Import(dbgPath, silent);
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
