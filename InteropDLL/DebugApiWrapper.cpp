#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/Debugger.h"
#include "../Core/TraceLogger.h"
#include "../Core/MemoryDumper.h"
#include "../Core/MemoryAccessCounter.h"
#include "../Core/Disassembler.h"
#include "../Core/DebugTypes.h"
#include "../Core/Breakpoint.h"
#include "../Core/BreakpointManager.h"
#include "../Core/PpuTools.h"
#include "../Core/CodeDataLogger.h"
#include "../Core/EventManager.h"
#include "../Core/CallstackManager.h"

extern shared_ptr<Console> _console;

shared_ptr<Debugger> GetDebugger()
{
	return _console->GetDebugger();
}

extern "C"
{
	//Debugger wrapper
	DllExport void __stdcall InitializeDebugger()
	{
		GetDebugger();
	}

	DllExport void __stdcall ReleaseDebugger()
	{
		_console->StopDebugger();
	}

	DllExport bool __stdcall IsDebuggerRunning()
	{
		return _console->GetDebugger(false).get() != nullptr;
	}

	DllExport bool __stdcall IsExecutionStopped() { return GetDebugger()->IsExecutionStopped(); }
	DllExport void __stdcall ResumeExecution() { if(IsDebuggerRunning()) GetDebugger()->Run(); }
	DllExport void __stdcall Step(uint32_t count, StepType type) { GetDebugger()->Step(count, type); }

	DllExport void __stdcall GetDisassemblyLineData(uint32_t lineIndex, CodeLineData &data) { GetDebugger()->GetDisassembler()->GetLineData(lineIndex, data); }
	DllExport uint32_t __stdcall GetDisassemblyLineCount() { return GetDebugger()->GetDisassembler()->GetLineCount(); }
	DllExport uint32_t __stdcall GetDisassemblyLineIndex(uint32_t cpuAddress) { return GetDebugger()->GetDisassembler()->GetLineIndex(cpuAddress); }
	DllExport int32_t __stdcall SearchDisassembly(const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards) { return GetDebugger()->GetDisassembler()->SearchDisassembly(searchString, startPosition, endPosition, searchBackwards); }

	DllExport void __stdcall SetTraceOptions(TraceLoggerOptions options) { GetDebugger()->GetTraceLogger()->SetOptions(options); }
	DllExport void __stdcall StartTraceLogger(char* filename) { GetDebugger()->GetTraceLogger()->StartLogging(filename); }
	DllExport void __stdcall StopTraceLogger() { GetDebugger()->GetTraceLogger()->StopLogging(); }
	DllExport const char* GetExecutionTrace(uint32_t lineCount) { return GetDebugger()->GetTraceLogger()->GetExecutionTrace(lineCount); }

	DllExport void __stdcall SetBreakpoints(Breakpoint breakpoints[], uint32_t length) { GetDebugger()->GetBreakpointManager()->SetBreakpoints(breakpoints, length); }
	DllExport int32_t __stdcall EvaluateExpression(char* expression, EvalResultType *resultType, bool useCache) { return GetDebugger()->EvaluateExpression(expression, *resultType, useCache); }
	DllExport void __stdcall GetCallstack(StackFrameInfo *callstackArray, uint32_t &callstackSize) { GetDebugger()->GetCallstackManager()->GetCallstack(callstackArray, callstackSize); }

	DllExport void __stdcall GetState(DebugState &state) { GetDebugger()->GetState(state); }

	DllExport void __stdcall SetMemoryState(SnesMemoryType type, uint8_t *buffer, int32_t length) { GetDebugger()->GetMemoryDumper()->SetMemoryState(type, buffer, length); }
	DllExport uint32_t __stdcall GetMemorySize(SnesMemoryType type) { return GetDebugger()->GetMemoryDumper()->GetMemorySize(type); }
	DllExport void __stdcall GetMemoryState(SnesMemoryType type, uint8_t *buffer) { GetDebugger()->GetMemoryDumper()->GetMemoryState(type, buffer); }
	DllExport uint8_t __stdcall GetMemoryValue(SnesMemoryType type, uint32_t address) { return GetDebugger()->GetMemoryDumper()->GetMemoryValue(type, address); }
	DllExport void __stdcall SetMemoryValue(SnesMemoryType type, uint32_t address, uint8_t value) { return GetDebugger()->GetMemoryDumper()->SetMemoryValue(type, address, value); }
	DllExport void __stdcall SetMemoryValues(SnesMemoryType type, uint32_t address, uint8_t* data, int32_t length) { return GetDebugger()->GetMemoryDumper()->SetMemoryValues(type, address, data, length); }

	DllExport void __stdcall GetMemoryAccessStamps(uint32_t offset, uint32_t length, SnesMemoryType memoryType, MemoryOperationType operationType, uint64_t* stamps) { GetDebugger()->GetMemoryAccessCounter()->GetAccessStamps(offset, length, memoryType, operationType, stamps); }
	DllExport void __stdcall GetMemoryAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, MemoryOperationType operationType, uint32_t* counts) { GetDebugger()->GetMemoryAccessCounter()->GetAccessCounts(offset, length, memoryType, operationType, counts); }
	DllExport void __stdcall GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t* cdlData) { GetDebugger()->GetCodeDataLogger()->GetCdlData(offset, length, memoryType, cdlData); }

	DllExport void __stdcall GetTilemap(GetTilemapOptions options, uint8_t *vram, uint8_t *cgram, uint32_t *buffer) { GetDebugger()->GetPpuTools()->GetTilemap(options, vram, cgram, buffer); }
	DllExport void __stdcall GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint8_t *cgram, uint32_t *buffer) { GetDebugger()->GetPpuTools()->GetTileView(options, source, srcSize, cgram, buffer); }
	DllExport void __stdcall SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle) { GetDebugger()->GetPpuTools()->SetViewerUpdateTiming(viewerId, scanline, cycle); }

	DllExport void __stdcall GetDebugEvents(DebugEventInfo *infoArray, uint32_t &maxEventCount, bool getPreviousFrameData) { GetDebugger()->GetEventManager()->GetEvents(infoArray, maxEventCount, getPreviousFrameData); }
	DllExport uint32_t __stdcall GetDebugEventCount(bool getPreviousFrameData) { return GetDebugger()->GetEventManager()->GetEventCount(getPreviousFrameData); }
	DllExport void __stdcall GetEventViewerOutput(uint32_t *buffer, EventViewerDisplayOptions options) { GetDebugger()->GetEventManager()->GetDisplayBuffer(buffer, options); }
	DllExport DebugEventInfo __stdcall GetEventViewerEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions options) { return GetDebugger()->GetEventManager()->GetEvent(scanline, cycle, options); }
	DllExport void __stdcall TakeEventSnapshot(EventViewerDisplayOptions options) { GetDebugger()->GetEventManager()->TakeEventSnapshot(options); }
};