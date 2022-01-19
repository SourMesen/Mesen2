using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public static class DebugWorkspaceManager
	{
		private static DebugWorkspace? _workspace = null;
		private static RomInfo _romInfo = new();
		private static string _path = "";

		public static void Load()
		{
			if(_workspace != null) {
				Save();
			}

			_romInfo = EmuApi.GetRomInfo();
			_path = Path.Combine(ConfigManager.DebuggerFolder, Path.ChangeExtension(_romInfo.GetRomName(), ".json"));
			_workspace = DebugWorkspace.Load(_path);
		}

		public static void Save(bool releaseWorkspace = false)
		{
			_workspace?.Save(_path, _romInfo.CpuTypes);
			if(releaseWorkspace) {
				_workspace = null;
			}
		}
	}

	public class CpuDebugWorkspace
	{
		public List<string> WatchEntries { get; set; } = new();
		public List<CodeLabel> Labels { get; set; } = new();
		public List<Breakpoint> Breakpoints { get; set; } = new();
	}

	public class DebugWorkspace
	{
		public Dictionary<CpuType, CpuDebugWorkspace> WorkspaceByCpu { get; set; } = new();
		public List<string> TblMappings = new();

		public static DebugWorkspace Load(string path)
		{
			DebugWorkspace dbgWorkspace = new();
			if(File.Exists(path)) {
				try {
					dbgWorkspace = JsonSerializer.Deserialize<DebugWorkspace>(File.ReadAllText(path), JsonHelper.Options) ?? new DebugWorkspace();
				} catch {
				}
			}

			LabelManager.ResetLabels();
			BreakpointManager.ClearBreakpoints();
			if(dbgWorkspace.WorkspaceByCpu.Count == 0) {
				DefaultLabelHelper.SetDefaultLabels();
			} else {
				foreach((CpuType cpuType, CpuDebugWorkspace workspace) in dbgWorkspace.WorkspaceByCpu) {
					WatchManager.GetWatchManager(cpuType).WatchEntries = workspace.WatchEntries;
					LabelManager.SetLabels(workspace.Labels);
					BreakpointManager.AddBreakpoints(workspace.Breakpoints);
				}
			}
			
			return dbgWorkspace;
		}

		public void Save(string path, HashSet<CpuType> cpuTypes)
		{
			WorkspaceByCpu = new();
			foreach(CpuType cpuType in cpuTypes) {
				CpuDebugWorkspace workspace = new();
				workspace.WatchEntries = WatchManager.GetWatchManager(cpuType).WatchEntries;
				workspace.Labels = LabelManager.GetLabels(cpuType);
				workspace.Breakpoints = BreakpointManager.GetBreakpoints(cpuType);
				WorkspaceByCpu[cpuType] = workspace;
			}

			File.WriteAllText(path, JsonSerializer.Serialize(this, typeof(DebugWorkspace), JsonHelper.Options));
		}
	}
}
