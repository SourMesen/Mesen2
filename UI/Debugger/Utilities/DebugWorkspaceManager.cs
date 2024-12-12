using Mesen.Config;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
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
		public static ISymbolProvider? SymbolProvider { get; private set; }

		public static event EventHandler? SymbolProviderChanged;

		public static DebugWorkspace Workspace 
		{
			get
			{
				if(_workspace == null) {
					Load();
				}
				return _workspace;
			}
		}

		private static string? GetMatchingFile(string ext)
		{
			string path = Path.ChangeExtension(_romInfo.RomPath, ext);
			if(File.Exists(path)) {
				return path;
			}

			path = _romInfo.RomPath + "." + ext;
			if(File.Exists(path)) {
				return path;
			}

			return null;
		}

		[MemberNotNull(nameof(DebugWorkspaceManager._workspace))]
		public static void Load()
		{
			if(_workspace != null) {
				Save();
			}

			_romInfo = EmuApi.GetRomInfo();
			_path = Path.Combine(ConfigManager.DebuggerFolder, _romInfo.GetRomName() + ".json");

			LabelManager.SuspendEvents();
			_workspace = DebugWorkspace.Load(_path);

			SymbolProvider = null;
			if(ConfigManager.Config.Debug.Integration.AutoLoadDbgFiles) {
				string? dbgPath = GetMatchingFile(FileDialogHelper.DbgFileExt);
				if(dbgPath != null) {
					LoadDbgSymbolFile(dbgPath, false);
				}
			}

			if(SymbolProvider == null && ConfigManager.Config.Debug.Integration.AutoLoadSymFiles) {
				string? symPath = GetMatchingFile(FileDialogHelper.SymFileExt);
				if(symPath != null) {
					LoadSymFile(symPath, false);
				}
			}

			if(SymbolProvider == null && ConfigManager.Config.Debug.Integration.AutoLoadCdbFiles) {
				string? symPath = GetMatchingFile(FileDialogHelper.CdbFileExt);
				if(symPath != null) {
					LoadCdbFile(symPath, false);
				}
			}

			if(SymbolProvider == null && ConfigManager.Config.Debug.Integration.AutoLoadElfFiles) {
				string? symPath = GetMatchingFile(FileDialogHelper.ElfFileExt);
				if(symPath != null) {
					LoadElfFile(symPath, false);
				}
			}

			if(SymbolProvider == null && ConfigManager.Config.Debug.Integration.AutoLoadMlbFiles) {
				string? mlbPath = GetMatchingFile(FileDialogHelper.MesenLabelExt);
				if(mlbPath != null) {
					LoadMesenLabelFile(mlbPath, false);
				}
			}

			if(SymbolProvider == null && ConfigManager.Config.Debug.Integration.AutoLoadFnsFiles) {
				string? fnsPath = GetMatchingFile(FileDialogHelper.NesAsmLabelExt);
				if(fnsPath != null) {
					LoadNesAsmLabelFile(fnsPath, false);
				}
			}

			if(ConfigManager.Config.Debug.Integration.AutoLoadCdlFiles) {
				string? cdlPath = GetMatchingFile(FileDialogHelper.CdlExt);
				if(cdlPath != null) {
					LoadCdlFile(cdlPath);
				}
			}
			LabelManager.ResumeEvents();
			SymbolProviderChanged?.Invoke(null, EventArgs.Empty);
		}

		private static void ResetLabels()
		{
			if(ConfigManager.Config.Debug.Integration.ResetLabelsOnImport) {
				LabelManager.ResetLabels();
				DefaultLabelHelper.SetDefaultLabels();
			}
		}

		public static void LoadDbgSymbolFile(string path, bool showResult)
		{
			if(File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.DbgFileExt) {
				ResetLabels();
				SymbolProvider = DbgImporter.Import(_romInfo.Format, path, ConfigManager.Config.Debug.Integration.ImportComments, showResult);
			}
		}

		public static void LoadElfFile(string path, bool showResult)
		{
			if(File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.ElfFileExt) {
				ResetLabels();
				ElfImporter.Import(path, showResult, _romInfo.ConsoleType.GetMainCpuType());
			}
		}

		public static void LoadCdbFile(string path, bool showResult)
		{
			if(File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.CdbFileExt) {
				ResetLabels();
				SymbolProvider = SdccSymbolImporter.Import(_romInfo.Format, path, showResult);
			}
		}

		public static void LoadSymFile(string path, bool showResult)
		{
			if(File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.SymFileExt) {
				string symContent = File.ReadAllText(path);
				if(symContent.Contains("[labels]") || symContent.Contains("this file was created with wlalink")) {
					//Assume WLA-DX symbol files
					WlaDxImporter? importer = null;
					switch(_romInfo.ConsoleType) {
						case ConsoleType.Snes:
							if(_romInfo.CpuTypes.Contains(CpuType.Gameboy)) {
								importer = new GbWlaDxImporter();
							} else {
								importer = new SnesWlaDxImporter();
							}
							break;

						case ConsoleType.Gameboy: importer = new GbWlaDxImporter(); break;
						case ConsoleType.PcEngine: importer = new PceWlaDxImporter(); break;
						case ConsoleType.Sms: importer = new SmsWlaDxImporter(); break;
					}

					if(importer != null) {
						ResetLabels();
						importer.Import(path, showResult);
						SymbolProvider = importer;
					}
				} else {
					if(_romInfo.CpuTypes.Contains(CpuType.Gameboy)) {
						if(RgbdsSymbolFile.IsValidFile(path)) {
							RgbdsSymbolFile.Import(path, showResult);
						} else {
							BassLabelFile.Import(path, showResult, CpuType.Gameboy);
						}
					} else {
						if(_romInfo.ConsoleType == ConsoleType.PcEngine && PceasSymbolImporter.IsValidFile(symContent)) {
							PceasSymbolImporter importer = new PceasSymbolImporter();
							importer.Import(path, showResult);
							SymbolProvider = importer;
						} else if(_romInfo.ConsoleType == ConsoleType.PcEngine && LegacyPceasSymbolFile.IsValidFile(symContent)) {
							LegacyPceasSymbolFile.Import(path, showResult);
						} else {
							BassLabelFile.Import(path, showResult, _romInfo.ConsoleType.GetMainCpuType());
						}
					}
				}
			}
		}

		public static void LoadMesenLabelFile(string path, bool showResult)
		{
			if(File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.MesenLabelExt) {
				ResetLabels();
				MesenLabelFile.Import(path, showResult);
			}
		}

		public static void LoadNesAsmLabelFile(string path, bool showResult)
		{
			if(_romInfo.ConsoleType == ConsoleType.Nes && File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.NesAsmLabelExt) {
				ResetLabels();
				NesasmFnsImporter.Import(path, showResult);
			}
		}

		private static void LoadCdlFile(string path)
		{
			if(File.Exists(path) && Path.GetExtension(path).ToLower() == "." + FileDialogHelper.CdlExt) {
				DebugApi.LoadCdlFile(_romInfo.ConsoleType.GetMainCpuType().GetPrgRomMemoryType(), path);
			}
		}

		public static void LoadSupportedFile(string filename, bool showResult)
		{
			ISymbolProvider? symbolProvider = SymbolProvider;
			SymbolProvider = null;

			switch(Path.GetExtension(filename).ToLower().Substring(1)) {
				case FileDialogHelper.DbgFileExt: LoadDbgSymbolFile(filename, showResult); break;
				case FileDialogHelper.SymFileExt: LoadSymFile(filename, showResult); break;
				case FileDialogHelper.CdbFileExt: LoadCdbFile(filename, showResult); break;
				case FileDialogHelper.ElfFileExt: LoadElfFile(filename, showResult); break;
				case FileDialogHelper.MesenLabelExt: LoadMesenLabelFile(filename, showResult); break;
				case FileDialogHelper.NesAsmLabelExt: LoadNesAsmLabelFile(filename, showResult); break;
				case FileDialogHelper.CdlExt: LoadCdlFile(filename); SymbolProvider = symbolProvider; break;
			}

			if(SymbolProvider != symbolProvider) {
				SymbolProviderChanged?.Invoke(null, EventArgs.Empty);
			}
		}

		public static void Save(bool releaseWorkspace = false)
		{
			_workspace?.Save(_path, _romInfo.CpuTypes);
			if(releaseWorkspace) {
				_workspace = null;
			}
		}

		private static DateTime _previousAutoSave = DateTime.MinValue;
		public static void AutoSave()
		{
			//Automatically save when changing a label/breakpoint/watch to avoid losing progress if a crash occurs
			if((DateTime.Now - _previousAutoSave).TotalSeconds >= 60) {
				_workspace?.Save(_path, _romInfo.CpuTypes);
				_previousAutoSave = DateTime.Now;
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
		public string[] TblMappings { get; set; } = Array.Empty<string>();

		public static DebugWorkspace Load(string path)
		{
			DebugWorkspace dbgWorkspace = new();
			if(File.Exists(path)) {
				try {
					string fileData = File.ReadAllText(path);
					dbgWorkspace = (DebugWorkspace?)JsonSerializer.Deserialize(fileData, typeof(DebugWorkspace), MesenSerializerContext.Default) ?? new DebugWorkspace();
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

			FileHelper.WriteAllText(path, JsonSerializer.Serialize(this, typeof(DebugWorkspace), MesenSerializerContext.Default));
		}

		public void Reset()
		{
			foreach((CpuType cpuType, CpuDebugWorkspace workspace) in WorkspaceByCpu) {
				WatchManager.GetWatchManager(cpuType).WatchEntries = new();
				workspace.Breakpoints.Clear();
				workspace.Labels.Clear();
				workspace.WatchEntries.Clear();
			}
			BreakpointManager.ClearBreakpoints();
			LabelManager.ResetLabels();
			DefaultLabelHelper.SetDefaultLabels();
			LabelManager.RefreshLabels(true);
		}
	}
}
