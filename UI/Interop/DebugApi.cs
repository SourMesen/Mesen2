using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Avalonia;
using Mesen.Config;
using Mesen.Debugger;
using Mesen.Utilities;

namespace Mesen.Interop
{
	public class DebugApi
	{
		private const string DllPath = EmuApi.DllName;
		[DllImport(DllPath)] public static extern void InitializeDebugger();
		[DllImport(DllPath)] public static extern void ReleaseDebugger();

		[DllImport(DllPath)] public static extern void ResumeExecution();
		[DllImport(DllPath)] public static extern void Step(CpuType cpuType, Int32 instructionCount, StepType type = StepType.Step);

		[DllImport(DllPath)] public static extern void StartLogTraceToFile([MarshalAs(UnmanagedType.LPUTF8Str)] string filename);
		[DllImport(DllPath)] public static extern void StopLogTraceToFile();

		[DllImport(DllPath)] public static extern void SetTraceOptions(CpuType cpuType, InteropTraceLoggerOptions options);

		public const int TraceLogBufferSize = 30000;
		[DllImport(DllPath)] public static extern void ClearExecutionTrace();
		[DllImport(DllPath, EntryPoint = "GetExecutionTrace")] private static extern UInt32 GetExecutionTraceWrapper(IntPtr output, UInt32 startOffset, UInt32 maxRowCount);
		public static unsafe TraceRow[] GetExecutionTrace(UInt32 startOffset, UInt32 maxRowCount)
		{
			TraceRow[] rows = new TraceRow[maxRowCount];

			UInt32 rowCount;
			fixed(TraceRow* ptr = rows) {
				rowCount = DebugApi.GetExecutionTraceWrapper((IntPtr)ptr, startOffset, maxRowCount);
			}

			Array.Resize(ref rows, (int)rowCount);

			return rows;
		}

		public static UInt32 GetExecutionTraceSize()
		{
			return DebugApi.GetExecutionTraceWrapper(IntPtr.Zero, 0, DebugApi.TraceLogBufferSize);
		}

		[DllImport(DllPath, EntryPoint = "GetDebuggerLog")] private static extern void GetDebuggerLogWrapper(IntPtr outLog, Int32 maxLength);
		public static string GetLog() { return Utf8Utilities.CallStringApi(GetDebuggerLogWrapper, 100000); }

		[DllImport(DllPath)] private static extern UInt32 GetDisassemblyOutput(CpuType type, UInt32 address, [In, Out] InteropCodeLineData[] lineData, UInt32 rowCount);
		public static CodeLineData[] GetDisassemblyOutput(CpuType type, UInt32 address, UInt32 rowCount)
		{
			InteropCodeLineData[] rows = new InteropCodeLineData[rowCount];
			for(int i = 0; i < rowCount; i++) {
				rows[i].Comment = new byte[1000];
				rows[i].Text = new byte[1000];
				rows[i].ByteCode = new byte[8];
			}

			UInt32 resultCount = DebugApi.GetDisassemblyOutput(type, address, rows, rowCount);

			CodeLineData[] result = new CodeLineData[resultCount];
			for(int i = 0; i < resultCount; i++) {
				result[i] = new CodeLineData(rows[i]);
			}
			return result;
		}

		[DllImport(DllPath)] public static extern int GetDisassemblyRowAddress(CpuType type, UInt32 address, int rowOffset);
		[DllImport(DllPath)] public static extern int SearchDisassembly(CpuType type, [MarshalAs(UnmanagedType.LPUTF8Str)] string searchString, int startAddress, DisassemblySearchOptions options);
		
		[DllImport(DllPath)] private static extern UInt32 FindOccurrences(CpuType type, [MarshalAs(UnmanagedType.LPUTF8Str)] string searchString, DisassemblySearchOptions options, [In, Out] InteropCodeLineData[] lineData, UInt32 maxResultCount);
		public static CodeLineData[] FindOccurrences(CpuType type, string searchString, DisassemblySearchOptions options)
		{
			UInt32 maxResultCount = 500;
			InteropCodeLineData[] rows = new InteropCodeLineData[maxResultCount];
			for(int i = 0; i < maxResultCount; i++) {
				rows[i].Comment = new byte[1000];
				rows[i].Text = new byte[1000];
				rows[i].ByteCode = new byte[8];
			}

			UInt32 resultCount = DebugApi.FindOccurrences(type, searchString, options, rows, maxResultCount);

			CodeLineData[] result = new CodeLineData[resultCount];
			for(int i = 0; i < resultCount; i++) {
				result[i] = new CodeLineData(rows[i]);
			}
			return result;
		}

		[DllImport(DllPath)] private static extern void GetCpuState(IntPtr state, CpuType cpuType);
		public unsafe static T GetCpuState<[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicConstructors | DynamicallyAccessedMemberTypes.NonPublicConstructors)] T>(CpuType cpuType) where T : struct, BaseState
		{
			byte* ptr = stackalloc byte[Marshal.SizeOf<T>()];
			DebugApi.GetCpuState((IntPtr)ptr, cpuType);
			return Marshal.PtrToStructure<T>((IntPtr)ptr);
		}

		[DllImport(DllPath)] private static extern void GetPpuState(IntPtr state, CpuType cpuType);
		public unsafe static T GetPpuState<[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicConstructors | DynamicallyAccessedMemberTypes.NonPublicConstructors)] T>(CpuType cpuType) where T : struct, BaseState
		{
			byte* ptr = stackalloc byte[Marshal.SizeOf<T>()];
			DebugApi.GetPpuState((IntPtr)ptr, cpuType);
			return Marshal.PtrToStructure<T>((IntPtr)ptr);
		}

		public static BaseState GetPpuState(CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => GetPpuState<SnesPpuState>(cpuType),
				CpuType.Nes => GetPpuState<NesPpuState>(cpuType),
				CpuType.Gameboy => GetPpuState<GbPpuState>(cpuType),
				CpuType.Pce => GetPpuState<PceVideoState>(cpuType),
				CpuType.Sms => GetPpuState<SmsVdpState>(cpuType),
				CpuType.Gba => GetPpuState<GbaPpuState>(cpuType),
				CpuType.Ws => GetPpuState<WsPpuState>(cpuType),
				_ => throw new Exception("Unsupported cpu type")
			};
		}

		[DllImport(DllPath)] private static extern void GetPpuToolsState(CpuType cpuType, IntPtr state);
		public unsafe static T GetPpuToolsState<[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicConstructors | DynamicallyAccessedMemberTypes.NonPublicConstructors)] T>(CpuType cpuType) where T : struct, BaseState
		{
			byte* ptr = stackalloc byte[Marshal.SizeOf<T>()];
			DebugApi.GetPpuToolsState(cpuType, (IntPtr)ptr);
			return Marshal.PtrToStructure<T>((IntPtr)ptr);
		}

		public static BaseState GetPpuToolsState(CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => GetPpuToolsState<SnesPpuToolsState>(cpuType),
				CpuType.Nes => GetPpuToolsState<NesPpuToolsState>(cpuType),
				CpuType.Gameboy => GetPpuToolsState<EmptyPpuToolsState>(cpuType),
				CpuType.Pce => GetPpuToolsState<EmptyPpuToolsState>(cpuType),
				CpuType.Sms => GetPpuToolsState<EmptyPpuToolsState>(cpuType),
				CpuType.Gba => GetPpuToolsState<EmptyPpuToolsState>(cpuType),
				CpuType.Ws => GetPpuToolsState<EmptyPpuToolsState>(cpuType),
				_ => throw new Exception("Unsupported cpu type")
			};
		}

		[DllImport(DllPath)] private static extern void SetPpuState(IntPtr state, CpuType cpuType);
		public unsafe static void SetPpuState<T>(T state, CpuType cpuType) where T : BaseState
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidPpuState(ref state, cpuType));

			byte* stateBuffer = stackalloc byte[Marshal.SizeOf<T>()];
			Marshal.StructureToPtr(state, (IntPtr)stateBuffer, false);
			DebugApi.SetPpuState((IntPtr)stateBuffer, cpuType);
		}

		[DllImport(DllPath)] private static extern void SetCpuState(IntPtr state, CpuType cpuType);
		public unsafe static void SetCpuState<T>(T state, CpuType cpuType) where T : BaseState
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidCpuState(ref state, cpuType));

			byte* stateBuffer = stackalloc byte[Marshal.SizeOf<T>()];
			Marshal.StructureToPtr(state, (IntPtr)stateBuffer, false);
			DebugApi.SetCpuState((IntPtr)stateBuffer, cpuType);
		}

		[DllImport(DllPath)] private static extern void GetConsoleState(IntPtr state, ConsoleType consoleType);
		public unsafe static T GetConsoleState<[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicConstructors | DynamicallyAccessedMemberTypes.NonPublicConstructors)] T>(ConsoleType consoleType) where T : struct, BaseState
		{
			byte* ptr = stackalloc byte[Marshal.SizeOf<T>()];
			DebugApi.GetConsoleState((IntPtr)ptr, consoleType);
			return Marshal.PtrToStructure<T>((IntPtr)ptr);
		}

		[DllImport(DllPath)] public static extern void SetProgramCounter(CpuType cpuType, UInt32 address);
		[DllImport(DllPath)] public static extern UInt32 GetProgramCounter(CpuType cpuType, [MarshalAs(UnmanagedType.I1)] bool getInstPc);

		[DllImport(DllPath)] public static extern Int32 LoadScript([MarshalAs(UnmanagedType.LPUTF8Str)]string name, [MarshalAs(UnmanagedType.LPUTF8Str)]string path, [MarshalAs(UnmanagedType.LPUTF8Str)] string content, Int32 scriptId = -1);
		[DllImport(DllPath)] public static extern void RemoveScript(Int32 scriptId);

		[DllImport(DllPath, EntryPoint = "GetScriptLog")] private static extern void GetScriptLogWrapper(Int32 scriptId, IntPtr outScriptLog, Int32 maxLength);
		public unsafe static string GetScriptLog(Int32 scriptId)
		{
			byte[] outScriptLog = new byte[100000];
			fixed(byte* ptr = outScriptLog) {
				DebugApi.GetScriptLogWrapper(scriptId, (IntPtr)ptr, outScriptLog.Length);
				return Utf8Utilities.PtrToStringUtf8((IntPtr)ptr);
			}
		}

		[DllImport(DllPath)] public static extern Int64 EvaluateExpression([MarshalAs(UnmanagedType.LPUTF8Str)] string expression, CpuType cpuType, out EvalResultType resultType, [MarshalAs(UnmanagedType.I1)] bool useCache);

		[DllImport(DllPath)] public static extern DebuggerFeatures GetDebuggerFeatures(CpuType type);
		[DllImport(DllPath)] public static extern CpuInstructionProgress GetInstructionProgress(CpuType type);

		[DllImport(DllPath)] public static extern Int32 GetMemorySize(MemoryType type);
		[DllImport(DllPath)] public static extern Byte GetMemoryValue(MemoryType type, UInt32 address);
		[DllImport(DllPath)] public static extern void SetMemoryValue(MemoryType type, UInt32 address, byte value);
		[DllImport(DllPath)] public static extern void SetMemoryValues(MemoryType type, UInt32 address, [In] byte[] data, Int32 length);
		[DllImport(DllPath)] public static extern void SetMemoryState(MemoryType type, [In] byte[] buffer, Int32 length);
		
		[DllImport(DllPath)][return: MarshalAs(UnmanagedType.I1)] public static extern bool HasUndoHistory();
		[DllImport(DllPath)] public static extern void PerformUndo();

		[DllImport(DllPath)] public static extern void UpdateFrozenAddresses(CpuType cpuType, UInt32 start, UInt32 end, [MarshalAs(UnmanagedType.I1)] bool freeze);
		[DllImport(DllPath)] private static extern void GetFrozenState(CpuType type, UInt32 start, UInt32 end, [In, Out] byte[] outState);
		public static byte[] GetFrozenState(CpuType cpuType, UInt32 start, UInt32 end)
		{
			byte[] outState = new byte[end - start + 1];
			DebugApi.GetFrozenState(cpuType, start, end, outState);
			return outState;
		}

		[DllImport(DllPath)] public static extern AddressInfo GetAbsoluteAddress(AddressInfo relAddress);
		[DllImport(DllPath)] public static extern AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType);

		[DllImport(DllPath)] public static extern void SetLabel(uint address, MemoryType memType, [MarshalAs(UnmanagedType.LPUTF8Str)] string label, [MarshalAs(UnmanagedType.LPUTF8Str)] string comment);
		[DllImport(DllPath)] public static extern void ClearLabels();

		[DllImport(DllPath)] public static extern void SetBreakpoints([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] InteropBreakpoint[] breakpoints, UInt32 length);
		
		[DllImport(DllPath)] public static extern void SetInputOverrides(UInt32 index, DebugControllerState state);
		[DllImport(DllPath)] private static extern void GetAvailableInputOverrides([In, Out] byte[] availableIndexes);
		
		public static List<int> GetAvailableInputOverrides()
		{
			byte[] availableIndexes = new byte[8];
			GetAvailableInputOverrides(availableIndexes);
			
			List<int> indexes = new List<int>();
			for(int i = 0; i < 8; i++) {
				if(availableIndexes[i] != 0) {
					indexes.Add(i);
				}
			}
			return indexes;
		}

		[DllImport(DllPath, EntryPoint = "GetRomHeader")] private static extern void GetRomHeaderWrapper([In, Out] byte[] headerData, ref UInt32 size);
		public static byte[] GetRomHeader()
		{
			UInt32 size = 0x1000;
			byte[] headerData = new byte[size];
			DebugApi.GetRomHeaderWrapper(headerData, ref size);
			Array.Resize(ref headerData, (Int32)size);
			return headerData;
		}

		[DllImport(DllPath)][return: MarshalAs(UnmanagedType.I1)] public static extern bool SaveRomToDisk([MarshalAs(UnmanagedType.LPUTF8Str)] string filename, [MarshalAs(UnmanagedType.I1)] bool saveAsIps, CdlStripOption cdlStripOption);

		[DllImport(DllPath, EntryPoint = "GetMemoryValues")] private static extern void GetMemoryValuesWrapper(MemoryType type, UInt32 start, UInt32 end, [In, Out] byte[] buffer);
		public static byte[] GetMemoryValues(MemoryType type, UInt32 start, UInt32 end)
		{
			byte[] buffer = new byte[end - start + 1];
			DebugApi.GetMemoryValuesWrapper(type, start, end, buffer);
			return buffer;
		}

		public static void GetMemoryValues(MemoryType type, UInt32 start, UInt32 end, ref byte[] dst)
		{
			Array.Resize(ref dst, (int)(end - start + 1));
			DebugApi.GetMemoryValuesWrapper(type, start, end, dst);
		}

		[DllImport(DllPath, EntryPoint = "GetMemoryState")] private static extern void GetMemoryStateWrapper(MemoryType type, [In, Out] byte[] buffer);
		public static byte[] GetMemoryState(MemoryType type)
		{
			byte[] buffer = new byte[DebugApi.GetMemorySize(type)];
			DebugApi.GetMemoryStateWrapper(type, buffer);
			return buffer;
		}

		public static void GetMemoryState(MemoryType type, ref byte[] dst)
		{
			int length = DebugApi.GetMemorySize(type);
			Array.Resize(ref dst, length);
			DebugApi.GetMemoryStateWrapper(type, dst);
		}

		[DllImport(DllPath)] private static extern DebugTilemapInfo GetTilemap(CpuType cpuType, InteropGetTilemapOptions options, IntPtr state, IntPtr ppuToolsState, byte[] vram, UInt32[] palette, IntPtr outputBuffer);
		public unsafe static DebugTilemapInfo GetTilemap(CpuType cpuType, GetTilemapOptions options, BaseState state, BaseState ppuToolsState, byte[] vram, UInt32[] palette, IntPtr outputBuffer)
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidPpuState(ref state, cpuType));

			fixed(byte* compareVramPtr = options.CompareVram) {
				fixed(AddressCounters* accessCounters = options.AccessCounters) {
					byte* stateBuffer = stackalloc byte[GetStateSize(state)];
					Marshal.StructureToPtr(state, (IntPtr)stateBuffer, false);

					byte* ppuToolsStateBuffer = stackalloc byte[GetStateSize(ppuToolsState)];
					Marshal.StructureToPtr(ppuToolsState, (IntPtr)ppuToolsStateBuffer, false);

					InteropGetTilemapOptions interopOptions = options.ToInterop();
					interopOptions.CompareVram = (IntPtr)compareVramPtr;
					interopOptions.AccessCounters = (IntPtr)accessCounters;
					return DebugApi.GetTilemap(cpuType, interopOptions, (IntPtr)stateBuffer, (IntPtr)ppuToolsStateBuffer, vram, palette, outputBuffer);
				}
			}
		}

		[DllImport(DllPath)] private static extern FrameInfo GetTilemapSize(CpuType cpuType, InteropGetTilemapOptions options, IntPtr state);
		public unsafe static FrameInfo GetTilemapSize(CpuType cpuType, GetTilemapOptions options, BaseState state)
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidPpuState(ref state, cpuType));

			byte* ptr = stackalloc byte[GetStateSize(state)];
			Marshal.StructureToPtr(state, (IntPtr)ptr, false);

			return DebugApi.GetTilemapSize(cpuType, options.ToInterop(), (IntPtr)ptr);
		}

		[DllImport(DllPath)] private static extern DebugTilemapTileInfo GetTilemapTileInfo(UInt32 x, UInt32 y, CpuType cpuType, InteropGetTilemapOptions options, byte[] vram, IntPtr state, IntPtr ppuToolsState);
		public unsafe static DebugTilemapTileInfo? GetTilemapTileInfo(UInt32 x, UInt32 y, CpuType cpuType, GetTilemapOptions options, byte[] vram, BaseState state, BaseState ppuToolsState)
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidPpuState(ref state, cpuType));

			byte* ptr = stackalloc byte[GetStateSize(state)];
			Marshal.StructureToPtr(state, (IntPtr)ptr, false);

			byte* ppuToolsStateBuffer = stackalloc byte[GetStateSize(ppuToolsState)];
			Marshal.StructureToPtr(ppuToolsState, (IntPtr)ppuToolsStateBuffer, false);

			DebugTilemapTileInfo info = DebugApi.GetTilemapTileInfo(x, y, cpuType, options.ToInterop(), vram, (IntPtr)ptr, (IntPtr)ppuToolsStateBuffer);
			return info.Row >= 0 ? info : null;
		}

		[DllImport(DllPath)] public static extern void GetTileView(CpuType cpuType, GetTileViewOptions options, byte[] source, int srcSize, UInt32[] palette, IntPtr buffer);

		[DllImport(DllPath)] private static extern DebugSpritePreviewInfo GetSpritePreviewInfo(CpuType cpuType, GetSpritePreviewOptions options, IntPtr state, IntPtr ppuToolsState);
		public unsafe static DebugSpritePreviewInfo GetSpritePreviewInfo(CpuType cpuType, GetSpritePreviewOptions options, BaseState state, BaseState ppuToolsState)
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidPpuState(ref state, cpuType));

			byte* ptr = stackalloc byte[GetStateSize(state)];
			Marshal.StructureToPtr(state, (IntPtr)ptr, false);

			byte* ppuToolsStateBuffer = stackalloc byte[GetStateSize(ppuToolsState)];
			Marshal.StructureToPtr(ppuToolsState, (IntPtr)ppuToolsStateBuffer, false);

			return DebugApi.GetSpritePreviewInfo(cpuType, options, (IntPtr)ptr, (IntPtr)ppuToolsStateBuffer);
		}

		[DllImport(DllPath)] private static extern void GetSpriteList(CpuType cpuType, GetSpritePreviewOptions options, IntPtr state, IntPtr ppuToolsState, byte[] vram, byte[]? spriteRam, UInt32[] palette, IntPtr sprites, IntPtr spritePreviews, IntPtr screenPreview);
		public unsafe static void GetSpriteList(ref DebugSpriteInfo[] result, ref UInt32[] spritePreviews, CpuType cpuType, GetSpritePreviewOptions options, BaseState state, BaseState ppuToolsState, byte[] vram, byte[] spriteRam, UInt32[] palette, IntPtr screenPreview)
		{
			Debug.Assert(state.GetType().IsValueType);
			Debug.Assert(IsValidPpuState(ref state, cpuType));

			byte* statePtr = stackalloc byte[GetStateSize(state)];
			Marshal.StructureToPtr(state, (IntPtr)statePtr, false);

			byte* ppuToolsStateBuffer = stackalloc byte[GetStateSize(ppuToolsState)];
			Marshal.StructureToPtr(ppuToolsState, (IntPtr)ppuToolsStateBuffer, false);

			int count = (int)GetSpritePreviewInfo(cpuType, options, (IntPtr)statePtr, (IntPtr)ppuToolsStateBuffer).SpriteCount;
			if(count != result.Length) {
				Array.Resize(ref result, count);
			}

			if(count*128*128 != spritePreviews.Length) {
				Array.Resize(ref spritePreviews, count*128*128);
			}

			fixed(DebugSpriteInfo* spritesPtr = result) {
				fixed(UInt32* spritePreviewsPtr = spritePreviews) {
					DebugApi.GetSpriteList(cpuType, options, (IntPtr)statePtr, (IntPtr)ppuToolsStateBuffer, vram, spriteRam.Length > 0 ? spriteRam : null, palette, (IntPtr)spritesPtr, (IntPtr)spritePreviewsPtr, screenPreview);
				}
			}
		}

		[DllImport(DllPath)] public static extern DebugPaletteInfo GetPaletteInfo(CpuType cpuType, GetPaletteInfoOptions options = new());
		[DllImport(DllPath)] public static extern void SetPaletteColor(CpuType cpuType, int colorIndex, UInt32 color);
		
		[DllImport(DllPath)] public static extern int GetTilePixel(AddressInfo tileAddress, TileFormat format, int x, int y);
		[DllImport(DllPath)] public static extern void SetTilePixel(AddressInfo tileAddress, TileFormat format, int x, int y, int color);

		[DllImport(DllPath)] public static extern void SetViewerUpdateTiming(Int32 viewerId, Int32 scanline, Int32 cycle, CpuType cpuType);
		[DllImport(DllPath)] public static extern void RemoveViewerId(int viewerId, CpuType cpuType);

		[DllImport(DllPath)] private static extern UInt32 GetDebugEventCount(CpuType cpuType);
		[DllImport(DllPath, EntryPoint = "GetDebugEvents")] private static extern void GetDebugEventsWrapper(CpuType cpuType, [In, Out] DebugEventInfo[] eventArray, ref UInt32 maxEventCount);
		public static DebugEventInfo[] GetDebugEvents(CpuType cpuType)
		{
			UInt32 maxEventCount = GetDebugEventCount(cpuType);
			DebugEventInfo[] debugEvents = new DebugEventInfo[maxEventCount];

			DebugApi.GetDebugEventsWrapper(cpuType, debugEvents, ref maxEventCount);
			if(maxEventCount < debugEvents.Length) {
				//Remove the excess from the array if needed
				Array.Resize(ref debugEvents, (int)maxEventCount);
			}

			return debugEvents;
		}

		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropSnesEventViewerConfig config);
		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropNesEventViewerConfig config);
		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropGbEventViewerConfig config);
		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropGbaEventViewerConfig config);
		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropPceEventViewerConfig config);
		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropSmsEventViewerConfig config);
		[DllImport(DllPath)] public static extern void SetEventViewerConfig(CpuType cpuType, InteropWsEventViewerConfig config);

		[DllImport(DllPath, EntryPoint = "GetEventViewerEvent")] private static extern DebugEventInfo GetEventViewerEventWrapper(CpuType cpuType, UInt16 scanline, UInt16 cycle);
		public static DebugEventInfo? GetEventViewerEvent(CpuType cpuType, UInt16 scanline, UInt16 cycle)
		{
			DebugEventInfo evt = DebugApi.GetEventViewerEventWrapper(cpuType, scanline, cycle);
			if(evt.ProgramCounter != UInt32.MaxValue) {
				return evt;
			}
			return null;
		}

		[DllImport(DllPath)] public static extern UInt32 TakeEventSnapshot(CpuType cpuType, [MarshalAs(UnmanagedType.I1)] bool forAutoRefresh);

		[DllImport(DllPath)] public static extern FrameInfo GetEventViewerDisplaySize(CpuType cpuType);
		[DllImport(DllPath)] public static extern void GetEventViewerOutput(CpuType cpuType, IntPtr buffer, UInt32 bufferSize);

		[DllImport(DllPath, EntryPoint = "GetCallstack")] private static extern void GetCallstackWrapper(CpuType type, [In, Out] StackFrameInfo[] callstackArray, ref UInt32 callstackSize);
		public static StackFrameInfo[] GetCallstack(CpuType type)
		{
			StackFrameInfo[] callstack = new StackFrameInfo[512];
			UInt32 callstackSize = 0;

			DebugApi.GetCallstackWrapper(type, callstack, ref callstackSize);
			Array.Resize(ref callstack, (int)callstackSize);

			return callstack;
		}

		[DllImport(DllPath)] public static extern void ResetProfiler(CpuType type);
		[DllImport(DllPath, EntryPoint = "GetProfilerData")] private static extern void GetProfilerDataWrapper(CpuType type, IntPtr profilerData, ref UInt32 functionCount);
		public static unsafe int GetProfilerData(CpuType type, ref ProfiledFunction[] profilerData)
		{
			UInt32 functionCount = 0;
			fixed(ProfiledFunction* ptr = profilerData) {
				DebugApi.GetProfilerDataWrapper(type, (IntPtr)ptr, ref functionCount);
			}
			return (int)functionCount;
		}

		[DllImport(DllPath, EntryPoint = "GetTokenList")] private static extern void GetTokenListWrapper(CpuType cpuType, IntPtr tokenListBuffer);
		public static unsafe string[] GetTokenList(CpuType type)
		{
			byte[] tokenListBuffer = new byte[1000];

			fixed(byte* ptr = tokenListBuffer) {
				DebugApi.GetTokenListWrapper(type, (IntPtr)ptr);
			}

			int index = Array.IndexOf<byte>(tokenListBuffer, 0);
			if(index >= 0) {
				return UTF8Encoding.UTF8.GetString(tokenListBuffer, 0, index).Split("|", StringSplitOptions.RemoveEmptyEntries);
			}

			return Array.Empty<string>();
		}

		[DllImport(DllPath)] public static extern void ResetMemoryAccessCounts();
		public static unsafe void GetMemoryAccessCounts(MemoryType type, ref AddressCounters[] counts)
		{
			int size = DebugApi.GetMemorySize(type);
			Array.Resize(ref counts, size);

			fixed(AddressCounters* ptr = counts) {
				DebugApi.GetMemoryAccessCountsWrapper(0, (uint)size, type, (IntPtr)ptr);
			}
		}

		public static AddressCounters[] GetMemoryAccessCounts(MemoryType type)
		{
			int size = DebugApi.GetMemorySize(type);
			AddressCounters[] counts = new AddressCounters[size];
			GetMemoryAccessCounts(type, ref counts);
			return counts;
		}

		[DllImport(DllPath, EntryPoint = "GetMemoryAccessCounts")] private static extern void GetMemoryAccessCountsWrapper(UInt32 offset, UInt32 length, MemoryType type, IntPtr counts);
		public static unsafe AddressCounters[] GetMemoryAccessCounts(UInt32 offset, UInt32 length, MemoryType type)
		{
			AddressCounters[] counts = new AddressCounters[length];

			fixed(AddressCounters* ptr = counts) {
				DebugApi.GetMemoryAccessCountsWrapper(offset, length, type, (IntPtr)ptr);
			}

			return counts;
		}

		[DllImport(DllPath, EntryPoint = "GetCdlData")] private static extern void GetCdlDataWrapper(UInt32 offset, UInt32 length, MemoryType memType, [In, Out] CdlFlags[] cdlData);

		public static CdlFlags[] GetCdlData(UInt32 offset, UInt32 length, MemoryType memType)
		{
			CdlFlags[] cdlData = new CdlFlags[length];
			DebugApi.GetCdlDataWrapper(offset, length, memType, cdlData);
			return cdlData;
		}

		public static CdlFlags[] GetCdlData(CpuType cpuType)
		{
			return DebugApi.GetCdlData(0, (uint)DebugApi.GetMemorySize(cpuType.GetPrgRomMemoryType()), cpuType.GetPrgRomMemoryType());
		}

		[DllImport(DllPath)] public static extern void ResetCdl(MemoryType memType);
		[DllImport(DllPath)] public static extern void SaveCdlFile(MemoryType memType, [MarshalAs(UnmanagedType.LPUTF8Str)] string cdlFile);
		[DllImport(DllPath)] public static extern void LoadCdlFile(MemoryType memType, [MarshalAs(UnmanagedType.LPUTF8Str)] string cdlFile);
		[DllImport(DllPath)] public static extern void SetCdlData(MemoryType memType, [In] byte[] cdlData, Int32 length);
		[DllImport(DllPath)] public static extern void MarkBytesAs(MemoryType memType, UInt32 start, UInt32 end, CdlFlags type);
		[DllImport(DllPath)] public static extern CdlStatistics GetCdlStatistics(MemoryType memType);

		[DllImport(DllPath)] private static extern UInt32 GetCdlFunctions(MemoryType memType, IntPtr functions, UInt32 maxSize);
		public unsafe static UInt32[] GetCdlFunctions(MemoryType memType)
		{
			UInt32[] functions = new UInt32[0x40000];
			UInt32 count;
			fixed(UInt32* functionPtr = functions) {
				count = DebugApi.GetCdlFunctions(memType, (IntPtr)functionPtr, (UInt32)functions.Length);
			}
			Array.Resize(ref functions, (int)count);
			return functions;
		}

		[DllImport(DllPath, EntryPoint = "AssembleCode")] private static extern UInt32 AssembleCodeWrapper(CpuType cpuType, [MarshalAs(UnmanagedType.LPUTF8Str)] string code, UInt32 startAddress, [In, Out] Int16[] assembledCodeBuffer);
		public static Int16[] AssembleCode(CpuType cpuType, string code, UInt32 startAddress)
		{
			code = code.Replace(Environment.NewLine, "\n");
			Int16[] assembledCode = new Int16[100000];
			UInt32 size = DebugApi.AssembleCodeWrapper(cpuType, code, startAddress, assembledCode);
			Array.Resize(ref assembledCode, (int)size);
			return assembledCode;
		}

		private static bool IsValidCpuState<T>(ref T state, CpuType cpuType) where T : BaseState
		{
			return cpuType switch {
				CpuType.Snes => state is SnesCpuState,
				CpuType.Spc => state is SpcState,
				CpuType.NecDsp => state is NecDspState,
				CpuType.Sa1 => state is SnesCpuState,
				CpuType.Gsu => state is GsuState,
				CpuType.Cx4 => state is Cx4State,
				CpuType.St018 => state is ArmV3CpuState,
				CpuType.Gameboy => state is GbCpuState,
				CpuType.Nes => state is NesCpuState,
				CpuType.Pce => state is PceCpuState,
				CpuType.Sms => state is SmsCpuState,
				CpuType.Gba => state is GbaCpuState,
				CpuType.Ws => state is WsCpuState,
				_ => false
			};
		}

		private static bool IsValidPpuState<T>(ref T state, CpuType cpuType) where T : BaseState
		{
			return cpuType.GetConsoleType() switch {
				ConsoleType.Snes => state is SnesPpuState,
				ConsoleType.Nes => state is NesPpuState,
				ConsoleType.Gameboy => state is GbPpuState,
				ConsoleType.PcEngine => state is PceVideoState,
				ConsoleType.Sms => state is SmsVdpState,
				ConsoleType.Gba => state is GbaPpuState,
				ConsoleType.Ws => state is WsPpuState,
				_ => false
			};
		}

		private static int GetStateSize(BaseState state)
		{
#pragma warning disable IL3050 // Calling members annotated with 'RequiresDynamicCodeAttribute' may break functionality when AOT compiling.
			return Marshal.SizeOf(state.GetType());
#pragma warning restore IL3050 // Calling members annotated with 'RequiresDynamicCodeAttribute' may break functionality when AOT compiling.
		}

	}

	public enum MemoryType
	{
		SnesMemory,
		SpcMemory,
		Sa1Memory,
		NecDspMemory,
		GsuMemory,
		Cx4Memory,
		St018Memory,
		GameboyMemory,
		NesMemory,
		NesPpuMemory,
		PceMemory,
		SmsMemory,
		GbaMemory,
		WsMemory,

		SnesPrgRom,
		SnesWorkRam,
		SnesSaveRam,
		SnesVideoRam,
		SnesSpriteRam,
		SnesCgRam,
		SnesRegister,
		SpcRam,
		SpcRom,
		SpcDspRegisters,
		DspProgramRom,
		DspDataRom,
		DspDataRam,
		Sa1InternalRam,
		GsuWorkRam,
		Cx4DataRam,
		BsxPsRam,
		BsxMemoryPack,
		St018PrgRom,
		St018DataRom,
		St018WorkRam,
		SufamiTurboFirmware,
		SufamiTurboSecondCart,
		SufamiTurboSecondCartRam,

		GbPrgRom,
		GbWorkRam,
		GbCartRam,
		GbHighRam,
		GbBootRom,
		GbVideoRam,
		GbSpriteRam,

		NesPrgRom,
		NesInternalRam,
		NesWorkRam,
		NesSaveRam,
		NesNametableRam,
		NesMapperRam,
		NesSpriteRam,
		NesSecondarySpriteRam,
		NesPaletteRam,
		NesChrRam,
		NesChrRom,

		PcePrgRom,
		PceWorkRam,
		PceSaveRam,
		PceCdromRam,
		PceCardRam,
		PceAdpcmRam,
		PceArcadeCardRam,
		PceVideoRam,
		PceVideoRamVdc2,
		PceSpriteRam,
		PceSpriteRamVdc2,
		PcePaletteRam,

		SmsPrgRom,
		SmsWorkRam,
		SmsCartRam,
		SmsBootRom,
		SmsVideoRam,
		SmsPaletteRam,
		SmsPort,

		GbaPrgRom,
		GbaBootRom,
		GbaSaveRam,
		GbaIntWorkRam,
		GbaExtWorkRam,
		GbaVideoRam,
		GbaSpriteRam,
		GbaPaletteRam,

		WsPrgRom,
		WsWorkRam,
		WsCartRam,
		WsCartEeprom,
		WsBootRom,
		WsInternalEeprom,
		WsPort,

		None,
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct AddressCounters
	{
		public UInt64 ReadStamp;
		public UInt64 WriteStamp;
		public UInt64 ExecStamp;
		public UInt32 ReadCounter;
		public UInt32 WriteCounter;
		public UInt32 ExecCounter;
	}

	public struct AddressInfo
	{
		public Int32 Address;
		public MemoryType Type;
	}

	public enum MemoryOperationType
	{
		Read = 0,
		Write = 1,
		ExecOpCode = 2,
		ExecOperand = 3,
		DmaRead = 4,
		DmaWrite = 5,
		DummyRead = 6,
		DummyWrite = 7,
		PpuRenderingRead = 8,
		Idle = 9
	}

	public struct MemoryOperationInfo
	{
		public UInt32 Address;
		public Int32 Value;
		public MemoryOperationType Type;
		public MemoryType MemType;
	}

	public enum DebugEventType
	{
		Register,
		Nmi,
		Irq,
		Breakpoint,
		BgColorChange,
		SpriteZeroHit,
		DmcDmaRead,
		DmaRead
	}

	public struct DmaChannelConfig
	{
		public UInt16 SrcAddress;
		public UInt16 TransferSize;
		public UInt16 HdmaTableAddress;
		public byte SrcBank;
		public byte DestAddress;

		[MarshalAs(UnmanagedType.I1)] public bool DmaActive;

		[MarshalAs(UnmanagedType.I1)] public bool InvertDirection;
		[MarshalAs(UnmanagedType.I1)] public bool Decrement;
		[MarshalAs(UnmanagedType.I1)] public bool FixedTransfer;
		[MarshalAs(UnmanagedType.I1)] public bool HdmaIndirectAddressing;

		public byte TransferMode;

		public byte HdmaBank;
		public byte HdmaLineCounterAndRepeat;

		[MarshalAs(UnmanagedType.I1)] public bool DoTransfer;
		[MarshalAs(UnmanagedType.I1)] public bool HdmaFinished;
		[MarshalAs(UnmanagedType.I1)] public bool UnusedControlFlag;

		public byte UnusedRegister;
	}

	public enum EventFlags
	{
		PreviousFrame = 1 << 0,
		RegFirstWrite = 1 << 1,
		RegSecondWrite = 1 << 2,
		HasTargetMemory = 1 << 3,
		SmsVdpPaletteWrite = 1 << 4,
		ReadWriteOp = 1 << 5,
	}

	public record struct DebugEventInfo
	{
		public MemoryOperationInfo Operation;
		public DebugEventType Type;
		public UInt32 ProgramCounter;
		public Int16 Scanline;
		public UInt16 Cycle;
		public Int16 BreakpointId;
		public sbyte DmaChannel;
		public DmaChannelConfig DmaChannelInfo;
		public EventFlags Flags;
		public Int32 RegisterId;
		public MemoryOperationInfo TargetMemory;
		public UInt32 Color;
	};

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropEventViewerCategoryCfg
	{
		[MarshalAs(UnmanagedType.I1)]
		public bool Visible;

		public UInt32 Color;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropSnesEventViewerConfig
	{
		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg Nmi;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		public InteropEventViewerCategoryCfg PpuRegisterReads;
		public InteropEventViewerCategoryCfg PpuRegisterCgramWrites;
		public InteropEventViewerCategoryCfg PpuRegisterVramWrites;
		public InteropEventViewerCategoryCfg PpuRegisterOamWrites;
		public InteropEventViewerCategoryCfg PpuRegisterMode7Writes;
		public InteropEventViewerCategoryCfg PpuRegisterBgOptionWrites;
		public InteropEventViewerCategoryCfg PpuRegisterBgScrollWrites;
		public InteropEventViewerCategoryCfg PpuRegisterWindowWrites;
		public InteropEventViewerCategoryCfg PpuRegisterOtherWrites;

		public InteropEventViewerCategoryCfg ApuRegisterReads;
		public InteropEventViewerCategoryCfg ApuRegisterWrites;
		public InteropEventViewerCategoryCfg CpuRegisterReads;
		public InteropEventViewerCategoryCfg CpuRegisterWrites;
		public InteropEventViewerCategoryCfg WorkRamRegisterReads;
		public InteropEventViewerCategoryCfg WorkRamRegisterWrites;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
		public byte[] ShowDmaChannels = new byte[8];
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropNesEventViewerConfig
	{
		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg Nmi;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		public InteropEventViewerCategoryCfg MapperRegisterWrites;
		public InteropEventViewerCategoryCfg MapperRegisterReads;
		public InteropEventViewerCategoryCfg ApuRegisterWrites;
		public InteropEventViewerCategoryCfg ApuRegisterReads;
		public InteropEventViewerCategoryCfg ControlRegisterWrites;
		public InteropEventViewerCategoryCfg ControlRegisterReads;

		public InteropEventViewerCategoryCfg Ppu2000Write;
		public InteropEventViewerCategoryCfg Ppu2001Write;
		public InteropEventViewerCategoryCfg Ppu2003Write;
		public InteropEventViewerCategoryCfg Ppu2004Write;
		public InteropEventViewerCategoryCfg Ppu2005Write;
		public InteropEventViewerCategoryCfg Ppu2006Write;
		public InteropEventViewerCategoryCfg Ppu2007Write;

		public InteropEventViewerCategoryCfg Ppu2002Read;
		public InteropEventViewerCategoryCfg Ppu2004Read;
		public InteropEventViewerCategoryCfg Ppu2007Read;

		public InteropEventViewerCategoryCfg DmcDmaReads;
		public InteropEventViewerCategoryCfg OtherDmaReads;
		public InteropEventViewerCategoryCfg SpriteZeroHit;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;
		[MarshalAs(UnmanagedType.I1)] public bool ShowNtscBorders;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropGbEventViewerConfig
	{
		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		public InteropEventViewerCategoryCfg PpuRegisterCgramWrites;
		public InteropEventViewerCategoryCfg PpuRegisterVramWrites;
		public InteropEventViewerCategoryCfg PpuRegisterOamWrites;
		public InteropEventViewerCategoryCfg PpuRegisterBgScrollWrites;
		public InteropEventViewerCategoryCfg PpuRegisterWindowWrites;
		public InteropEventViewerCategoryCfg PpuRegisterOtherWrites;

		public InteropEventViewerCategoryCfg PpuRegisterCgramReads;
		public InteropEventViewerCategoryCfg PpuRegisterVramReads;
		public InteropEventViewerCategoryCfg PpuRegisterOamReads;
		public InteropEventViewerCategoryCfg PpuRegisterBgScrollReads;
		public InteropEventViewerCategoryCfg PpuRegisterWindowReads;
		public InteropEventViewerCategoryCfg PpuRegisterOtherReads;

		public InteropEventViewerCategoryCfg ApuRegisterReads;
		public InteropEventViewerCategoryCfg ApuRegisterWrites;

		public InteropEventViewerCategoryCfg SerialReads;
		public InteropEventViewerCategoryCfg SerialWrites;

		public InteropEventViewerCategoryCfg TimerReads;
		public InteropEventViewerCategoryCfg TimerWrites;

		public InteropEventViewerCategoryCfg InputReads;
		public InteropEventViewerCategoryCfg InputWrites;

		public InteropEventViewerCategoryCfg OtherRegisterReads;
		public InteropEventViewerCategoryCfg OtherRegisterWrites;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropGbaEventViewerConfig
	{
		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		public InteropEventViewerCategoryCfg PaletteReads;
		public InteropEventViewerCategoryCfg PaletteWrites;
		public InteropEventViewerCategoryCfg VramReads;
		public InteropEventViewerCategoryCfg VramWrites;
		public InteropEventViewerCategoryCfg OamReads;
		public InteropEventViewerCategoryCfg OamWrites;
		
		public InteropEventViewerCategoryCfg PpuRegisterBgScrollReads;
		public InteropEventViewerCategoryCfg PpuRegisterBgScrollWrites;
		public InteropEventViewerCategoryCfg PpuRegisterWindowReads;
		public InteropEventViewerCategoryCfg PpuRegisterWindowWrites;
		public InteropEventViewerCategoryCfg PpuRegisterOtherReads;
		public InteropEventViewerCategoryCfg PpuRegisterOtherWrites;

		public InteropEventViewerCategoryCfg DmaRegisterReads;
		public InteropEventViewerCategoryCfg DmaRegisterWrites;

		public InteropEventViewerCategoryCfg ApuRegisterReads;
		public InteropEventViewerCategoryCfg ApuRegisterWrites;

		public InteropEventViewerCategoryCfg SerialReads;
		public InteropEventViewerCategoryCfg SerialWrites;

		public InteropEventViewerCategoryCfg TimerReads;
		public InteropEventViewerCategoryCfg TimerWrites;

		public InteropEventViewerCategoryCfg InputReads;
		public InteropEventViewerCategoryCfg InputWrites;

		public InteropEventViewerCategoryCfg OtherRegisterReads;
		public InteropEventViewerCategoryCfg OtherRegisterWrites;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropPceEventViewerConfig
	{
		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		public InteropEventViewerCategoryCfg VdcStatusReads;

		public InteropEventViewerCategoryCfg VdcVramWrites;
		public InteropEventViewerCategoryCfg VdcVramReads;

		public InteropEventViewerCategoryCfg VdcRegSelectWrites;
		public InteropEventViewerCategoryCfg VdcControlWrites;
		public InteropEventViewerCategoryCfg VdcRcrWrites;
		public InteropEventViewerCategoryCfg VdcHvConfigWrites;
		public InteropEventViewerCategoryCfg VdcMemoryWidthWrites;
		public InteropEventViewerCategoryCfg VdcScrollWrites;
		public InteropEventViewerCategoryCfg VdcDmaWrites;

		public InteropEventViewerCategoryCfg VceWrites;
		public InteropEventViewerCategoryCfg VceReads;
		public InteropEventViewerCategoryCfg PsgWrites;
		public InteropEventViewerCategoryCfg PsgReads;
		public InteropEventViewerCategoryCfg TimerWrites;
		public InteropEventViewerCategoryCfg TimerReads;
		public InteropEventViewerCategoryCfg IoWrites;
		public InteropEventViewerCategoryCfg IoReads;
		public InteropEventViewerCategoryCfg IrqControlWrites;
		public InteropEventViewerCategoryCfg IrqControlReads;

		public InteropEventViewerCategoryCfg CdRomWrites;
		public InteropEventViewerCategoryCfg CdRomReads;
		public InteropEventViewerCategoryCfg AdpcmWrites;
		public InteropEventViewerCategoryCfg AdpcmReads;
		public InteropEventViewerCategoryCfg ArcadeCardWrites;
		public InteropEventViewerCategoryCfg ArcadeCardReads;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropSmsEventViewerConfig
	{
		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg Nmi;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		public InteropEventViewerCategoryCfg VdpPaletteWrite;
		public InteropEventViewerCategoryCfg VdpVramWrite;

		public InteropEventViewerCategoryCfg VdpVCounterRead;
		public InteropEventViewerCategoryCfg VdpHCounterRead;
		public InteropEventViewerCategoryCfg VdpVramRead;
		public InteropEventViewerCategoryCfg VdpControlPortRead;
		public InteropEventViewerCategoryCfg VdpControlPortWrite;

		public InteropEventViewerCategoryCfg PsgWrite;
		public InteropEventViewerCategoryCfg IoWrite;
		public InteropEventViewerCategoryCfg IoRead;

		public InteropEventViewerCategoryCfg MemoryControlWrite;

		public InteropEventViewerCategoryCfg GameGearPortWrite;
		public InteropEventViewerCategoryCfg GameGearPortRead;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class InteropWsEventViewerConfig
	{
		public InteropEventViewerCategoryCfg PpuPaletteRead;
		public InteropEventViewerCategoryCfg PpuPaletteWrite;
		public InteropEventViewerCategoryCfg PpuVramRead;
		public InteropEventViewerCategoryCfg PpuVramWrite;
		public InteropEventViewerCategoryCfg PpuVCounterRead;
		public InteropEventViewerCategoryCfg PpuScrollRead;
		public InteropEventViewerCategoryCfg PpuScrollWrite;
		public InteropEventViewerCategoryCfg PpuWindowRead;
		public InteropEventViewerCategoryCfg PpuWindowWrite;
		public InteropEventViewerCategoryCfg PpuOtherRead;
		public InteropEventViewerCategoryCfg PpuOtherWrite;
		public InteropEventViewerCategoryCfg AudioRead;
		public InteropEventViewerCategoryCfg AudioWrite;
		public InteropEventViewerCategoryCfg SerialRead;
		public InteropEventViewerCategoryCfg SerialWrite;
		public InteropEventViewerCategoryCfg DmaRead;
		public InteropEventViewerCategoryCfg DmaWrite;
		public InteropEventViewerCategoryCfg InputRead;
		public InteropEventViewerCategoryCfg InputWrite;
		public InteropEventViewerCategoryCfg IrqRead;
		public InteropEventViewerCategoryCfg IrqWrite;
		public InteropEventViewerCategoryCfg TimerRead;
		public InteropEventViewerCategoryCfg TimerWrite;
		public InteropEventViewerCategoryCfg EepromRead;
		public InteropEventViewerCategoryCfg EepromWrite;
		public InteropEventViewerCategoryCfg CartRead;
		public InteropEventViewerCategoryCfg CartWrite;
		public InteropEventViewerCategoryCfg OtherRead;
		public InteropEventViewerCategoryCfg OtherWrite;

		public InteropEventViewerCategoryCfg Irq;
		public InteropEventViewerCategoryCfg MarkedBreakpoints;

		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;
	}

	public enum TilemapDisplayMode
	{
		Default,
		Grayscale,
		AttributeView
	}

	public enum TilemapHighlightMode
	{
		None,
		Changes,
		Writes
	}

	public class GetTilemapOptions
	{
		public byte Layer;
		public byte[]? CompareVram;
		public AddressCounters[]? AccessCounters;

		public UInt64 MasterClock;
		public TilemapHighlightMode TileHighlightMode;
		public TilemapHighlightMode AttributeHighlightMode;

		public TilemapDisplayMode DisplayMode;

		public InteropGetTilemapOptions ToInterop()
		{
			return new InteropGetTilemapOptions() {
				Layer = Layer,
				MasterClock = MasterClock,
				TileHighlightMode = TileHighlightMode,
				AttributeHighlightMode = AttributeHighlightMode,
				DisplayMode = DisplayMode
			};
		}
	}

	public struct InteropGetTilemapOptions
	{
		public byte Layer;
		public IntPtr CompareVram;
		public IntPtr AccessCounters;
		
		public UInt64 MasterClock;
		public TilemapHighlightMode TileHighlightMode;
		public TilemapHighlightMode AttributeHighlightMode;

		public TilemapDisplayMode DisplayMode;
	}

	public enum TileBackground
	{
		Default,
		Transparent,
		PaletteColor,
		Black,
		White,
		Magenta,
	}

	public enum SpriteBackground
	{
		Gray,
		Background,
		Transparent,
		Black,
		White,
		Magenta,
	}

	public enum SpriteVisibility : byte
	{
		Visible = 0,
		Offscreen = 1,
		Disabled = 2
	}

	public enum NullableBoolean : sbyte
	{
		Undefined = -1,
		False = 0,
		True = 1
	}

	public enum TilemapMirroring
	{
		None,
		Horizontal,
		Vertical,
		SingleScreenA,
		SingleScreenB,
		FourScreens,
	}

	public struct DebugTilemapInfo
	{
		public UInt32 Bpp;
		public TileFormat Format;
		public TilemapMirroring Mirroring;

		public UInt32 TileWidth;
		public UInt32 TileHeight;

		public UInt32 ScrollX;
		public UInt32 ScrollWidth;
		public UInt32 ScrollY;
		public UInt32 ScrollHeight;

		public UInt32 RowCount;
		public UInt32 ColumnCount;
		public UInt32 TilemapAddress;
		public UInt32 TilesetAddress;
		public sbyte Priority;
	}

	public struct DebugTilemapTileInfo
	{
		public Int32 Row;
		public Int32 Column;

		public Int32 Width;
		public Int32 Height;

		public Int32 TileMapAddress;

		public Int32 TileIndex;
		public Int32 TileAddress;
		
		public Int32 PixelData;

		public Int32 PaletteIndex;
		public Int32 PaletteAddress;
		public Int32 BasePaletteIndex;

		public Int32 AttributeAddress;
		public Int16 AttributeData;

		public NullableBoolean HorizontalMirroring;
		public NullableBoolean VerticalMirroring;
		public NullableBoolean HighPriority;
	};

	public enum TileFilter
	{
		None,
		HideUnused,
		HideUsed
	}

	public struct GetTileViewOptions
	{
		public MemoryType MemType;
		public TileFormat Format;
		public TileLayout Layout;
		public TileFilter Filter;
		public TileBackground Background;
		public Int32 Width;
		public Int32 Height;
		public Int32 StartAddress;
		public Int32 Palette;
		[MarshalAs(UnmanagedType.I1)] public bool UseGrayscalePalette;
	}

	public struct GetSpritePreviewOptions
	{
		public SpriteBackground Background;
	}

	public struct GetPaletteInfoOptions
	{
		public TileFormat Format;
	}

	public struct DebugSpritePreviewInfo
	{
		public UInt32 Width;
		public UInt32 Height;
		public UInt32 SpriteCount;
		public Int32 CoordOffsetX;
		public Int32 CoordOffsetY;

		public UInt32 VisibleX;
		public UInt32 VisibleY;
		public UInt32 VisibleWidth;
		public UInt32 VisibleHeight;

		[MarshalAs(UnmanagedType.I1)] public bool WrapBottomToTop;
		[MarshalAs(UnmanagedType.I1)] public bool WrapRightToLeft;
	}

	public enum DebugSpritePriority
	{
		Undefined = -1,
		Number0 = 0,
		Number1 = 1,
		Number2 = 2,
		Number3 = 3,
		Foreground = 4,
		Background = 5
	}

	public unsafe struct DebugSpriteInfo
	{
		public Int32 TileIndex;
		public Int32 TileAddress;
		public Int32 PaletteAddress;
		public TileFormat Format;

		public Int16 SpriteIndex;

		public Int16 X;
		public Int16 Y;
		public Int16 RawX;
		public Int16 RawY;

		public Int16 Bpp;
		public Int16 Palette;
		public DebugSpritePriority Priority;
		public UInt16 Width;
		public UInt16 Height;
		public NullableBoolean HorizontalMirror;
		public NullableBoolean VerticalMirror;
		public NullableBoolean MosaicEnabled;
		public NullableBoolean BlendingEnabled;
		public NullableBoolean WindowMode;
		public NullableBoolean TransformEnabled;
		public NullableBoolean DoubleSize;
		public sbyte TransformParamIndex;
		public SpriteVisibility Visibility;
		[MarshalAs(UnmanagedType.I1)] public bool UseExtendedVram;
		public NullableBoolean UseSecondTable;

		public UInt32 TileCount;
		public fixed UInt32 TileAddresses[8*8];
	}

	public enum RawPaletteFormat
	{
		Indexed,
		Rgb555,
		Rgb333,
		Rgb222,
		Rgb444,
		Bgr444,
	}

	public unsafe struct DebugPaletteInfo
	{
		public MemoryType PaletteMemType;
		public UInt32 PaletteMemOffset;
		[MarshalAs(UnmanagedType.I1)] public bool HasMemType;

		public UInt32 ColorCount;
		public UInt32 BgColorCount;
		public UInt32 SpriteColorCount;
		public UInt32 SpritePaletteOffset;

		public UInt32 ColorsPerPalette;

		public RawPaletteFormat RawFormat;
		public fixed UInt32 RawPalette[512];
		public fixed UInt32 RgbPalette[512];

		public unsafe UInt32[] GetRgbPalette()
		{
			UInt32[] result = new UInt32[ColorCount];
			fixed(UInt32* pal = RgbPalette) {
				for(int i = 0; i < ColorCount; i++) {
					result[i] = pal[i];
				}
			}
			return result;
		}

		public unsafe UInt32[] GetRawPalette()
		{
			UInt32[] result = new UInt32[ColorCount];
			fixed(UInt32* pal = RawPalette) {
				for(int i = 0; i < ColorCount; i++) {
					result[i] = pal[i];
				}
			}
			return result;
		}
	};

	public enum TileFormat
	{
		Bpp2,
		Bpp4,
		Bpp8,
		DirectColor,
		Mode7,
		Mode7DirectColor,
		Mode7ExtBg,
		NesBpp2,
		PceSpriteBpp4,
		PceSpriteBpp2Sp01,
		PceSpriteBpp2Sp23,
		PceBackgroundBpp2Cg0,
		PceBackgroundBpp2Cg1,
		SmsBpp4,
		SmsSgBpp1,
		GbaBpp4,
		GbaBpp8,
		WsBpp4Packed
	}

	public static class TileFormatExtensions
	{
		public static PixelSize GetTileSize(this TileFormat format)
		{
			return format switch {
				TileFormat.PceSpriteBpp4 => new PixelSize(16, 16),
				TileFormat.PceSpriteBpp2Sp01 => new PixelSize(16, 16),
				TileFormat.PceSpriteBpp2Sp23 => new PixelSize(16, 16),
				_ => new PixelSize(8, 8),
			};
		}

		public static int GetBitsPerPixel(this TileFormat format)
		{
			return format switch {
				TileFormat.Bpp2 => 2,
				TileFormat.Bpp4 => 4,
				TileFormat.Bpp8 => 8,
				TileFormat.DirectColor => 8,
				TileFormat.Mode7 => 16,
				TileFormat.Mode7DirectColor => 16,
				TileFormat.Mode7ExtBg => 16,
				TileFormat.NesBpp2 => 2,

				TileFormat.PceSpriteBpp4 => 4,
				//Treat all PCE 2BPP modes as 4bpp because the tile data still
				//covers the same amount of space in RAM as the 4bpp tiles
				TileFormat.PceSpriteBpp2Sp01 => 4,
				TileFormat.PceSpriteBpp2Sp23 => 4,
				TileFormat.PceBackgroundBpp2Cg0 => 4,
				TileFormat.PceBackgroundBpp2Cg1 => 4,
				
				TileFormat.SmsBpp4 => 4,
				TileFormat.SmsSgBpp1 => 1,
				
				TileFormat.GbaBpp4 => 4,
				TileFormat.GbaBpp8 => 8,
				
				TileFormat.WsBpp4Packed => 4,

				_ => throw new Exception("TileFormat not supported"),
			};
		}

		public static int GetBytesPerTile(this TileFormat format)
		{
			int bitsPerPixel = format.GetBitsPerPixel();
			PixelSize tileSize = format.GetTileSize();
			return tileSize.Width * tileSize.Height * bitsPerPixel / 8;
		}
	}

	public enum TileLayout
	{
		Normal,
		SingleLine8x16,
		SingleLine16x16
	};

	[Serializable]
	public struct InteropTraceLoggerOptions
	{
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		[MarshalAs(UnmanagedType.I1)] public bool IndentCode;
		[MarshalAs(UnmanagedType.I1)] public bool UseLabels;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
		public byte[] Condition;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
		public byte[] Format;
	}

	public enum VectorType
	{
		Indirect,
		Direct,
		x86,
		x86WithOffset
	}

	public struct CpuVectorDefinition
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 15)]
		public byte[] Name;
		public UInt32 Address;
		public VectorType Type;
	}

	public struct DebuggerFeatures
	{
		[MarshalAs(UnmanagedType.I1)] public bool RunToIrq;
		[MarshalAs(UnmanagedType.I1)] public bool RunToNmi;
		[MarshalAs(UnmanagedType.I1)] public bool StepOver;
		[MarshalAs(UnmanagedType.I1)] public bool StepOut;
		[MarshalAs(UnmanagedType.I1)] public bool StepBack;
		[MarshalAs(UnmanagedType.I1)] public bool ChangeProgramCounter;
		[MarshalAs(UnmanagedType.I1)] public bool CallStack;
		[MarshalAs(UnmanagedType.I1)] public bool CpuCycleStep;

		public byte IrqVectorOffset;
		public byte CpuVectorCount;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
		public CpuVectorDefinition[] CpuVectors;
	}

	public struct CpuInstructionProgress
	{
		public UInt64 StartCycle;
		public UInt64 CurrentCycle;
		public UInt32 LastOpCode;
		public MemoryOperationInfo LastMemOperation;
	}

	public enum EvalResultType
	{
		Numeric = 0,
		Boolean = 1,
		Invalid = 2,
		DivideBy0 = 3,
		OutOfScope = 4
	}

	public struct StackFrameInfo
	{
		public UInt32 Source;
		public AddressInfo AbsSource;
		public UInt32 Target;
		public AddressInfo AbsTarget;
		public UInt32 Return;
		public UInt32 ReturnStackPointer;
		public AddressInfo AbsReturn;
		public StackFrameFlags Flags;
	};

	public enum StackFrameFlags
	{
		None = 0,
		Nmi = 1,
		Irq = 2
	}
	
	public enum CpuType : byte
	{
		Snes,
		Spc,
		NecDsp,
		Sa1,
		Gsu,
		Cx4,
		St018,
		Gameboy,
		Nes,
		Pce,
		Sms,
		Gba,
		Ws
	}

	public enum StepType
	{
		Step,
		StepOut,
		StepOver,
		CpuCycleStep,
		PpuStep,
		PpuScanline,
		PpuFrame,
		SpecificScanline,
		RunToNmi,
		RunToIrq,
		StepBack
	}

	public enum BreakSource
	{
		Unspecified = -1,
		Breakpoint = 0,
		Pause,
		CpuStep,
		PpuStep,
		Irq,
		Nmi,
		InternalOperation,

		BreakOnBrk,
		BreakOnCop,
		BreakOnWdm,
		BreakOnStp,
		BreakOnUninitMemoryRead,

		GbInvalidOamAccess,
		GbInvalidVramAccess,
		GbDisableLcdOutsideVblank,
		GbInvalidOpCode,
		GbNopLoad,
		GbOamCorruption,

		NesBreakOnDecayedOamRead,
		NesBreakOnPpu2000ScrollGlitch,
		NesBreakOnPpu2006ScrollGlitch,
		BreakOnUnofficialOpCode,
		NesBusConflict,
		NesBreakOnCpuCrash,
		NesBreakOnExtOutputMode,

		PceBreakOnInvalidVramAddress,

		SmsNopLoad,

		GbaInvalidOpCode,
		GbaNopLoad,
		GbaUnalignedMemoryAccess,
		
		BreakOnUndefinedOpCode
	}

	public struct BreakEvent
	{
		public BreakSource Source;
		public CpuType SourceCpu;
		public MemoryOperationInfo Operation;
		public Int32 BreakpointId;
	}

	public enum CdlStripOption
	{
		StripNone = 0,
		StripUnused = 1,
		StripUsed = 2
	}

	public enum CdlFlags : byte
	{
		None = 0x00,
		Code = 0x01,
		Data = 0x02,
		JumpTarget = 0x04,
		SubEntryPoint = 0x08,

		IndexMode8 = 0x10,
		MemoryMode8 = 0x20,

		NesChrDrawn = 0x01,
		NesPcmData = 0x80
	}

	public struct CdlStatistics
	{
		public UInt32 CodeBytes;
		public UInt32 DataBytes;
		public UInt32 TotalBytes;

		public UInt32 JumpTargetCount;
		public UInt32 FunctionCount;

		//CHR ROM (NES-specific)
		public UInt32 DrawnChrBytes;
		public UInt32 TotalChrBytes;
	}

	public struct ProfiledFunction
	{
		public UInt64 ExclusiveCycles;
		public UInt64 InclusiveCycles;
		public UInt64 CallCount;
		public UInt64 MinCycles;
		public UInt64 MaxCycles;
		public AddressInfo Address;
		public StackFrameFlags Flags;

		public UInt64 GetAvgCycles()
		{
			return CallCount == 0 ? 0 : (InclusiveCycles / CallCount);
		}
	}

	public unsafe struct TraceRow
	{
		public UInt32 ProgramCounter;
		public CpuType Type;

		public fixed byte ByteCode[8];
		public byte ByteCodeSize;

		public UInt32 LogSize;
		public fixed byte LogOutput[500];

		public unsafe string GetOutput()
		{
			fixed(byte* output = LogOutput) {
				return UTF8Encoding.UTF8.GetString(output, (int)LogSize);
			}
		}

		public byte[] GetByteCode()
		{
			byte[] result = new byte[ByteCodeSize];
			fixed(byte* ptr = ByteCode) {
				for(int i = 0; i < ByteCodeSize; i++) {
					result[i] = ptr[i];
				}
			}
			return result;
		}

		public unsafe string GetByteCodeStr()
		{
			fixed(byte* output = ByteCode) {
				StringBuilder sb = new StringBuilder();
				int i;
				for(i = 0; i < ByteCodeSize && i < 8; i++) {
					sb.Append(ByteCode[i].ToString("X2") + " ");
				}
				return sb.ToString().Trim();
			}
		}
	}

	public struct DisassemblySearchOptions
	{
		[MarshalAs(UnmanagedType.I1)] public bool MatchCase;
		[MarshalAs(UnmanagedType.I1)] public bool MatchWholeWord;
		[MarshalAs(UnmanagedType.I1)] public bool SearchBackwards;
		[MarshalAs(UnmanagedType.I1)] public bool SkipFirstLine;
	}

	public struct DebugControllerState
	{
		[MarshalAs(UnmanagedType.I1)] public bool A;
		[MarshalAs(UnmanagedType.I1)] public bool B;
		[MarshalAs(UnmanagedType.I1)] public bool X;
		[MarshalAs(UnmanagedType.I1)] public bool Y;
		[MarshalAs(UnmanagedType.I1)] public bool L;
		[MarshalAs(UnmanagedType.I1)] public bool R;
		[MarshalAs(UnmanagedType.I1)] public bool U;
		[MarshalAs(UnmanagedType.I1)] public bool D;
		[MarshalAs(UnmanagedType.I1)] public bool Up;
		[MarshalAs(UnmanagedType.I1)] public bool Down;
		[MarshalAs(UnmanagedType.I1)] public bool Left;
		[MarshalAs(UnmanagedType.I1)] public bool Right;
		[MarshalAs(UnmanagedType.I1)] public bool Select;
		[MarshalAs(UnmanagedType.I1)] public bool Start;
	}
}
