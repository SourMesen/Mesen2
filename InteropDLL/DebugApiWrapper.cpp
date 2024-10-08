#include "Common.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/DebuggerRequest.h"
#include "Core/Debugger/Debugger.h"
#include "Core/Debugger/IDebugger.h"
#include "Core/Debugger/MemoryDumper.h"
#include "Core/Debugger/MemoryAccessCounter.h"
#include "Core/Debugger/CdlManager.h"
#include "Core/Debugger/Disassembler.h"
#include "Core/Debugger/DisassemblySearch.h"
#include "Core/Debugger/DebugTypes.h"
#include "Core/Debugger/Breakpoint.h"
#include "Core/Debugger/BreakpointManager.h"
#include "Core/Debugger/PpuTools.h"
#include "Core/Debugger/CodeDataLogger.h"
#include "Core/Debugger/CallstackManager.h"
#include "Core/Debugger/LabelManager.h"
#include "Core/Debugger/ScriptManager.h"
#include "Core/Debugger/Profiler.h"
#include "Core/Debugger/IAssembler.h"
#include "Core/Debugger/BaseEventManager.h"
#include "Core/Debugger/ITraceLogger.h"
#include "Core/Debugger/TraceLogFileSaver.h"
#include "Core/Debugger/FrozenAddressManager.h"
#include "Core/Gameboy/GbTypes.h"
#include "Utilities/StringUtilities.h"

extern unique_ptr<Emulator> _emu;

template<typename T>
T WrapDebuggerCall(std::function<T(Debugger* debugger)> func)
{
	DebuggerRequest dbgRequest = _emu->GetDebugger(true);
	if(dbgRequest.GetDebugger()) {
		return func(dbgRequest.GetDebugger());
	} else {
		return {};
	}
}

template<>
void WrapDebuggerCall(std::function<void(Debugger* debugger)> func)
{
	DebuggerRequest dbgRequest = _emu->GetDebugger(true);
	if(dbgRequest.GetDebugger()) {
		func(dbgRequest.GetDebugger());
	}
}

#define WithDebugger(t, x) WrapDebuggerCall<t>([&](Debugger* dbg) -> t { return dbg->x; });
#define WithTool(t, x, f) WrapDebuggerCall<t>([&](Debugger* dbg) -> t { if(dbg->x) { return dbg->x->f; } else { return {}; } });
#define WithToolVoid(x, f) WrapDebuggerCall<void>([&](Debugger* dbg) -> void { if(dbg->x) { return dbg->x->f; } });

extern "C"
{
	//Debugger wrapper
	DllExport void __stdcall InitializeDebugger()
	{
		_emu->InitDebugger();
	}

	DllExport void __stdcall ReleaseDebugger()
	{
		_emu->StopDebugger();
	}

	DllExport bool __stdcall IsDebuggerRunning()
	{
		return _emu->GetDebugger().GetDebugger() != nullptr;
	}

	DllExport bool __stdcall IsExecutionStopped() { return WithDebugger(bool, IsExecutionStopped()); }
	DllExport void __stdcall ResumeExecution() { if(IsDebuggerRunning()) WithDebugger(void, Run()); }
	DllExport void __stdcall Step(CpuType cpuType, uint32_t count, StepType type) { WithDebugger(void, Step(cpuType, count, type)); }

	DllExport uint32_t __stdcall GetDisassemblyOutput(CpuType type, uint32_t lineIndex, CodeLineData output[], uint32_t rowCount) { return WithDebugger(uint32_t, GetDisassembler()->GetDisassemblyOutput(type, lineIndex, output, rowCount)); }
	DllExport uint32_t __stdcall GetDisassemblyRowAddress(CpuType type, uint32_t address, int32_t rowOffset) { return WithDebugger(uint32_t, GetDisassembler()->GetDisassemblyRowAddress(type, address, rowOffset)); }
	DllExport int32_t __stdcall SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, DisassemblySearchOptions options) { return WithDebugger(int32_t, GetDisassemblySearch()->SearchDisassembly(type, searchString, startPosition, options)); }
	DllExport uint32_t __stdcall FindOccurrences(CpuType type, const char* searchString, DisassemblySearchOptions options, CodeLineData results[], uint32_t maxResultCount) { return WithDebugger(uint32_t, GetDisassemblySearch()->FindOccurrences(type, searchString, options, results, maxResultCount)); }

	DllExport void __stdcall SetTraceOptions(CpuType type, TraceLoggerOptions options) { WithToolVoid(GetTraceLogger(type), SetOptions(options)); }
	DllExport uint32_t __stdcall GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t lineCount) { return WithDebugger(uint32_t, GetExecutionTrace(output, startOffset, lineCount)); }
	DllExport void __stdcall ClearExecutionTrace() { WithDebugger(void, ClearExecutionTrace()); }

	DllExport void __stdcall StartLogTraceToFile(const char* filename) { WithDebugger(void, GetTraceLogFileSaver()->StartLogging(filename)); }
	DllExport void __stdcall StopLogTraceToFile() { WithDebugger(void, GetTraceLogFileSaver()->StopLogging()); }

	DllExport void __stdcall SetBreakpoints(Breakpoint breakpoints[], uint32_t length) { WithDebugger(void, SetBreakpoints(breakpoints, length)); }
	
	DllExport void __stdcall SetInputOverrides(uint32_t index, DebugControllerState state) { WithDebugger(void, SetInputOverrides(index, state)); }
	DllExport void __stdcall GetAvailableInputOverrides(uint8_t* availableIndexes) { WithDebugger(void, GetAvailableInputOverrides(availableIndexes)); }
	
	DllExport void __stdcall GetTokenList(CpuType cpuType, char* tokenList) { WithDebugger(void, GetTokenList(cpuType, tokenList)); }
	DllExport int64_t __stdcall EvaluateExpression(const char* expression, CpuType cpuType, EvalResultType* resultType, bool useCache) { return WithDebugger(int64_t, EvaluateExpression(expression, cpuType, *resultType, useCache)); }

	DllExport void __stdcall GetCallstack(CpuType cpuType, StackFrameInfo* callstackArray, uint32_t& callstackSize)
	{
		callstackSize = 0;
		WithToolVoid(GetCallstackManager(cpuType), GetCallstack(callstackArray, callstackSize));
	}

	DllExport void __stdcall GetProfilerData(CpuType cpuType, ProfiledFunction* profilerData, uint32_t& functionCount)
	{
		functionCount = 0;
		WithToolVoid(GetCallstackManager(cpuType), GetProfiler()->GetProfilerData(profilerData, functionCount));
	}

	DllExport void __stdcall ResetProfiler(CpuType cpuType) { WithToolVoid(GetCallstackManager(cpuType), GetProfiler()->Reset()); }

	DllExport void __stdcall GetConsoleState(BaseState& state, ConsoleType consoleType) { WithDebugger(void, GetConsoleState(state, consoleType)); }
	DllExport void __stdcall GetCpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, GetCpuState(state, cpuType)); }
	DllExport void __stdcall GetPpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, GetPpuState(state, cpuType)); }

	DllExport void __stdcall SetCpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, SetCpuState(state, cpuType)); }
	DllExport void __stdcall SetPpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, SetPpuState(state, cpuType)); }

	DllExport uint32_t __stdcall GetProgramCounter(CpuType cpuType, bool getInstPc) { return WithDebugger(uint32_t, GetProgramCounter(cpuType, getInstPc)); }
	DllExport void __stdcall SetProgramCounter(CpuType cpuType, uint32_t addr) { WithDebugger(void, SetProgramCounter(cpuType, addr)); }
	DllExport DebuggerFeatures __stdcall GetDebuggerFeatures(CpuType cpuType) { return WithDebugger(DebuggerFeatures, GetDebuggerFeatures(cpuType)); }
	DllExport CpuInstructionProgress __stdcall GetInstructionProgress(CpuType cpuType) { return WithDebugger(CpuInstructionProgress, GetInstructionProgress(cpuType)); }

	DllExport void __stdcall GetDebuggerLog(char* outBuffer, uint32_t maxLength)
	{
		string logString = WithDebugger(string, GetLog());
		StringUtilities::CopyToBuffer(logString, outBuffer, maxLength);
	}
	
	DllExport void __stdcall UpdateFrozenAddresses(CpuType cpuType, uint32_t start, uint32_t end, bool freeze) { return WithToolVoid(GetFrozenAddressManager(cpuType), UpdateFrozenAddresses(start, end, freeze)); }
	DllExport void __stdcall GetFrozenState(CpuType cpuType, uint32_t start, uint32_t end, bool* outState) { return WithToolVoid(GetFrozenAddressManager(cpuType), GetFrozenState(start, end, outState)); }

	DllExport void __stdcall SetMemoryState(MemoryType type, uint8_t* buffer, int32_t length) { WithDebugger(void, GetMemoryDumper()->SetMemoryState(type, buffer, length)); }
	DllExport uint32_t __stdcall GetMemorySize(MemoryType type) { return WithDebugger(uint32_t, GetMemoryDumper()->GetMemorySize(type)); }
	DllExport void __stdcall GetMemoryState(MemoryType type, uint8_t* buffer) { WithDebugger(void, GetMemoryDumper()->GetMemoryState(type, buffer)); }
	DllExport uint8_t __stdcall GetMemoryValue(MemoryType type, uint32_t address) { return WithDebugger(uint8_t, GetMemoryDumper()->GetMemoryValue(type, address)); }
	DllExport void __stdcall GetMemoryValues(MemoryType type, uint32_t start, uint32_t end, uint8_t* output) { WithDebugger(void, GetMemoryDumper()->GetMemoryValues(type, start, end, output)); }
	DllExport void __stdcall SetMemoryValue(MemoryType type, uint32_t address, uint8_t value) { WithDebugger(void, GetMemoryDumper()->SetMemoryValue(type, address, value)); }
	DllExport void __stdcall SetMemoryValues(MemoryType type, uint32_t address, uint8_t* data, int32_t length) { WithDebugger(void, GetMemoryDumper()->SetMemoryValues(type, address, data, length)); }
	DllExport bool __stdcall HasUndoHistory() { return WithDebugger(bool, GetMemoryDumper()->HasUndoHistory()); }
	DllExport void __stdcall PerformUndo() { WithDebugger(void, GetMemoryDumper()->PerformUndo()); }

	DllExport AddressInfo __stdcall GetAbsoluteAddress(AddressInfo relAddress) { return WithDebugger(AddressInfo, GetAbsoluteAddress(relAddress)); }
	DllExport AddressInfo __stdcall GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) { return WithDebugger(AddressInfo, GetRelativeAddress(absAddress, cpuType)); }

	DllExport void __stdcall SetLabel(uint32_t address, MemoryType memType, char* label, char* comment) { WithDebugger(void, GetLabelManager()->SetLabel(address, memType, label, comment)); }
	DllExport void __stdcall ClearLabels() { WithDebugger(void, GetLabelManager()->ClearLabels()); }

	DllExport void __stdcall ResetMemoryAccessCounts() { WithDebugger(void, GetMemoryAccessCounter()->ResetCounts()); }
	DllExport void __stdcall GetMemoryAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters* counts) { WithDebugger(void, GetMemoryAccessCounter()->GetAccessCounts(offset, length, memoryType, counts)); }

	DllExport CdlStatistics __stdcall GetCdlStatistics(MemoryType memoryType) { return WithDebugger(CdlStatistics, GetCdlManager()->GetCdlStatistics(memoryType)); }
	DllExport uint32_t __stdcall GetCdlFunctions(MemoryType memoryType, uint32_t functions[], uint32_t maxSize) { return WithDebugger(uint32_t, GetCdlManager()->GetCdlFunctions(memoryType, functions, maxSize)); }
	DllExport void __stdcall ResetCdl(MemoryType memoryType) { WithDebugger(void, GetCdlManager()->ResetCdl(memoryType)); }
	DllExport void __stdcall SaveCdlFile(MemoryType memoryType, char* cdlFile) { WithDebugger(void, GetCdlManager()->SaveCdlFile(memoryType, cdlFile)); }
	DllExport void __stdcall LoadCdlFile(MemoryType memoryType, char* cdlFile) { WithDebugger(void, GetCdlManager()->LoadCdlFile(memoryType, cdlFile)); }
	DllExport void __stdcall GetCdlData(uint32_t offset, uint32_t length, MemoryType memoryType, uint8_t* cdlData) { WithDebugger(void, GetCdlManager()->GetCdlData(offset, length, memoryType, cdlData)); }
	DllExport void __stdcall SetCdlData(MemoryType memoryType, uint8_t* cdlData, uint32_t length) { WithDebugger(void, GetCdlManager()->SetCdlData(memoryType, cdlData, length)); }
	DllExport void __stdcall MarkBytesAs(MemoryType memoryType, uint32_t start, uint32_t end, uint8_t flags) { WithDebugger(void, GetCdlManager()->MarkBytesAs(memoryType, start, end, flags)); }

	DllExport void __stdcall GetTileView(CpuType cpuType, GetTileViewOptions options, uint8_t* source, uint32_t srcSize, uint32_t* colors, uint32_t* buffer) { WithToolVoid(GetPpuTools(cpuType), GetTileView(options, source, srcSize, colors, buffer)); }

	DllExport void __stdcall GetPpuToolsState(CpuType cpuType, BaseState& state) { return WithToolVoid(GetPpuTools(cpuType), GetPpuToolsState(state)); }
	DllExport DebugTilemapInfo __stdcall GetTilemap(CpuType cpuType, GetTilemapOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outputBuffer) { return WithTool(DebugTilemapInfo, GetPpuTools(cpuType), GetTilemap(options, state, ppuToolsState, vram, palette, outputBuffer)); }
	DllExport FrameInfo __stdcall GetTilemapSize(CpuType cpuType, GetTilemapOptions options, BaseState& state) { return WithTool(FrameInfo, GetPpuTools(cpuType), GetTilemapSize(options, state)); }
	DllExport DebugTilemapTileInfo __stdcall GetTilemapTileInfo(uint32_t x, uint32_t y, CpuType cpuType, GetTilemapOptions options, uint8_t* vram, BaseState& state, BaseState& ppuToolsState) { return WithTool(DebugTilemapTileInfo, GetPpuTools(cpuType), GetTilemapTileInfo(x, y, vram, options, state, ppuToolsState)); }

	DllExport DebugSpritePreviewInfo __stdcall GetSpritePreviewInfo(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState) { return WithTool(DebugSpritePreviewInfo, GetPpuTools(cpuType), GetSpritePreviewInfo(options, state, ppuToolsState)); }
	DllExport void __stdcall GetSpriteList(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo sprites[], uint32_t* spritePreviews, uint32_t* screenPreview) { WithToolVoid(GetPpuTools(cpuType), GetSpriteList(options, state, ppuToolsState, vram, oamRam, palette, sprites, spritePreviews, screenPreview)); }

	DllExport DebugPaletteInfo __stdcall GetPaletteInfo(CpuType cpuType, GetPaletteInfoOptions options) { return WithTool(DebugPaletteInfo, GetPpuTools(cpuType), GetPaletteInfo(options)); }
	DllExport void __stdcall SetPaletteColor(CpuType cpuType, int32_t colorIndex, uint32_t color) { WithToolVoid(GetPpuTools(cpuType), SetPaletteColor(colorIndex, color)); }
	
	DllExport int32_t __stdcall GetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y) { return WithTool(int32_t, GetPpuTools(DebugUtilities::ToCpuType(tileAddress.Type)), GetTilePixel(tileAddress, format, x, y)); }
	DllExport void __stdcall SetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t color) { WithToolVoid(GetPpuTools(DebugUtilities::ToCpuType(tileAddress.Type)), SetTilePixel(tileAddress, format, x, y, color)); }

	DllExport void __stdcall SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle, CpuType cpuType) { WithToolVoid(GetPpuTools(cpuType), SetViewerUpdateTiming(viewerId, scanline, cycle)); }
	DllExport void __stdcall RemoveViewerId(uint32_t viewerId, CpuType cpuType) { WithToolVoid(GetPpuTools(cpuType), RemoveViewer(viewerId)); }

	DllExport void __stdcall SetEventViewerConfig(CpuType cpuType, BaseEventViewerConfig& config) { WithToolVoid(GetEventManager(cpuType), SetConfiguration(config)); }
	DllExport void __stdcall GetDebugEvents(CpuType cpuType, DebugEventInfo* infoArray, uint32_t& maxEventCount) { WithToolVoid(GetEventManager(cpuType), GetEvents(infoArray, maxEventCount)); }
	DllExport uint32_t __stdcall GetDebugEventCount(CpuType cpuType) { return WithTool(uint32_t, GetEventManager(cpuType), GetEventCount()); }
	DllExport FrameInfo __stdcall GetEventViewerDisplaySize(CpuType cpuType) { return WithTool(FrameInfo, GetEventManager(cpuType), GetDisplayBufferSize()); }
	DllExport void __stdcall GetEventViewerOutput(CpuType cpuType, uint32_t* buffer, uint32_t bufferSize) { WithToolVoid(GetEventManager(cpuType), GetDisplayBuffer(buffer, bufferSize)); }
	DllExport DebugEventInfo __stdcall GetEventViewerEvent(CpuType cpuType, uint16_t scanline, uint16_t cycle) { return WithTool(DebugEventInfo, GetEventManager(cpuType), GetEvent(scanline, cycle)); }
	DllExport uint32_t __stdcall TakeEventSnapshot(CpuType cpuType, bool forAutoRefresh) { return WithTool(uint32_t, GetEventManager(cpuType), TakeEventSnapshot(forAutoRefresh)); }

	DllExport int32_t __stdcall LoadScript(char* name, char* path, char* content, int32_t scriptId) { return WithTool(int32_t, GetScriptManager(), LoadScript(name, path, content, scriptId)); }
	DllExport void __stdcall RemoveScript(int32_t scriptId) { WithToolVoid(GetScriptManager(), RemoveScript(scriptId)); }

	DllExport void __stdcall GetScriptLog(int32_t scriptId, char* outScriptLog, uint32_t maxLength)
	{
		string log = WithTool(string, GetScriptManager(), GetScriptLog(scriptId));
		StringUtilities::CopyToBuffer(log, outScriptLog, maxLength);
	}

	DllExport uint32_t __stdcall AssembleCode(CpuType cpuType, char* code, uint32_t startAddress, int16_t* assembledOutput) { return WithTool(uint32_t, GetAssembler(cpuType), AssembleCode(code, startAddress, assembledOutput)); }
	
	DllExport void __stdcall GetRomHeader(uint8_t* headerData, uint32_t& size) { WithToolVoid(GetMainDebugger(), GetRomHeader(headerData, size)); }
	DllExport bool __stdcall SaveRomToDisk(char* filename, bool saveIpsFile, CdlStripOption cdlStripOption) { return WithDebugger(bool, SaveRomToDisk(filename, saveIpsFile, cdlStripOption)); }
};