using Avalonia.Input;
using System;

namespace Mesen.Config
{
	public class DebuggerShortcutsConfig
	{
		//Shared
		[ShortcutName("Increase Font Size")]
		public DbgShortKeys IncreaseFontSize = new(KeyModifiers.Control, Key.OemPlus);
		[ShortcutName("Decrease Font Size")]
		public DbgShortKeys DecreaseFontSize = new(KeyModifiers.Control, Key.OemMinus);
		[ShortcutName("Reset Font Size")]
		public DbgShortKeys ResetFontSize = new(KeyModifiers.Control, Key.D0);

		[ShortcutName("Go To...")]
		public DbgShortKeys GoTo = new(KeyModifiers.Control, Key.G);

		[ShortcutName("Find")]
		public DbgShortKeys Find = new(KeyModifiers.Control, Key.F);
		[ShortcutName("Find Next")]
		public DbgShortKeys FindNext = new(Key.F3);
		[ShortcutName("Find Previous")]
		public DbgShortKeys FindPrev = new(KeyModifiers.Shift, Key.F3);

		[ShortcutName("Undo")]
		public DbgShortKeys Undo = new(KeyModifiers.Control, Key.Z);
		[ShortcutName("Copy")]
		public DbgShortKeys Copy = new(KeyModifiers.Control, Key.C);
		[ShortcutName("Cut")]
		public DbgShortKeys Cut = new(KeyModifiers.Control, Key.X);
		[ShortcutName("Paste")]
		public DbgShortKeys Paste = new(KeyModifiers.Control, Key.V);
		[ShortcutName("Select All")]
		public DbgShortKeys SelectAll = new(KeyModifiers.Control, Key.A);

		[ShortcutName("Refresh")]
		public DbgShortKeys Refresh = new(Key.F5);

		[ShortcutName("Mark Selection as Code")]
		public DbgShortKeys MarkAsCode = new(KeyModifiers.Control, Key.D1);
		[ShortcutName("Mark Selection as Data")]
		public DbgShortKeys MarkAsData = new(KeyModifiers.Control, Key.D2);
		[ShortcutName("Mark Selection as Unidentified Code/Data")]
		public DbgShortKeys MarkAsUnidentified = new(KeyModifiers.Control, Key.D3);

		[ShortcutName("Go to All")]
		public DbgShortKeys GoToAll = new(KeyModifiers.Control, Key.OemComma);

		[ShortcutName("Zoom In")]
		public DbgShortKeys ZoomIn = new(KeyModifiers.Control, Key.OemPlus);
		[ShortcutName("Zoom Out")]
		public DbgShortKeys ZoomOut = new(KeyModifiers.Control, Key.OemMinus);

		[ShortcutName("Save as PNG")]
		public DbgShortKeys SaveAsPng = new(KeyModifiers.Control, Key.S);

		[ShortcutName("Edit in Memory Viewer")]
		public DbgShortKeys CodeWindow_EditInMemoryViewer = new(Key.F1);
		[ShortcutName("View in disassembly")]
		public DbgShortKeys MemoryViewer_ViewInDisassembly = new();

		[ShortcutName("Open Assembler")]
		public DbgShortKeys OpenAssembler = new(KeyModifiers.Control, Key.U);
		[ShortcutName("Open Debugger")]
		public DbgShortKeys OpenDebugger = new(KeyModifiers.Control, Key.D);
		[ShortcutName("Open SPC Debugger")]
		public DbgShortKeys OpenSpcDebugger = new(KeyModifiers.Control, Key.F);
		[ShortcutName("Open SA-1 Debugger")]
		public DbgShortKeys OpenSa1Debugger = new();
		[ShortcutName("Open GSU Debugger")]
		public DbgShortKeys OpenGsuDebugger = new();
		[ShortcutName("Open DSP Debugger")]
		public DbgShortKeys OpenNecDspDebugger = new();
		[ShortcutName("Open CX4 Debugger")]
		public DbgShortKeys OpenCx4Debugger = new();
		[ShortcutName("Open Game Boy Debugger")]
		public DbgShortKeys OpenGameboyDebugger = new();
		[ShortcutName("Open Event Viewer")]
		public DbgShortKeys OpenEventViewer = new(KeyModifiers.Control, Key.E);
		[ShortcutName("Open Memory Tools")]
		public DbgShortKeys OpenMemoryTools = new(KeyModifiers.Control, Key.M);
		[ShortcutName("Open Performance Profiler")]
		public DbgShortKeys OpenProfiler = new(KeyModifiers.Control, Key.Y);
		[ShortcutName("Open Script Window")]
		public DbgShortKeys OpenScriptWindow = new(KeyModifiers.Control, Key.N);
		[ShortcutName("Open Trace Logger")]
		public DbgShortKeys OpenTraceLogger = new(KeyModifiers.Control, Key.J);
		[ShortcutName("Open Register Viewer")]
		public DbgShortKeys OpenRegisterViewer = new(KeyModifiers.Control, Key.K);
		[ShortcutName("Open Debug Log")]
		public DbgShortKeys OpenDebugLog = new(KeyModifiers.Control, Key.B);

		[ShortcutName("Open Tilemap Viewer")]
		public DbgShortKeys OpenTilemapViewer = new(KeyModifiers.Control, Key.D1);
		[ShortcutName("Open Tile Viewer")]
		public DbgShortKeys OpenTileViewer = new(KeyModifiers.Control, Key.D2);
		[ShortcutName("Open Sprite Viewer")]
		public DbgShortKeys OpenSpriteViewer = new(KeyModifiers.Control, Key.D3);
		[ShortcutName("Open Palette Viewer")]
		public DbgShortKeys OpenPaletteViewer = new(KeyModifiers.Control, Key.D4);

		//Debugger window
		[ShortcutName("Reset")]
		public DbgShortKeys Reset = new(KeyModifiers.Control, Key.R);
		[ShortcutName("Power Cycle")]
		public DbgShortKeys PowerCycle = new(KeyModifiers.Control, Key.T);
		[ShortcutName("Reload ROM")]
		public DbgShortKeys ReloadRom = new();

		[ShortcutName("Continue")]
		public DbgShortKeys Continue = new(Key.F5);
		[ShortcutName("Break")]
		public DbgShortKeys Break = new(KeyModifiers.Control | KeyModifiers.Alt, Key.Cancel);
		[ShortcutName("Toggle Break/Continue")]
		public DbgShortKeys ToggleBreakContinue = new(Key.Escape);
		[ShortcutName("Step Into")]
		public DbgShortKeys StepInto = new(Key.F11);
		[ShortcutName("Step Over")]
		public DbgShortKeys StepOver = new(Key.F10);
		[ShortcutName("Step Out")]
		public DbgShortKeys StepOut = new(KeyModifiers.Shift, Key.F11);
		[ShortcutName("Step Back")]
		public DbgShortKeys StepBack = new(KeyModifiers.Shift, Key.F10);

		[ShortcutName("Run one CPU Cycle")]
		public DbgShortKeys RunCpuCycle = new();
		[ShortcutName("Run one PPU Cycle")]
		public DbgShortKeys RunPpuCycle = new(Key.F6);
		[ShortcutName("Run one scanline")]
		public DbgShortKeys RunPpuScanline = new(Key.F7);
		[ShortcutName("Run one frame")]
		public DbgShortKeys RunPpuFrame = new(Key.F8);

		[ShortcutName("Break In...")]
		public DbgShortKeys BreakIn = new(KeyModifiers.Control, Key.B);
		[ShortcutName("Break On...")]
		public DbgShortKeys BreakOn = new(KeyModifiers.Alt, Key.B);

		[ShortcutName("Find Occurrences")]
		public DbgShortKeys FindOccurrences = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F);
		[ShortcutName("Go To Program Counter")]
		public DbgShortKeys GoToProgramCounter = new(KeyModifiers.Alt, Key.Multiply);

		[ShortcutName("Toggle Verified Data Display")]
		public DbgShortKeys ToggleVerifiedData = new(KeyModifiers.Alt, Key.D1);
		[ShortcutName("Toggle Unidentified Code/Data Display")]
		public DbgShortKeys ToggleUnidentifiedCodeData = new(KeyModifiers.Alt, Key.D2);

		[ShortcutName("Code Window: Set Next Statement")]
		public DbgShortKeys CodeWindow_SetNextStatement = new(KeyModifiers.Control | KeyModifiers.Shift, Key.F10);
		[ShortcutName("Code Window: Edit Subroutine")]
		public DbgShortKeys CodeWindow_EditSubroutine = new(Key.F4);
		[ShortcutName("Code Window: Edit Selected Code")]
		public DbgShortKeys CodeWindow_EditSelectedCode = new();
		[ShortcutName("Code Window: Edit Source File (Source View)")]
		public DbgShortKeys CodeWindow_EditSourceFile = new(Key.F4);
		[ShortcutName("Code Window: Edit Label")]
		public DbgShortKeys CodeWindow_EditLabel = new(Key.F2);
		[ShortcutName("Code Window: Navigate Back")]
		public DbgShortKeys CodeWindow_NavigateBack = new(KeyModifiers.Alt, Key.Left);
		[ShortcutName("Code Window: Navigate Forward")]
		public DbgShortKeys CodeWindow_NavigateForward = new(KeyModifiers.Alt, Key.Right);
		[ShortcutName("Code Window: Toggle Breakpoint")]
		public DbgShortKeys CodeWindow_ToggleBreakpoint = new(Key.F9);
		[ShortcutName("Code Window: Disable/Enable Breakpoint")]
		public DbgShortKeys CodeWindow_DisableEnableBreakpoint = new(KeyModifiers.Control, Key.F9);
		[ShortcutName("Code Window: Switch View (Disassembly / Source View)")]
		public DbgShortKeys CodeWindow_SwitchView = new(KeyModifiers.Control, Key.Q);

		[ShortcutName("Function List: Edit Label")]
		public DbgShortKeys FunctionList_EditLabel = new(Key.F2);
		[ShortcutName("Function List: Add Breakpoint")]
		public DbgShortKeys FunctionList_AddBreakpoint = new();
		[ShortcutName("Function List: Find Occurrences")]
		public DbgShortKeys FunctionList_FindOccurrences = new();

		[ShortcutName("Label List: Add Label")]
		public DbgShortKeys LabelList_Add = new(Key.Insert);
		[ShortcutName("Label List: Edit Label")]
		public DbgShortKeys LabelList_Edit = new(Key.F2);
		[ShortcutName("Label List: Delete Label")]
		public DbgShortKeys LabelList_Delete = new(Key.Delete);
		[ShortcutName("Label List: Add Breakpoint")]
		public DbgShortKeys LabelList_AddBreakpoint = new();
		[ShortcutName("Label List: Add to Watch")]
		public DbgShortKeys LabelList_AddToWatch = new();
		[ShortcutName("Label List: Find Occurrences")]
		public DbgShortKeys LabelList_FindOccurrences = new();
		[ShortcutName("Label List: View in CPU Memory")]
		public DbgShortKeys LabelList_ViewInCpuMemory = new();
		[ShortcutName("Label List: View in [memory type]")]
		public DbgShortKeys LabelList_ViewInMemoryType = new();

		[ShortcutName("Breakpoint List: Add Breakpoint")]
		public DbgShortKeys BreakpointList_Add = new(Key.Insert);
		[ShortcutName("Breakpoint List: Edit Breakpoint")]
		public DbgShortKeys BreakpointList_Edit = new(Key.F2);
		[ShortcutName("Breakpoint List: Go To Location")]
		public DbgShortKeys BreakpointList_GoToLocation = new();
		[ShortcutName("Breakpoint List: Delete Breakpoint")]
		public DbgShortKeys BreakpointList_Delete = new(Key.Delete);

		[ShortcutName("Watch List: Delete")]
		public DbgShortKeys WatchList_Delete = new(Key.Delete);
		[ShortcutName("Watch List: Move Up")]
		public DbgShortKeys WatchList_MoveUp = new(KeyModifiers.Control, Key.Up);
		[ShortcutName("Watch List: Move Down")]
		public DbgShortKeys WatchList_MoveDown = new(KeyModifiers.Control, Key.Down);

		[ShortcutName("Save Rom")]
		public DbgShortKeys SaveRom = new(KeyModifiers.Control, Key.S);
		[ShortcutName("Save Rom As...")]
		public DbgShortKeys SaveRomAs = new();
		[ShortcutName("Save edits as IPS patch...")]
		public DbgShortKeys SaveEditAsIps = new();
		[ShortcutName("Revert PRG/CHR changes")]
		public DbgShortKeys RevertPrgChrChanges = new();

		//Memory Tools
		[ShortcutName("Freeze")]
		public DbgShortKeys MemoryViewer_Freeze = new(KeyModifiers.Control, Key.Q);
		[ShortcutName("Unfreeze")]
		public DbgShortKeys MemoryViewer_Unfreeze = new(KeyModifiers.Control, Key.W);
		[ShortcutName("Add to Watch")]
		public DbgShortKeys MemoryViewer_AddToWatch = new();
		[ShortcutName("Edit Breakpoint")]
		public DbgShortKeys MemoryViewer_EditBreakpoint = new();
		[ShortcutName("Edit Label")]
		public DbgShortKeys MemoryViewer_EditLabel = new();
		[ShortcutName("Import")]
		public DbgShortKeys MemoryViewer_Import = new(KeyModifiers.Control, Key.O);
		[ShortcutName("Export")]
		public DbgShortKeys MemoryViewer_Export = new(KeyModifiers.Control, Key.S);
		[ShortcutName("View in CPU/PPU Memory")]
		public DbgShortKeys MemoryViewer_ViewInCpuMemory = new();
		[ShortcutName("View in [memory type]")]
		public DbgShortKeys MemoryViewer_ViewInMemoryType = new();

		//Script Window
		[ShortcutName("Open Script")]
		public DbgShortKeys ScriptWindow_OpenScript = new(KeyModifiers.Control, Key.N);
		[ShortcutName("Save Script")]
		public DbgShortKeys ScriptWindow_SaveScript = new(KeyModifiers.Control, Key.S);
		[ShortcutName("Run Script")]
		public DbgShortKeys ScriptWindow_RunScript = new(Key.F5);
		[ShortcutName("Stop Script")]
		public DbgShortKeys ScriptWindow_StopScript = new(Key.Escape);
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
			return ShortcutKey != Key.None ? new KeyGesture(ShortcutKey, Modifiers).ToString() : "";
		}
	}

	public class ShortcutNameAttribute : Attribute
	{
		public string Name { get; private set; }

		public ShortcutNameAttribute(string name)
		{
			this.Name = name;
		}
	}
}
