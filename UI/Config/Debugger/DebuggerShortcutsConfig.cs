using Avalonia.Input;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;
using System.Text.RegularExpressions;

namespace Mesen.Config
{
	public class DebuggerShortcutsConfig : BaseConfig<DebuggerShortcutsConfig>, IJsonOnDeserialized
	{
		private List<DebuggerShortcutInfo> _shortcuts = new();
		private Dictionary<DebuggerShortcut, DebuggerShortcutInfo> _lookup = new();

		public DebuggerShortcutsConfig()
		{
			Init();
		}

		public List<DebuggerShortcutInfo> Shortcuts
		{
			get => _shortcuts;
			set
			{
				_shortcuts = value;
				InitLookup();
			}
		}

		private void InitLookup()
		{
			_lookup = new();
			foreach(DebuggerShortcutInfo shortcut in _shortcuts) {
				_lookup[shortcut.Shortcut] = shortcut;
			}
		}

		public DbgShortKeys Get(DebuggerShortcut shortcut)
		{
			if(_lookup.TryGetValue(shortcut, out DebuggerShortcutInfo? info)) {
				return info.KeyBinding;
			} else {
				throw new Exception("Invalid shortcut");
			}
		}

		internal DebuggerShortcutInfo GetBindable(DebuggerShortcut shortcut)
		{
			if(_lookup.TryGetValue(shortcut, out DebuggerShortcutInfo? info)) {
				return info;
			} else {
				throw new Exception("Invalid shortcut");
			}
		}

		private void Add(DebuggerShortcutInfo info)
		{
			if(!_lookup.ContainsKey(info.Shortcut)) {
				Shortcuts.Add(info);
				_lookup[info.Shortcut] = info;
			}
		}

		public void OnDeserialized()
		{
			Init();
		}

		private void Init()
		{
			//Shared
			Add(new() { Shortcut = DebuggerShortcut.GoToAddress, KeyBinding = new(KeyModifiers.Control, Key.G) });

			Add(new() { Shortcut = DebuggerShortcut.Find, KeyBinding = new(KeyModifiers.Control, Key.F) });
			Add(new() { Shortcut = DebuggerShortcut.FindNext, KeyBinding = new(Key.F3) });
			Add(new() { Shortcut = DebuggerShortcut.FindPrev, KeyBinding = new(KeyModifiers.Shift, Key.F3) });

			Add(new() { Shortcut = DebuggerShortcut.Copy, KeyBinding = new(KeyModifiers.Control, Key.C) });
			Add(new() { Shortcut = DebuggerShortcut.Paste, KeyBinding = new(KeyModifiers.Control, Key.V) });
			Add(new() { Shortcut = DebuggerShortcut.SelectAll, KeyBinding = new(KeyModifiers.Control, Key.A) });
			
			Add(new() { Shortcut = DebuggerShortcut.Undo, KeyBinding = new(KeyModifiers.Control, Key.Z) });

			Add(new() { Shortcut = DebuggerShortcut.Refresh, KeyBinding = new(Key.F5) });

			Add(new() { Shortcut = DebuggerShortcut.MarkAsCode, KeyBinding = new(KeyModifiers.Control, Key.D1) });
			Add(new() { Shortcut = DebuggerShortcut.MarkAsData, KeyBinding = new(KeyModifiers.Control, Key.D2) });
			Add(new() { Shortcut = DebuggerShortcut.MarkAsUnidentified, KeyBinding = new(KeyModifiers.Control, Key.D3) });

			Add(new() { Shortcut = DebuggerShortcut.GoToAll, KeyBinding = new(KeyModifiers.Control, Key.OemComma) });

			Add(new() { Shortcut = DebuggerShortcut.ZoomIn, KeyBinding = new(KeyModifiers.Control, Key.OemPlus) });
			Add(new() { Shortcut = DebuggerShortcut.ZoomOut, KeyBinding = new(KeyModifiers.Control, Key.OemMinus) });

			Add(new() { Shortcut = DebuggerShortcut.SaveAsPng, KeyBinding = new(KeyModifiers.Control, Key.S) });
			Add(new() { Shortcut = DebuggerShortcut.ToggleSettingsPanel, KeyBinding = new(KeyModifiers.Control, Key.Q) });

			Add(new() { Shortcut = DebuggerShortcut.OpenAssembler, KeyBinding = new(KeyModifiers.Control, Key.U) });
			Add(new() { Shortcut = DebuggerShortcut.OpenDebugger, KeyBinding = new(KeyModifiers.Control, Key.D) });
			Add(new() { Shortcut = DebuggerShortcut.OpenSpcDebugger, KeyBinding = new(KeyModifiers.Control, Key.F) });
			Add(new() { Shortcut = DebuggerShortcut.OpenSa1Debugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenSt018Debugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenGsuDebugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenNecDspDebugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenCx4Debugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenGameboyDebugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenEventViewer, KeyBinding = new(KeyModifiers.Control, Key.E) });
			Add(new() { Shortcut = DebuggerShortcut.OpenMemoryTools, KeyBinding = new(KeyModifiers.Control, Key.M) });
			Add(new() { Shortcut = DebuggerShortcut.OpenProfiler, KeyBinding = new(KeyModifiers.Control, Key.Y) });
			Add(new() { Shortcut = DebuggerShortcut.OpenScriptWindow, KeyBinding = new(KeyModifiers.Control, Key.N) });
			Add(new() { Shortcut = DebuggerShortcut.OpenWatchWindow, KeyBinding = new(KeyModifiers.Control, Key.W) });
			Add(new() { Shortcut = DebuggerShortcut.OpenTraceLogger, KeyBinding = new(KeyModifiers.Control, Key.J) });
			Add(new() { Shortcut = DebuggerShortcut.OpenMemorySearch, KeyBinding = new(KeyModifiers.Control, Key.I) });
			Add(new() { Shortcut = DebuggerShortcut.OpenRegisterViewer, KeyBinding = new(KeyModifiers.Control, Key.K) });
			Add(new() { Shortcut = DebuggerShortcut.OpenDebugLog, KeyBinding = new(KeyModifiers.Control, Key.B) });
			Add(new() { Shortcut = DebuggerShortcut.OpenNesHeaderEditor, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenDebugSettings, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.OpenTilemapViewer, KeyBinding = new(KeyModifiers.Control, Key.D1) });
			Add(new() { Shortcut = DebuggerShortcut.OpenTileViewer, KeyBinding = new(KeyModifiers.Control, Key.D2) });
			Add(new() { Shortcut = DebuggerShortcut.OpenSpriteViewer, KeyBinding = new(KeyModifiers.Control, Key.D3) });
			Add(new() { Shortcut = DebuggerShortcut.OpenPaletteViewer, KeyBinding = new(KeyModifiers.Control, Key.D4) });

			//Debugger window
			Add(new() { Shortcut = DebuggerShortcut.Reset, KeyBinding = new(KeyModifiers.Control, Key.R) });
			Add(new() { Shortcut = DebuggerShortcut.PowerCycle, KeyBinding = new(KeyModifiers.Control, Key.T) });
			Add(new() { Shortcut = DebuggerShortcut.ReloadRom, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.R) });

			Add(new() { Shortcut = DebuggerShortcut.ToggleBreakContinue, KeyBinding = new(Key.Escape) });
			Add(new() { Shortcut = DebuggerShortcut.Continue, KeyBinding = new(Key.F5) });
			Add(new() { Shortcut = DebuggerShortcut.Break, KeyBinding = new(KeyModifiers.Shift, Key.F5) });
			Add(new() { Shortcut = DebuggerShortcut.StepInto, KeyBinding = new(Key.F11) });
			Add(new() { Shortcut = DebuggerShortcut.StepOver, KeyBinding = new(Key.F10) });
			Add(new() { Shortcut = DebuggerShortcut.StepOut, KeyBinding = new(KeyModifiers.Shift, Key.F11) });
			Add(new() { Shortcut = DebuggerShortcut.StepBack, KeyBinding = new(KeyModifiers.Shift, Key.F10) });
			Add(new() { Shortcut = DebuggerShortcut.StepBackScanline, KeyBinding = new(KeyModifiers.Shift, Key.F7) });
			Add(new() { Shortcut = DebuggerShortcut.StepBackFrame, KeyBinding = new(KeyModifiers.Shift, Key.F8) });

			Add(new() { Shortcut = DebuggerShortcut.RunCpuCycle, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.RunPpuCycle, KeyBinding = new(Key.F6) });
			Add(new() { Shortcut = DebuggerShortcut.RunPpuScanline, KeyBinding = new(Key.F7) });
			Add(new() { Shortcut = DebuggerShortcut.RunPpuFrame, KeyBinding = new(Key.F8) });

			Add(new() { Shortcut = DebuggerShortcut.RunToIrq, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.RunToNmi, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.BreakIn, KeyBinding = new(KeyModifiers.Control, Key.B) });
			Add(new() { Shortcut = DebuggerShortcut.BreakOn, KeyBinding = new(KeyModifiers.Alt, Key.B) });

			Add(new() { Shortcut = DebuggerShortcut.FindOccurrences, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F) });
			
			Add(new() { Shortcut = DebuggerShortcut.GoToProgramCounter, KeyBinding = new(KeyModifiers.Alt, Key.Multiply) });
			Add(new() { Shortcut = DebuggerShortcut.GoToCpuVector1, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.GoToCpuVector2, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.GoToCpuVector3, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.GoToCpuVector4, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.GoToCpuVector5, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_ViewInMemoryViewer, KeyBinding = new(Key.F1) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_AddToWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_GoToLocation, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_MoveProgramCounter, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F10) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_RunToLocation, KeyBinding = new(KeyModifiers.Control, Key.F11) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_EditSelectedCode, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_EditLabel, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_EditComment, KeyBinding = new(Key.OemSemicolon) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_ToggleBreakpoint, KeyBinding = new(Key.F9) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_SwitchView, KeyBinding = new(KeyModifiers.Control, Key.Q) });
			
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_NavigateBack, KeyBinding = new(KeyModifiers.Control, Key.OemMinus) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_NavigateForward, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.OemMinus) });

			Add(new() { Shortcut = DebuggerShortcut.LabelList_Add, KeyBinding = new(Key.Insert) });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_Edit, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_Delete, KeyBinding = new(Key.Delete) });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_ToggleBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_AddToWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_FindOccurrences, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_ViewInMemoryViewer, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_GoToLocation, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.FunctionList_EditLabel, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.FunctionList_ToggleBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.FunctionList_ViewInMemoryViewer, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.FunctionList_GoToLocation, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.FunctionList_FindOccurrences, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_Add, KeyBinding = new(Key.Insert) });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_AddForbid, KeyBinding = new(KeyModifiers.Control, Key.Insert) });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_Edit, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_GoToLocation, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_ViewInMemoryViewer, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_Delete, KeyBinding = new(Key.Delete) });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_EnableBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_DisableBreakpoint, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.WatchList_Delete, KeyBinding = new(Key.Delete) });
			Add(new() { Shortcut = DebuggerShortcut.WatchList_MoveUp, KeyBinding = new(KeyModifiers.Alt, Key.Up) });
			Add(new() { Shortcut = DebuggerShortcut.WatchList_MoveDown, KeyBinding = new(KeyModifiers.Alt, Key.Down) });

			Add(new() { Shortcut = DebuggerShortcut.CallStack_EditLabel, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.CallStack_GoToLocation, KeyBinding = new() });
			
			Add(new() { Shortcut = DebuggerShortcut.FindResultList_AddWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.FindResultList_GoToLocation, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.FindResultList_ToggleBreakpoint, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.SaveRom, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.SaveRomAs, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.SaveEditAsIps, KeyBinding = new() });
			
			Add(new() { Shortcut = DebuggerShortcut.ResetCdl, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LoadCdl, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.SaveCdl, KeyBinding = new() });
		
			Add(new() { Shortcut = DebuggerShortcut.ImportLabels, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.ExportLabels, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.ImportWatchEntries, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.ExportWatchEntries, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.ResetWorkspace, KeyBinding = new() });

			//Tilemap viewer
			Add(new() { Shortcut = DebuggerShortcut.TilemapViewer_ViewInMemoryViewer, KeyBinding = new(Key.F1) });
			Add(new() { Shortcut = DebuggerShortcut.TilemapViewer_EditTile, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.TilemapViewer_ViewInTileViewer, KeyBinding = new(Key.F3) });
			Add(new() { Shortcut = DebuggerShortcut.TilemapViewer_EditTilemapBreakpoint, KeyBinding = new(Key.F4) });
			Add(new() { Shortcut = DebuggerShortcut.TilemapViewer_EditAttributeBreakpoint, KeyBinding = new() });

			//Sprite viewer
			Add(new() { Shortcut = DebuggerShortcut.SpriteViewer_ViewInMemoryViewer, KeyBinding = new(Key.F1) });
			Add(new() { Shortcut = DebuggerShortcut.SpriteViewer_ViewInTileViewer, KeyBinding = new(Key.F3) });
			Add(new() { Shortcut = DebuggerShortcut.SpriteViewer_EditSprite, KeyBinding = new(Key.F2) });

			//Tile viewer
			Add(new() { Shortcut = DebuggerShortcut.TileViewer_ViewInMemoryViewer, KeyBinding = new(Key.F1) });
			Add(new() { Shortcut = DebuggerShortcut.TileViewer_EditTile, KeyBinding = new(Key.F2) });
			
			//Palette viewer
			Add(new() { Shortcut = DebuggerShortcut.PaletteViewer_EditColor, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.PaletteViewer_ViewInMemoryViewer, KeyBinding = new(Key.F1) });

			//Memory Tools
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_Freeze, KeyBinding = new(KeyModifiers.Control, Key.Q) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_Unfreeze, KeyBinding = new(KeyModifiers.Control, Key.W) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_AddToWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_EditBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_EditLabel, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_Import, KeyBinding = new(KeyModifiers.Control, Key.O) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_Export, KeyBinding = new(KeyModifiers.Control, Key.S) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_ResetAccessCounters, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInMemoryType, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInDebugger, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToPrevCode, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToNextCode, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToPrevData, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToNextData, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToPrevUnknown, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToNextUnknown, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToPrevRead, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToNextRead, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToPrevWrite, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToNextWrite, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToPrevExec, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_GoToNextExec, KeyBinding = new() });

			//Trace Logger
			Add(new() { Shortcut = DebuggerShortcut.TraceLogger_EditBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.TraceLogger_EditLabel, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.TraceLogger_ViewInDebugger, KeyBinding = new(KeyModifiers.Control, Key.F1) });
			Add(new() { Shortcut = DebuggerShortcut.TraceLogger_ViewInMemoryViewer, KeyBinding = new(Key.F1) });

			//Script Window
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_NewScript, KeyBinding = new(KeyModifiers.Control, Key.N) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_OpenScript, KeyBinding = new(KeyModifiers.Control, Key.O) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_SaveScript, KeyBinding = new(KeyModifiers.Control, Key.S) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_RunScript, KeyBinding = new(Key.F5) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_StopScript, KeyBinding = new(Key.Escape) });

			//Register viewer
			Add(new() { Shortcut = DebuggerShortcut.RegisterViewer_EditBreakpoint, KeyBinding = new(Key.F2) });

			//Tile editor
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_RotateLeft, KeyBinding = new(KeyModifiers.Control, Key.Left) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_RotateRight, KeyBinding = new(KeyModifiers.Control, Key.Right) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_FlipHorizontal, KeyBinding = new(KeyModifiers.Control, Key.Up) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_FlipVertical, KeyBinding = new(KeyModifiers.Control, Key.Down) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_TranslateLeft, KeyBinding = new(KeyModifiers.Shift, Key.Left) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_TranslateRight, KeyBinding = new(KeyModifiers.Shift, Key.Right) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_TranslateUp, KeyBinding = new(KeyModifiers.Shift, Key.Up) });
			Add(new() { Shortcut = DebuggerShortcut.TileEditor_TranslateDown, KeyBinding = new(KeyModifiers.Shift, Key.Down) });
		}
	}

	public enum DebuggerShortcut
	{
		GoToAddress,
		Find,
		FindNext,
		FindPrev,
		Copy,
		Paste,
		Undo,
		SelectAll,
		Refresh,
		MarkAsCode,
		MarkAsData,
		MarkAsUnidentified,
		GoToAll,
		ZoomIn,
		ZoomOut,
		SaveAsPng,
		ToggleSettingsPanel,
		CodeWindow_ViewInMemoryViewer,
		MemoryViewer_ViewInDebugger,
		OpenAssembler,
		OpenDebugger,
		OpenSpcDebugger,
		OpenSa1Debugger,
		OpenSt018Debugger,
		OpenGsuDebugger,
		OpenNecDspDebugger,
		OpenCx4Debugger,
		OpenGameboyDebugger,
		OpenEventViewer,
		OpenMemoryTools,
		OpenProfiler,
		OpenScriptWindow,
		OpenWatchWindow,
		OpenMemorySearch,
		OpenTraceLogger,
		OpenRegisterViewer,
		OpenDebugLog,
		OpenTilemapViewer,
		OpenTileViewer,
		OpenSpriteViewer,
		OpenPaletteViewer,
		OpenNesHeaderEditor,
		OpenDebugSettings,
		Reset,
		PowerCycle,
		ReloadRom,
		ToggleBreakContinue,
		Continue,
		Break,
		StepInto,
		StepOver,
		StepOut,
		StepBack,
		StepBackScanline,
		StepBackFrame,
		RunCpuCycle,
		RunPpuCycle,
		RunPpuScanline,
		RunPpuFrame,
		BreakIn,
		BreakOn,
		FindOccurrences,
		GoToProgramCounter,
		GoToCpuVector1,
		GoToCpuVector2,
		GoToCpuVector3,
		GoToCpuVector4,
		GoToCpuVector5,
		CodeWindow_MoveProgramCounter,
		CodeWindow_RunToLocation,
		CodeWindow_EditSelectedCode,
		CodeWindow_EditLabel,
		CodeWindow_EditComment,
		CodeWindow_NavigateBack,
		CodeWindow_NavigateForward,
		CodeWindow_ToggleBreakpoint,
		CodeWindow_SwitchView,
		LabelList_Add,
		LabelList_Edit,
		LabelList_Delete,
		LabelList_ToggleBreakpoint,
		LabelList_AddToWatch,
		LabelList_FindOccurrences,
		LabelList_ViewInMemoryViewer,
		LabelList_GoToLocation,
		FunctionList_EditLabel,
		FunctionList_ToggleBreakpoint,
		FunctionList_GoToLocation,
		FunctionList_FindOccurrences,
		FunctionList_ViewInMemoryViewer,
		BreakpointList_Add,
		BreakpointList_AddForbid,
		BreakpointList_Edit,
		BreakpointList_GoToLocation,
		BreakpointList_ViewInMemoryViewer,
		BreakpointList_Delete,
		BreakpointList_EnableBreakpoint,
		BreakpointList_DisableBreakpoint,
		WatchList_Add,
		WatchList_Edit,
		WatchList_Delete,
		WatchList_MoveUp,
		WatchList_MoveDown,
		CallStack_EditLabel,
		CallStack_GoToLocation,
		FindResultList_GoToLocation,
		FindResultList_AddWatch,
		FindResultList_ToggleBreakpoint,
		SaveRom,
		SaveRomAs,
		SaveEditAsIps,
		MemoryViewer_Freeze,
		MemoryViewer_Unfreeze,
		MemoryViewer_AddToWatch,
		MemoryViewer_EditBreakpoint,
		MemoryViewer_EditLabel,
		MemoryViewer_Import,
		MemoryViewer_Export,
		MemoryViewer_ResetAccessCounters,
		MemoryViewer_ViewInMemoryType,

		MemoryViewer_GoToPrevCode,
		MemoryViewer_GoToNextCode,
		MemoryViewer_GoToPrevData,
		MemoryViewer_GoToNextData,
		MemoryViewer_GoToPrevUnknown,
		MemoryViewer_GoToNextUnknown,
		MemoryViewer_GoToPrevRead,
		MemoryViewer_GoToNextRead,
		MemoryViewer_GoToPrevWrite,
		MemoryViewer_GoToNextWrite,
		MemoryViewer_GoToPrevExec,
		MemoryViewer_GoToNextExec,

		ScriptWindow_NewScript,
		ScriptWindow_OpenScript,
		ScriptWindow_SaveScript,
		ScriptWindow_RunScript,
		ScriptWindow_StopScript,
		CodeWindow_AddToWatch,
		CodeWindow_GoToLocation,
		ResetCdl,
		LoadCdl,
		SaveCdl,
		RunToNmi,
		RunToIrq,
		ImportLabels,
		ExportLabels,
		ImportWatchEntries,
		ExportWatchEntries,
		ResetWorkspace,
		TilemapViewer_EditTilemapBreakpoint,
		TilemapViewer_EditAttributeBreakpoint,
		TilemapViewer_ViewInTileViewer,
		TilemapViewer_ViewInMemoryViewer,
		TilemapViewer_EditTile,
		SpriteViewer_ViewInMemoryViewer,
		SpriteViewer_ViewInTileViewer,
		SpriteViewer_EditSprite,
		TileViewer_ViewInMemoryViewer,
		TileViewer_EditTile,
		PaletteViewer_EditColor,
		PaletteViewer_ViewInMemoryViewer,
		TraceLogger_EditBreakpoint,
		TraceLogger_EditLabel,
		TraceLogger_ViewInDebugger,
		TraceLogger_ViewInMemoryViewer,
		RegisterViewer_EditBreakpoint,
		TileEditor_RotateRight,
		TileEditor_RotateLeft,
		TileEditor_FlipHorizontal,
		TileEditor_FlipVertical,
		TileEditor_TranslateLeft,
		TileEditor_TranslateRight,
		TileEditor_TranslateUp,
		TileEditor_TranslateDown,
	}

	public class DebuggerShortcutInfo : ViewModelBase
	{
		[Reactive] public DebuggerShortcut Shortcut { get; set; }
		[Reactive] public DbgShortKeys KeyBinding { get; set; } = new();
	}

	public class DbgShortKeys
	{
		private static Regex _numberKeyRegex = new Regex("D[0-9]", RegexOptions.Compiled);

		public KeyModifiers Modifiers { get; set; }
		public Key ShortcutKey { get; set; }

		public DbgShortKeys() { }

		public DbgShortKeys(Key key) : this(KeyModifiers.None, key) { }

		public DbgShortKeys(KeyModifiers modifiers, Key key)
		{
			Modifiers = modifiers;
			ShortcutKey = key;
		}

		public override string ToString()
		{
			if(ShortcutKey != Key.None) {
				KeyModifiers modifiers = Modifiers;
				switch(ShortcutKey) {
					case Key.LeftAlt:
					case Key.LeftShift:
					case Key.LeftCtrl:
					case Key.RightAlt:
					case Key.RightShift:
					case Key.RightCtrl:
						//Display "LeftCtrl" as "LeftCtrl" instead of "Ctrl+LeftCtrl", etc.
						modifiers = KeyModifiers.None;
						break;
				}

				string shortcut = new KeyGesture(ShortcutKey, modifiers).ToString();
				shortcut = shortcut.Replace("Oem", "");
				
				//Rename D0-D9 to 0-9
				shortcut = _numberKeyRegex.Replace(shortcut, (Match match) => match.Value.Substring(1));

				return shortcut;
			}
			return "";
		}
	}
}
