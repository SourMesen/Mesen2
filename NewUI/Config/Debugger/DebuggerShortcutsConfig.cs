using Avalonia.Input;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Config
{
	public class DebuggerShortcutsConfig
	{
		public DbgShortKeys Get(DebuggerShortcut shortcut)
		{
			DbgShortKeys? binding = Shortcuts.Find(s => s.Shortcut == shortcut)?.KeyBinding;
			if(binding == null) {
				throw new Exception("Invalid shortcut");
			}
			return binding;
		}

		internal DebuggerShortcutInfo GetBindable(DebuggerShortcut shortcut)
		{
			DebuggerShortcutInfo? binding = Shortcuts.Find(s => s.Shortcut == shortcut);
			if(binding == null) {
				throw new Exception("Invalid shortcut");
			}
			return binding;
		}

		public List<DebuggerShortcutInfo> Shortcuts { get; set; } = new() {
			//Shared
			new() { Shortcut = DebuggerShortcut.IncreaseFontSize, KeyBinding = new(KeyModifiers.Control, Key.OemPlus) },
			new() { Shortcut = DebuggerShortcut.DecreaseFontSize, KeyBinding = new(KeyModifiers.Control, Key.OemMinus) },
			new() { Shortcut = DebuggerShortcut.ResetFontSize, KeyBinding = new(KeyModifiers.Control, Key.D0) },

			new() { Shortcut = DebuggerShortcut.GoTo, KeyBinding = new(KeyModifiers.Control, Key.G) },

			new() { Shortcut = DebuggerShortcut.Find, KeyBinding = new(KeyModifiers.Control, Key.F) },
			new() { Shortcut = DebuggerShortcut.FindNext, KeyBinding = new(Key.F3) },
			new() { Shortcut = DebuggerShortcut.FindPrev, KeyBinding = new(KeyModifiers.Shift, Key.F3) },

			new() { Shortcut = DebuggerShortcut.Undo, KeyBinding = new(KeyModifiers.Control, Key.Z) },
			new() { Shortcut = DebuggerShortcut.Copy, KeyBinding = new(KeyModifiers.Control, Key.C) },
			new() { Shortcut = DebuggerShortcut.Cut, KeyBinding = new(KeyModifiers.Control, Key.X) },
			new() { Shortcut = DebuggerShortcut.Paste, KeyBinding = new(KeyModifiers.Control, Key.V) },
			new() { Shortcut = DebuggerShortcut.SelectAll, KeyBinding = new(KeyModifiers.Control, Key.A) },

			new() { Shortcut = DebuggerShortcut.Refresh, KeyBinding = new(Key.F5) },

			new() { Shortcut = DebuggerShortcut.MarkAsCode, KeyBinding = new(KeyModifiers.Control, Key.D1) },
			new() { Shortcut = DebuggerShortcut.MarkAsData, KeyBinding = new(KeyModifiers.Control, Key.D2) },
			new() { Shortcut = DebuggerShortcut.MarkAsUnidentified, KeyBinding = new(KeyModifiers.Control, Key.D3) },

			new() { Shortcut = DebuggerShortcut.GoToAll, KeyBinding = new(KeyModifiers.Control, Key.OemComma) },

			new() { Shortcut = DebuggerShortcut.ZoomIn, KeyBinding = new(KeyModifiers.Control, Key.OemPlus) },
			new() { Shortcut = DebuggerShortcut.ZoomOut, KeyBinding = new(KeyModifiers.Control, Key.OemMinus) },

			new() { Shortcut = DebuggerShortcut.SaveAsPng, KeyBinding = new(KeyModifiers.Control, Key.S) },

			new() { Shortcut = DebuggerShortcut.CodeWindow_EditInMemoryViewer, KeyBinding = new(Key.F1) },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInDisassembly, KeyBinding = new() },

			new() { Shortcut = DebuggerShortcut.OpenAssembler, KeyBinding = new(KeyModifiers.Control, Key.U) },
			new() { Shortcut = DebuggerShortcut.OpenDebugger, KeyBinding = new(KeyModifiers.Control, Key.D) },
			new() { Shortcut = DebuggerShortcut.OpenSpcDebugger, KeyBinding = new(KeyModifiers.Control, Key.F) },
			new() { Shortcut = DebuggerShortcut.OpenSa1Debugger, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.OpenGsuDebugger, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.OpenNecDspDebugger, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.OpenCx4Debugger, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.OpenGameboyDebugger, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.OpenEventViewer, KeyBinding = new(KeyModifiers.Control, Key.E) },
			new() { Shortcut = DebuggerShortcut.OpenMemoryTools, KeyBinding = new(KeyModifiers.Control, Key.M) },
			new() { Shortcut = DebuggerShortcut.OpenProfiler, KeyBinding = new(KeyModifiers.Control, Key.Y) },
			new() { Shortcut = DebuggerShortcut.OpenScriptWindow, KeyBinding = new(KeyModifiers.Control, Key.N) },
			new() { Shortcut = DebuggerShortcut.OpenTraceLogger, KeyBinding = new(KeyModifiers.Control, Key.J) },
			new() { Shortcut = DebuggerShortcut.OpenRegisterViewer, KeyBinding = new(KeyModifiers.Control, Key.K) },
			new() { Shortcut = DebuggerShortcut.OpenDebugLog, KeyBinding = new(KeyModifiers.Control, Key.B) },

			new() { Shortcut = DebuggerShortcut.OpenTilemapViewer, KeyBinding = new(KeyModifiers.Control, Key.D1) },
			new() { Shortcut = DebuggerShortcut.OpenTileViewer, KeyBinding = new(KeyModifiers.Control, Key.D2) },
			new() { Shortcut = DebuggerShortcut.OpenSpriteViewer, KeyBinding = new(KeyModifiers.Control, Key.D3) },
			new() { Shortcut = DebuggerShortcut.OpenPaletteViewer, KeyBinding = new(KeyModifiers.Control, Key.D4) },

			//Debugger window
			new() { Shortcut = DebuggerShortcut.Reset, KeyBinding = new(KeyModifiers.Control, Key.R) },
			new() { Shortcut = DebuggerShortcut.PowerCycle, KeyBinding = new(KeyModifiers.Control, Key.T) },
			new() { Shortcut = DebuggerShortcut.ReloadRom, KeyBinding = new() },

			new() { Shortcut = DebuggerShortcut.Continue, KeyBinding = new(Key.F5) },
			new() { Shortcut = DebuggerShortcut.Break, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Alt, Key.Cancel) },
			new() { Shortcut = DebuggerShortcut.ToggleBreakContinue, KeyBinding = new(Key.Escape) },
			new() { Shortcut = DebuggerShortcut.StepInto, KeyBinding = new(Key.F11) },
			new() { Shortcut = DebuggerShortcut.StepOver, KeyBinding = new(Key.F10) },
			new() { Shortcut = DebuggerShortcut.StepOut, KeyBinding = new(KeyModifiers.Shift, Key.F11) },
			new() { Shortcut = DebuggerShortcut.StepBack, KeyBinding = new(KeyModifiers.Shift, Key.F10) },

			new() { Shortcut = DebuggerShortcut.RunCpuCycle, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.RunPpuCycle, KeyBinding = new(Key.F6) },
			new() { Shortcut = DebuggerShortcut.RunPpuScanline, KeyBinding = new(Key.F7) },
			new() { Shortcut = DebuggerShortcut.RunPpuFrame, KeyBinding = new(Key.F8) },

			new() { Shortcut = DebuggerShortcut.BreakIn, KeyBinding = new(KeyModifiers.Control, Key.B) },
			new() { Shortcut = DebuggerShortcut.BreakOn, KeyBinding = new(KeyModifiers.Alt, Key.B) },

			new() { Shortcut = DebuggerShortcut.FindOccurrences, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F) },
			new() { Shortcut = DebuggerShortcut.GoToProgramCounter, KeyBinding = new(KeyModifiers.Alt, Key.Multiply) },

			new() { Shortcut = DebuggerShortcut.CodeWindow_SetNextStatement, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F10) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_EditSelectedCode, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.CodeWindow_EditSourceFile, KeyBinding = new(Key.F4) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_EditLabel, KeyBinding = new(Key.F2) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_NavigateBack, KeyBinding = new(KeyModifiers.Alt, Key.Left) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_NavigateForward, KeyBinding = new(KeyModifiers.Alt, Key.Right) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_ToggleBreakpoint, KeyBinding = new(Key.F9) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_DisableEnableBreakpoint, KeyBinding = new(KeyModifiers.Control, Key.F9) },
			new() { Shortcut = DebuggerShortcut.CodeWindow_SwitchView, KeyBinding = new(KeyModifiers.Control, Key.Q) },

			new() { Shortcut = DebuggerShortcut.LabelList_Add, KeyBinding = new(Key.Insert) },
			new() { Shortcut = DebuggerShortcut.LabelList_Edit, KeyBinding = new(Key.F2) },
			new() { Shortcut = DebuggerShortcut.LabelList_Delete, KeyBinding = new(Key.Delete) },
			new() { Shortcut = DebuggerShortcut.LabelList_AddBreakpoint, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.LabelList_AddToWatch, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.LabelList_FindOccurrences, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.LabelList_ViewInCpuMemory, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.LabelList_ViewInMemoryType, KeyBinding = new() },

			new() { Shortcut = DebuggerShortcut.BreakpointList_Add, KeyBinding = new(Key.Insert) },
			new() { Shortcut = DebuggerShortcut.BreakpointList_Edit, KeyBinding = new(Key.F2) },
			new() { Shortcut = DebuggerShortcut.BreakpointList_GoToLocation, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.BreakpointList_Delete, KeyBinding = new(Key.Delete) },

			new() { Shortcut = DebuggerShortcut.WatchList_Delete, KeyBinding = new(Key.Delete) },
			new() { Shortcut = DebuggerShortcut.WatchList_MoveUp, KeyBinding = new(KeyModifiers.Alt, Key.Up) },
			new() { Shortcut = DebuggerShortcut.WatchList_MoveDown, KeyBinding = new(KeyModifiers.Alt, Key.Down) },

			new() { Shortcut = DebuggerShortcut.SaveRom, KeyBinding = new(KeyModifiers.Control, Key.S) },
			new() { Shortcut = DebuggerShortcut.SaveRomAs, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.SaveEditAsIps, KeyBinding = new() },
			
			//Memory Tools
			//new() { Shortcut = eDebuggerShortcut.MemoryViewer_Freeze, KeyBinding = new(KeyModifiers.Control, Key.Q) },
			//new() { Shortcut = eDebuggerShortcut.MemoryViewer_Unfreeze, KeyBinding = new(KeyModifiers.Control, Key.W) },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_AddToWatch, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_EditBreakpoint, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_EditLabel, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_Import, KeyBinding = new(KeyModifiers.Control, Key.O) },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_Export, KeyBinding = new(KeyModifiers.Control, Key.S) },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInCpuMemory, KeyBinding = new() },
			new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInMemoryType, KeyBinding = new() },

			//Script Window
			new() { Shortcut = DebuggerShortcut.ScriptWindow_OpenScript, KeyBinding = new(KeyModifiers.Control, Key.N) },
			new() { Shortcut = DebuggerShortcut.ScriptWindow_SaveScript, KeyBinding = new(KeyModifiers.Control, Key.S) },
			new() { Shortcut = DebuggerShortcut.ScriptWindow_RunScript, KeyBinding = new(Key.F5) },
			new() { Shortcut = DebuggerShortcut.ScriptWindow_StopScript, KeyBinding = new(Key.Escape) },
		};
	}

	public enum DebuggerShortcut
	{
		IncreaseFontSize,
		DecreaseFontSize,
		ResetFontSize,
		GoTo,
		Find,
		FindNext,
		FindPrev,
		Undo,
		Copy,
		Cut,
		Paste,
		SelectAll,
		Refresh,
		MarkAsCode,
		MarkAsData,
		MarkAsUnidentified,
		GoToAll,
		ZoomIn,
		ZoomOut,
		SaveAsPng,
		CodeWindow_EditInMemoryViewer,
		MemoryViewer_ViewInDisassembly,
		OpenAssembler,
		OpenDebugger,
		OpenSpcDebugger,
		OpenSa1Debugger,
		OpenGsuDebugger,
		OpenNecDspDebugger,
		OpenCx4Debugger,
		OpenGameboyDebugger,
		OpenEventViewer,
		OpenMemoryTools,
		OpenProfiler,
		OpenScriptWindow,
		OpenTraceLogger,
		OpenRegisterViewer,
		OpenDebugLog,
		OpenTilemapViewer,
		OpenTileViewer,
		OpenSpriteViewer,
		OpenPaletteViewer,
		Reset,
		PowerCycle,
		ReloadRom,
		Continue,
		Break,
		ToggleBreakContinue,
		StepInto,
		StepOver,
		StepOut,
		StepBack,
		RunCpuCycle,
		RunPpuCycle,
		RunPpuScanline,
		RunPpuFrame,
		BreakIn,
		BreakOn,
		FindOccurrences,
		GoToProgramCounter,
		CodeWindow_SetNextStatement,
		CodeWindow_EditSelectedCode,
		CodeWindow_EditSourceFile,
		CodeWindow_EditLabel,
		CodeWindow_NavigateBack,
		CodeWindow_NavigateForward,
		CodeWindow_ToggleBreakpoint,
		CodeWindow_DisableEnableBreakpoint,
		CodeWindow_SwitchView,
		LabelList_Add,
		LabelList_Edit,
		LabelList_Delete,
		LabelList_AddBreakpoint,
		LabelList_AddToWatch,
		LabelList_FindOccurrences,
		LabelList_ViewInCpuMemory,
		LabelList_ViewInMemoryType,
		BreakpointList_Add,
		BreakpointList_Edit,
		BreakpointList_GoToLocation,
		BreakpointList_Delete,
		WatchList_Delete,
		WatchList_MoveUp,
		WatchList_MoveDown,
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
		MemoryViewer_ViewInCpuMemory,
		MemoryViewer_ViewInMemoryType,
		ScriptWindow_OpenScript,
		ScriptWindow_SaveScript,
		ScriptWindow_RunScript,
		ScriptWindow_StopScript
	}

	public class DebuggerShortcutInfo : ViewModelBase
	{
		[Reactive] public DebuggerShortcut Shortcut { get; set; }
		[Reactive] public DbgShortKeys KeyBinding { get; set; } = new();
	}

	public class DbgShortKeys
	{
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
			return ShortcutKey != Key.None ? new KeyGesture(ShortcutKey, Modifiers).ToString().Replace("Oem", "") : "";
		}
	}
}
