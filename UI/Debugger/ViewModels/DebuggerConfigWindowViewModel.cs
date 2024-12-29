using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reflection;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerConfigWindowViewModel : DisposableViewModel
	{
		public DebuggerFontConfig Fonts { get; set; }
		public DebuggerConfig Debugger { get; set; }
		public ScriptWindowConfig Script { get; set; }
		public IntegrationConfig Integration { get; set; }

		public List<object> CpuTypeList { get; set; } = new();

		[Reactive] public DebugConfigWindowTab SelectedIndex { get; set; }

		public List<DebuggerShortcutInfo> SharedShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> MemoryToolsShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> MiscShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> ScriptShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> DebuggerShortcuts { get; set; } = new();

		private DebuggerConfig _backupDebugger;
		private DebuggerFontConfig _backupFont;
		private ScriptWindowConfig _backupScript;
		private IntegrationConfig _backupIntegration;
		private DebuggerShortcutsConfig _backupShortcuts;
		private Dictionary<object, HashSet<string>> _changes = new();

		[Obsolete("For designer only")]
		public DebuggerConfigWindowViewModel() : this(DebugConfigWindowTab.Debugger) { }

		public DebuggerConfigWindowViewModel(DebugConfigWindowTab tab)
		{
			SelectedIndex = tab;
			Debugger = ConfigManager.Config.Debug.Debugger;
			Fonts = ConfigManager.Config.Debug.Fonts;
			Script = ConfigManager.Config.Debug.ScriptWindow;
			Integration = ConfigManager.Config.Debug.Integration;

			_backupDebugger = Debugger.Clone();
			_backupFont = Fonts.Clone();
			_backupScript = Script.Clone();
			_backupIntegration = Integration.Clone();
			_backupShortcuts = ConfigManager.Config.Debug.Shortcuts.Clone();

			InitShortcutLists();

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Debugger, Config_PropertyChanged));
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Fonts, Config_PropertyChanged));
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Script, Config_PropertyChanged));
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Integration, Config_PropertyChanged));
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			if(sender == null || e.PropertyName == null) {
				return;
			}

			if(!_changes.TryGetValue(sender, out HashSet<string>? changes)) {
				_changes[sender] = new HashSet<string>() { e.PropertyName };
			} else {
				changes.Add(e.PropertyName);
			}

			ConfigManager.Config.Debug.ApplyConfig();
			ConfigManager.Config.Debug.Fonts.ApplyConfig();
		}

		public void RevertChanges()
		{
			RevertChanges(Debugger, _backupDebugger);
			RevertChanges(Script, _backupScript);
			RevertChanges(Integration, _backupIntegration);
			
			RevertChanges(Fonts.DisassemblyFont, _backupFont.DisassemblyFont);
			RevertChanges(Fonts.AssemblerFont, _backupFont.AssemblerFont);
			RevertChanges(Fonts.MemoryViewerFont, _backupFont.MemoryViewerFont);
			RevertChanges(Fonts.ScriptWindowFont, _backupFont.ScriptWindowFont);
			RevertChanges(Fonts.OtherMonoFont, _backupFont.OtherMonoFont);

			ConfigManager.Config.Debug.Shortcuts = _backupShortcuts;
		}

		public bool IsDirty()
		{
			return (
				!Debugger.IsIdentical(_backupDebugger) ||
				!Script.IsIdentical(_backupScript) ||
				!Integration.IsIdentical(_backupIntegration) ||
				!Fonts.IsIdentical(_backupFont)
			);
		}

		private void RevertChanges<[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicProperties | DynamicallyAccessedMemberTypes.NonPublicProperties)] T>(T current, T original) where T : ReactiveObject
		{
			if(_changes.TryGetValue(current, out HashSet<string>? changes)) {
				foreach(string propertyName in changes) {
					PropertyInfo? prop = typeof(T).GetProperty(propertyName, BindingFlags.Public | BindingFlags.Instance);
					if(prop != null) {
						prop.SetValue(current, prop.GetValue(original));
					}
				}
			}
		}

		private void InitShortcutLists()
		{
			SharedShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				//DebuggerShortcut.IncreaseFontSize,
				//DebuggerShortcut.DecreaseFontSize,
				//DebuggerShortcut.ResetFontSize,
				DebuggerShortcut.ToggleSettingsPanel,
				DebuggerShortcut.ZoomIn,
				DebuggerShortcut.ZoomOut,
				DebuggerShortcut.SaveAsPng,
				DebuggerShortcut.GoToAddress,
				DebuggerShortcut.Find,
				DebuggerShortcut.FindNext,
				DebuggerShortcut.FindPrev,
				//DebuggerShortcut.Undo,
				//DebuggerShortcut.Cut,
				DebuggerShortcut.Copy,
				DebuggerShortcut.Paste,
				DebuggerShortcut.SelectAll,
				DebuggerShortcut.Undo,
				DebuggerShortcut.Refresh,
				DebuggerShortcut.MarkAsCode,
				DebuggerShortcut.MarkAsData,
				DebuggerShortcut.MarkAsUnidentified,
				DebuggerShortcut.GoToAll,

				DebuggerShortcut.OpenAssembler,
				DebuggerShortcut.OpenDebugger,
				DebuggerShortcut.OpenEventViewer,
				DebuggerShortcut.OpenMemoryTools,
				DebuggerShortcut.OpenRegisterViewer,
				DebuggerShortcut.OpenProfiler,
				DebuggerShortcut.OpenScriptWindow,
				DebuggerShortcut.OpenTraceLogger,
				DebuggerShortcut.OpenWatchWindow,
				DebuggerShortcut.OpenMemorySearch,
				DebuggerShortcut.OpenDebugLog,
				DebuggerShortcut.OpenNesHeaderEditor,

				DebuggerShortcut.OpenTilemapViewer,
				DebuggerShortcut.OpenTileViewer,
				DebuggerShortcut.OpenSpriteViewer,
				DebuggerShortcut.OpenPaletteViewer,

				DebuggerShortcut.OpenSpcDebugger,
				DebuggerShortcut.OpenSa1Debugger,
				DebuggerShortcut.OpenSt018Debugger,
				DebuggerShortcut.OpenGameboyDebugger,
				DebuggerShortcut.OpenGsuDebugger,
				DebuggerShortcut.OpenNecDspDebugger,
				DebuggerShortcut.OpenCx4Debugger,
			});

			MemoryToolsShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.MemoryViewer_Freeze,
				DebuggerShortcut.MemoryViewer_Unfreeze,
				DebuggerShortcut.MemoryViewer_AddToWatch,
				DebuggerShortcut.MemoryViewer_EditBreakpoint,
				DebuggerShortcut.MemoryViewer_EditLabel,
				DebuggerShortcut.MemoryViewer_Import,
				DebuggerShortcut.MemoryViewer_Export,
				DebuggerShortcut.MemoryViewer_ResetAccessCounters,
				DebuggerShortcut.MemoryViewer_ViewInDebugger,
				DebuggerShortcut.MemoryViewer_ViewInMemoryType,

				DebuggerShortcut.MemoryViewer_GoToPrevRead,
				DebuggerShortcut.MemoryViewer_GoToNextRead,
				DebuggerShortcut.MemoryViewer_GoToPrevWrite,
				DebuggerShortcut.MemoryViewer_GoToNextWrite,
				DebuggerShortcut.MemoryViewer_GoToPrevExec,
				DebuggerShortcut.MemoryViewer_GoToNextExec,

				DebuggerShortcut.MemoryViewer_GoToPrevCode,
				DebuggerShortcut.MemoryViewer_GoToNextCode,
				DebuggerShortcut.MemoryViewer_GoToPrevData,
				DebuggerShortcut.MemoryViewer_GoToNextData,
				DebuggerShortcut.MemoryViewer_GoToPrevUnknown,
				DebuggerShortcut.MemoryViewer_GoToNextUnknown,
			});

			ScriptShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.ScriptWindow_OpenScript,
				DebuggerShortcut.ScriptWindow_SaveScript,
				DebuggerShortcut.ScriptWindow_RunScript,
				DebuggerShortcut.ScriptWindow_StopScript
			});

			MiscShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.TilemapViewer_ViewInMemoryViewer,
				DebuggerShortcut.TilemapViewer_ViewInTileViewer,
				DebuggerShortcut.TilemapViewer_EditTilemapBreakpoint,
				DebuggerShortcut.TilemapViewer_EditAttributeBreakpoint,
				DebuggerShortcut.TilemapViewer_EditTile,
				DebuggerShortcut.TileViewer_ViewInMemoryViewer,
				DebuggerShortcut.TileViewer_EditTile,
				DebuggerShortcut.TileEditor_FlipHorizontal,
				DebuggerShortcut.TileEditor_FlipVertical,
				DebuggerShortcut.TileEditor_RotateLeft,
				DebuggerShortcut.TileEditor_RotateRight,
				DebuggerShortcut.TileEditor_TranslateLeft,
				DebuggerShortcut.TileEditor_TranslateRight,
				DebuggerShortcut.TileEditor_TranslateUp,
				DebuggerShortcut.TileEditor_TranslateDown,
				DebuggerShortcut.SpriteViewer_ViewInMemoryViewer,
				DebuggerShortcut.SpriteViewer_ViewInTileViewer,
				DebuggerShortcut.SpriteViewer_EditSprite,
				DebuggerShortcut.PaletteViewer_EditColor,
				DebuggerShortcut.PaletteViewer_ViewInMemoryViewer,

				DebuggerShortcut.TraceLogger_EditBreakpoint,
				DebuggerShortcut.TraceLogger_EditLabel,
				DebuggerShortcut.TraceLogger_ViewInDebugger,
				DebuggerShortcut.TraceLogger_ViewInMemoryViewer,

				DebuggerShortcut.RegisterViewer_EditBreakpoint,
			});

			DebuggerShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.Reset,
				DebuggerShortcut.PowerCycle,
				DebuggerShortcut.ReloadRom,
				DebuggerShortcut.ToggleBreakContinue,
				DebuggerShortcut.Continue,
				DebuggerShortcut.Break,
				DebuggerShortcut.StepInto,
				DebuggerShortcut.StepOver,
				DebuggerShortcut.StepOut,
				DebuggerShortcut.StepBack,
				DebuggerShortcut.StepBackScanline,
				DebuggerShortcut.StepBackFrame,
				DebuggerShortcut.RunCpuCycle,
				DebuggerShortcut.RunPpuCycle,
				DebuggerShortcut.RunPpuScanline,
				DebuggerShortcut.RunPpuFrame,
				DebuggerShortcut.BreakIn,
				DebuggerShortcut.RunToNmi,
				DebuggerShortcut.RunToIrq,
				DebuggerShortcut.BreakOn,
				DebuggerShortcut.FindOccurrences,
				DebuggerShortcut.GoToProgramCounter,
				DebuggerShortcut.GoToCpuVector1,
				DebuggerShortcut.GoToCpuVector2,
				DebuggerShortcut.GoToCpuVector3,
				DebuggerShortcut.GoToCpuVector4,
				DebuggerShortcut.GoToCpuVector5,
				DebuggerShortcut.CodeWindow_EditSelectedCode,
				DebuggerShortcut.CodeWindow_AddToWatch,
				DebuggerShortcut.CodeWindow_ViewInMemoryViewer,
				DebuggerShortcut.CodeWindow_EditLabel,
				DebuggerShortcut.CodeWindow_EditComment,
				DebuggerShortcut.CodeWindow_NavigateBack,
				DebuggerShortcut.CodeWindow_NavigateForward,
				DebuggerShortcut.CodeWindow_ToggleBreakpoint,
				//DebuggerShortcut.CodeWindow_DisableEnableBreakpoint,
				DebuggerShortcut.CodeWindow_MoveProgramCounter,
				DebuggerShortcut.CodeWindow_RunToLocation,
				DebuggerShortcut.CodeWindow_GoToLocation,
				DebuggerShortcut.LabelList_Add,
				DebuggerShortcut.LabelList_Edit,
				DebuggerShortcut.LabelList_Delete,
				DebuggerShortcut.LabelList_AddToWatch,
				DebuggerShortcut.LabelList_FindOccurrences,
				DebuggerShortcut.LabelList_GoToLocation,
				DebuggerShortcut.LabelList_ToggleBreakpoint,
				DebuggerShortcut.LabelList_ViewInMemoryViewer,
				DebuggerShortcut.FunctionList_EditLabel,
				DebuggerShortcut.FunctionList_FindOccurrences,
				DebuggerShortcut.FunctionList_GoToLocation,
				DebuggerShortcut.FunctionList_ToggleBreakpoint,
				DebuggerShortcut.FunctionList_ViewInMemoryViewer,
				DebuggerShortcut.BreakpointList_Add,
				DebuggerShortcut.BreakpointList_AddForbid,
				DebuggerShortcut.BreakpointList_Edit,
				DebuggerShortcut.BreakpointList_GoToLocation,
				DebuggerShortcut.BreakpointList_ViewInMemoryViewer,
				DebuggerShortcut.BreakpointList_Delete,
				DebuggerShortcut.BreakpointList_EnableBreakpoint,
				DebuggerShortcut.BreakpointList_DisableBreakpoint,
				DebuggerShortcut.WatchList_Delete,
				DebuggerShortcut.WatchList_MoveUp,
				DebuggerShortcut.WatchList_MoveDown,
				DebuggerShortcut.CallStack_EditLabel,
				DebuggerShortcut.CallStack_GoToLocation,
				DebuggerShortcut.FindResultList_AddWatch,
				DebuggerShortcut.FindResultList_GoToLocation,
				DebuggerShortcut.FindResultList_ToggleBreakpoint,
				DebuggerShortcut.SaveRom,
				DebuggerShortcut.SaveRomAs,
				DebuggerShortcut.SaveEditAsIps,
				DebuggerShortcut.ResetCdl,
				DebuggerShortcut.LoadCdl,
				DebuggerShortcut.SaveCdl,
				DebuggerShortcut.ImportLabels,
				DebuggerShortcut.ExportLabels,
				DebuggerShortcut.ImportWatchEntries,
				DebuggerShortcut.ExportWatchEntries,
				DebuggerShortcut.ResetWorkspace
			});
		}

		private List<DebuggerShortcutInfo> CreateShortcutList(DebuggerShortcut[] debuggerShortcuts)
		{
			DebuggerShortcutsConfig shortcuts = ConfigManager.Config.Debug.Shortcuts;
			return debuggerShortcuts.Select(s => shortcuts.GetBindable(s)).ToList();
		}
	}

	public enum DebugConfigWindowTab
	{
		Debugger = 0,
		ScriptWindow = 1,
		//separator
		FontAndColors = 3,
		Integration = 4,
		//separator
		Shortcuts = 6
	}
}
