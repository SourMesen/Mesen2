using Avalonia.Input;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace Mesen.Config
{
	public class DebuggerShortcutsConfig : IJsonOnDeserialized
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
			Add(new() { Shortcut = DebuggerShortcut.IncreaseFontSize, KeyBinding = new(KeyModifiers.Control, Key.OemPlus) });
			Add(new() { Shortcut = DebuggerShortcut.DecreaseFontSize, KeyBinding = new(KeyModifiers.Control, Key.OemMinus) });
			Add(new() { Shortcut = DebuggerShortcut.ResetFontSize, KeyBinding = new(KeyModifiers.Control, Key.D0) });

			Add(new() { Shortcut = DebuggerShortcut.GoTo, KeyBinding = new(KeyModifiers.Control, Key.G) });

			Add(new() { Shortcut = DebuggerShortcut.Find, KeyBinding = new(KeyModifiers.Control, Key.F) });
			Add(new() { Shortcut = DebuggerShortcut.FindNext, KeyBinding = new(Key.F3) });
			Add(new() { Shortcut = DebuggerShortcut.FindPrev, KeyBinding = new(KeyModifiers.Shift, Key.F3) });

			Add(new() { Shortcut = DebuggerShortcut.Undo, KeyBinding = new(KeyModifiers.Control, Key.Z) });
			Add(new() { Shortcut = DebuggerShortcut.Copy, KeyBinding = new(KeyModifiers.Control, Key.C) });
			Add(new() { Shortcut = DebuggerShortcut.Cut, KeyBinding = new(KeyModifiers.Control, Key.X) });
			Add(new() { Shortcut = DebuggerShortcut.Paste, KeyBinding = new(KeyModifiers.Control, Key.V) });
			Add(new() { Shortcut = DebuggerShortcut.SelectAll, KeyBinding = new(KeyModifiers.Control, Key.A) });

			Add(new() { Shortcut = DebuggerShortcut.Refresh, KeyBinding = new(Key.F5) });

			Add(new() { Shortcut = DebuggerShortcut.MarkAsCode, KeyBinding = new(KeyModifiers.Control, Key.D1) });
			Add(new() { Shortcut = DebuggerShortcut.MarkAsData, KeyBinding = new(KeyModifiers.Control, Key.D2) });
			Add(new() { Shortcut = DebuggerShortcut.MarkAsUnidentified, KeyBinding = new(KeyModifiers.Control, Key.D3) });

			Add(new() { Shortcut = DebuggerShortcut.GoToAll, KeyBinding = new(KeyModifiers.Control, Key.OemComma) });

			Add(new() { Shortcut = DebuggerShortcut.ZoomIn, KeyBinding = new(KeyModifiers.Control, Key.OemPlus) });
			Add(new() { Shortcut = DebuggerShortcut.ZoomOut, KeyBinding = new(KeyModifiers.Control, Key.OemMinus) });

			Add(new() { Shortcut = DebuggerShortcut.SaveAsPng, KeyBinding = new(KeyModifiers.Control, Key.S) });

			Add(new() { Shortcut = DebuggerShortcut.OpenAssembler, KeyBinding = new(KeyModifiers.Control, Key.U) });
			Add(new() { Shortcut = DebuggerShortcut.OpenDebugger, KeyBinding = new(KeyModifiers.Control, Key.D) });
			Add(new() { Shortcut = DebuggerShortcut.OpenSpcDebugger, KeyBinding = new(KeyModifiers.Control, Key.F) });
			Add(new() { Shortcut = DebuggerShortcut.OpenSa1Debugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenGsuDebugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenNecDspDebugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenCx4Debugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenGameboyDebugger, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.OpenEventViewer, KeyBinding = new(KeyModifiers.Control, Key.E) });
			Add(new() { Shortcut = DebuggerShortcut.OpenMemoryTools, KeyBinding = new(KeyModifiers.Control, Key.M) });
			Add(new() { Shortcut = DebuggerShortcut.OpenProfiler, KeyBinding = new(KeyModifiers.Control, Key.Y) });
			Add(new() { Shortcut = DebuggerShortcut.OpenScriptWindow, KeyBinding = new(KeyModifiers.Control, Key.N) });
			Add(new() { Shortcut = DebuggerShortcut.OpenTraceLogger, KeyBinding = new(KeyModifiers.Control, Key.J) });
			Add(new() { Shortcut = DebuggerShortcut.OpenRegisterViewer, KeyBinding = new(KeyModifiers.Control, Key.K) });
			Add(new() { Shortcut = DebuggerShortcut.OpenDebugLog, KeyBinding = new(KeyModifiers.Control, Key.B) });
			Add(new() { Shortcut = DebuggerShortcut.OpenDebugSettings, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.OpenTilemapViewer, KeyBinding = new(KeyModifiers.Control, Key.D1) });
			Add(new() { Shortcut = DebuggerShortcut.OpenTileViewer, KeyBinding = new(KeyModifiers.Control, Key.D2) });
			Add(new() { Shortcut = DebuggerShortcut.OpenSpriteViewer, KeyBinding = new(KeyModifiers.Control, Key.D3) });
			Add(new() { Shortcut = DebuggerShortcut.OpenPaletteViewer, KeyBinding = new(KeyModifiers.Control, Key.D4) });

			//Debugger window
			Add(new() { Shortcut = DebuggerShortcut.Reset, KeyBinding = new(KeyModifiers.Control, Key.R) });
			Add(new() { Shortcut = DebuggerShortcut.PowerCycle, KeyBinding = new(KeyModifiers.Control, Key.T) });

			Add(new() { Shortcut = DebuggerShortcut.ToggleBreakContinue, KeyBinding = new(Key.Escape) });
			Add(new() { Shortcut = DebuggerShortcut.Continue, KeyBinding = new(Key.F5) });
			Add(new() { Shortcut = DebuggerShortcut.Break, KeyBinding = new(KeyModifiers.Shift, Key.F5) });
			Add(new() { Shortcut = DebuggerShortcut.StepInto, KeyBinding = new(Key.F11) });
			Add(new() { Shortcut = DebuggerShortcut.StepOver, KeyBinding = new(Key.F10) });
			Add(new() { Shortcut = DebuggerShortcut.StepOut, KeyBinding = new(KeyModifiers.Shift, Key.F11) });
			Add(new() { Shortcut = DebuggerShortcut.StepBack, KeyBinding = new(KeyModifiers.Shift, Key.F10) });

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

			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_ViewInMemoryViewer, KeyBinding = new(Key.F1) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_AddToWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_GoToLocation, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_MoveProgramCounter, KeyBinding = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F10) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_EditSelectedCode, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_EditSourceFile, KeyBinding = new(Key.F4) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_EditLabel, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_NavigateBack, KeyBinding = new(KeyModifiers.Alt, Key.Left) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_NavigateForward, KeyBinding = new(KeyModifiers.Alt, Key.Right) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_ToggleBreakpoint, KeyBinding = new(Key.F9) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_DisableEnableBreakpoint, KeyBinding = new(KeyModifiers.Control, Key.F9) });
			Add(new() { Shortcut = DebuggerShortcut.CodeWindow_SwitchView, KeyBinding = new(KeyModifiers.Control, Key.Q) });

			Add(new() { Shortcut = DebuggerShortcut.LabelList_Add, KeyBinding = new(Key.Insert) });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_Edit, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_Delete, KeyBinding = new(Key.Delete) });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_AddBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_AddToWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_FindOccurrences, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_ViewInMemoryViewer, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.LabelList_GoToLocation, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_Add, KeyBinding = new(Key.Insert) });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_Edit, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_GoToLocation, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.BreakpointList_Delete, KeyBinding = new(Key.Delete) });

			Add(new() { Shortcut = DebuggerShortcut.WatchList_Add, KeyBinding = new(Key.Insert) });
			Add(new() { Shortcut = DebuggerShortcut.WatchList_Edit, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.WatchList_Delete, KeyBinding = new(Key.Delete) });
			Add(new() { Shortcut = DebuggerShortcut.WatchList_MoveUp, KeyBinding = new(KeyModifiers.Alt, Key.Up) });
			Add(new() { Shortcut = DebuggerShortcut.WatchList_MoveDown, KeyBinding = new(KeyModifiers.Alt, Key.Down) });

			Add(new() { Shortcut = DebuggerShortcut.CallStack_EditLabel, KeyBinding = new(Key.F2) });
			Add(new() { Shortcut = DebuggerShortcut.CallStack_GoToLocation, KeyBinding = new() });

			Add(new() { Shortcut = DebuggerShortcut.SaveRom, KeyBinding = new(KeyModifiers.Control, Key.S) });
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

			//Memory Tools
			//Add(new() { Shortcut = eDebuggerShortcut.MemoryViewer_Freeze, KeyBinding = new(KeyModifiers.Control, Key.Q) });
			//Add(new() { Shortcut = eDebuggerShortcut.MemoryViewer_Unfreeze, KeyBinding = new(KeyModifiers.Control, Key.W) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_AddToWatch, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_EditBreakpoint, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_EditLabel, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_Import, KeyBinding = new(KeyModifiers.Control, Key.O) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_Export, KeyBinding = new(KeyModifiers.Control, Key.S) });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInCpuMemory, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInMemoryType, KeyBinding = new() });
			Add(new() { Shortcut = DebuggerShortcut.MemoryViewer_ViewInDebugger, KeyBinding = new() });

			//Script Window
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_NewScript, KeyBinding = new(KeyModifiers.Control, Key.N) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_OpenScript, KeyBinding = new(KeyModifiers.Control, Key.O) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_SaveScript, KeyBinding = new(KeyModifiers.Control, Key.S) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_RunScript, KeyBinding = new(Key.F5) });
			Add(new() { Shortcut = DebuggerShortcut.ScriptWindow_StopScript, KeyBinding = new(Key.Escape) });
		}
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
		CodeWindow_ViewInMemoryViewer,
		MemoryViewer_ViewInDebugger,
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
		OpenDebugSettings,
		Reset,
		PowerCycle,
		ToggleBreakContinue,
		Continue,
		Break,
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
		CodeWindow_MoveProgramCounter,
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
		LabelList_ViewInMemoryViewer,
		LabelList_GoToLocation,
		BreakpointList_Add,
		BreakpointList_Edit,
		BreakpointList_GoToLocation,
		BreakpointList_Delete,
		WatchList_Add,
		WatchList_Edit,
		WatchList_Delete,
		WatchList_MoveUp,
		WatchList_MoveDown,
		CallStack_EditLabel,
		CallStack_GoToLocation,
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
