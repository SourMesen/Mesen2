using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerConfigWindowViewModel : ViewModelBase
	{
		public FontConfig Font { get; set; }
		public DebuggerConfig Debugger { get; set; }
		public ScriptWindowConfig Script { get; set; }
		public DbgIntegrationConfig Integration { get; set; }
		[Reactive] public DebugConfigWindowTab SelectedIndex { get; set; }

		public List<DebuggerShortcutInfo> SharedShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> MemoryToolsShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> ScriptShortcuts { get; set; } = new();
		public List<DebuggerShortcutInfo> DebuggerShortcuts { get; set; } = new();

		public DebuggerConfigWindowViewModel(DebugConfigWindowTab tab)
		{
			SelectedIndex = tab;
			Debugger = ConfigManager.Config.Debug.Debugger;
			Font = ConfigManager.Config.Debug.Font;
			Script = ConfigManager.Config.Debug.ScriptWindow;
			Integration = ConfigManager.Config.Debug.DbgIntegration;

			InitShortcutLists();
		}

		private void InitShortcutLists()
		{
			SharedShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.IncreaseFontSize,
				DebuggerShortcut.DecreaseFontSize,
				DebuggerShortcut.ResetFontSize,
				DebuggerShortcut.ZoomIn,
				DebuggerShortcut.ZoomOut,
				DebuggerShortcut.SaveAsPng,
				DebuggerShortcut.GoTo,
				DebuggerShortcut.Find,
				DebuggerShortcut.FindNext,
				DebuggerShortcut.FindPrev,
				DebuggerShortcut.Undo,
				DebuggerShortcut.Cut,
				DebuggerShortcut.Copy,
				DebuggerShortcut.Paste,
				DebuggerShortcut.SelectAll,
				DebuggerShortcut.Refresh,
				DebuggerShortcut.MarkAsCode,
				DebuggerShortcut.MarkAsData,
				DebuggerShortcut.MarkAsUnidentified,
				DebuggerShortcut.GoToAll,
				DebuggerShortcut.CodeWindow_EditInMemoryViewer,
				DebuggerShortcut.MemoryViewer_ViewInDisassembly,

				DebuggerShortcut.OpenAssembler,
				DebuggerShortcut.OpenDebugger,
				DebuggerShortcut.OpenEventViewer,
				DebuggerShortcut.OpenMemoryTools,
				DebuggerShortcut.OpenRegisterViewer,
				DebuggerShortcut.OpenProfiler,
				DebuggerShortcut.OpenScriptWindow,
				DebuggerShortcut.OpenTraceLogger,
				DebuggerShortcut.OpenDebugLog,

				DebuggerShortcut.OpenTilemapViewer,
				DebuggerShortcut.OpenTileViewer,
				DebuggerShortcut.OpenSpriteViewer,
				DebuggerShortcut.OpenPaletteViewer,

				DebuggerShortcut.OpenSpcDebugger,
				DebuggerShortcut.OpenSa1Debugger,
				DebuggerShortcut.OpenGameboyDebugger,
				DebuggerShortcut.OpenGsuDebugger,
				DebuggerShortcut.OpenNecDspDebugger,
				DebuggerShortcut.OpenCx4Debugger,
			});

			MemoryToolsShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				//DebuggerShortcut.MemoryViewer_Freeze,
				//DebuggerShortcut.MemoryViewer_Unfreeze,
				DebuggerShortcut.MemoryViewer_AddToWatch,
				DebuggerShortcut.MemoryViewer_EditBreakpoint,
				DebuggerShortcut.MemoryViewer_EditLabel,
				DebuggerShortcut.MemoryViewer_Import,
				DebuggerShortcut.MemoryViewer_Export,
				//DebuggerShortcut.MemoryViewer_ViewInCpuMemory,
				//DebuggerShortcut.MemoryViewer_ViewInMemoryType
			});

			ScriptShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.ScriptWindow_OpenScript,
				DebuggerShortcut.ScriptWindow_SaveScript,
				DebuggerShortcut.ScriptWindow_RunScript,
				DebuggerShortcut.ScriptWindow_StopScript
			});

			DebuggerShortcuts = CreateShortcutList(new DebuggerShortcut[] {
				DebuggerShortcut.ReloadRom,
				DebuggerShortcut.Reset,
				DebuggerShortcut.PowerCycle,
				DebuggerShortcut.Continue,
				DebuggerShortcut.Break,
				DebuggerShortcut.ToggleBreakContinue,
				DebuggerShortcut.StepInto,
				DebuggerShortcut.StepOver,
				DebuggerShortcut.StepOut,
				//DebuggerShortcut.StepBack,
				DebuggerShortcut.RunPpuCycle,
				DebuggerShortcut.RunPpuScanline,
				DebuggerShortcut.RunPpuFrame,
				DebuggerShortcut.BreakIn,
				DebuggerShortcut.BreakOn,
				//DebuggerShortcut.FindOccurrences,
				DebuggerShortcut.GoToProgramCounter,
				//DebuggerShortcut.CodeWindow_SetNextStatement,
				//DebuggerShortcut.CodeWindow_EditSubroutine,
				DebuggerShortcut.CodeWindow_EditSelectedCode,
				//DebuggerShortcut.CodeWindow_EditSourceFile,
				DebuggerShortcut.CodeWindow_EditLabel,
				//DebuggerShortcut.CodeWindow_NavigateBack,
				//DebuggerShortcut.CodeWindow_NavigateForward,
				DebuggerShortcut.CodeWindow_ToggleBreakpoint,
				DebuggerShortcut.CodeWindow_DisableEnableBreakpoint,
				DebuggerShortcut.CodeWindow_SwitchView,
				//DebuggerShortcut.FunctionList_EditLabel,
				//DebuggerShortcut.FunctionList_AddBreakpoint,
				//DebuggerShortcut.FunctionList_FindOccurrences,
				DebuggerShortcut.LabelList_Add,
				DebuggerShortcut.LabelList_Edit,
				DebuggerShortcut.LabelList_Delete,
				DebuggerShortcut.LabelList_AddBreakpoint,
				DebuggerShortcut.LabelList_AddToWatch,
				//DebuggerShortcut.LabelList_FindOccurrences,
				DebuggerShortcut.LabelList_ViewInMemoryViewer,
				DebuggerShortcut.LabelList_GoToLocation,
				DebuggerShortcut.BreakpointList_Add,
				DebuggerShortcut.BreakpointList_Edit,
				DebuggerShortcut.BreakpointList_GoToLocation,
				DebuggerShortcut.BreakpointList_Delete,
				DebuggerShortcut.WatchList_Add,
				DebuggerShortcut.WatchList_Edit,
				DebuggerShortcut.WatchList_Delete,
				DebuggerShortcut.WatchList_MoveUp,
				DebuggerShortcut.WatchList_MoveDown,
				DebuggerShortcut.CallStack_EditLabel,
				DebuggerShortcut.CallStack_GoToLocation,
				//DebuggerShortcut.SaveRom,
				DebuggerShortcut.SaveRomAs,
				DebuggerShortcut.SaveEditAsIps,
				//DebuggerShortcut.RevertPrgChrChanges,
				//DebuggerShortcut.ToggleVerifiedData,
				//DebuggerShortcut.ToggleUnidentifiedCodeData
			});
		}

		private List<DebuggerShortcutInfo> CreateShortcutList(DebuggerShortcut[] debuggerShortcuts)
		{
			DebuggerShortcutsConfig shortcuts = ConfigManager.Config.Debug.Shortcuts;
			return debuggerShortcuts.Select(s => shortcuts.GetBindable(s)).ToList();
		}

		internal void SaveConfig()
		{
			//TODO
			ConfigManager.Config.ApplyConfig();
		}
	}

	public enum DebugConfigWindowTab
	{
		Debugger,
		ScriptWindow,
		FontAndColors,
		Integration,
		Shortcuts
	}
}
