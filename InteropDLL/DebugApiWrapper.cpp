#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/Debugger.h"
#include "../Core/TraceLogger.h"

extern shared_ptr<Console> _console;
shared_ptr<Debugger> _debugger;

shared_ptr<Debugger> GetDebugger()
{
	if(!_debugger) {
		_debugger = _console->GetDebugger();
	}
	
	return _debugger;
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
		_debugger.reset();
		//_console->StopDebugger();
	}

	DllExport bool __stdcall IsDebuggerRunning()
	{
		return _console->GetDebugger(false).get() != nullptr;
	}

	DllExport bool __stdcall IsExecutionStopped() { return GetDebugger()->IsExecutionStopped(); }
	DllExport void __stdcall ResumeExecution() { GetDebugger()->Run(); }
	DllExport void __stdcall Step(uint32_t count) { GetDebugger()->Step(count); }
	//DllExport const char* __stdcall DebugGetCode(uint32_t &length) { return GetDebugger()->GetCode(length); }

	DllExport void __stdcall SetTraceOptions(TraceLoggerOptions options) { GetDebugger()->GetTraceLogger()->SetOptions(options); }
	DllExport void __stdcall StartTraceLogger(char* filename) { GetDebugger()->GetTraceLogger()->StartLogging(filename); }
	DllExport void __stdcall StopTraceLogger() { GetDebugger()->GetTraceLogger()->StopLogging(); }
	DllExport const char* GetExecutionTrace(uint32_t lineCount) { return GetDebugger()->GetTraceLogger()->GetExecutionTrace(lineCount); }

	DllExport void __stdcall GetState(DebugState *state) { GetDebugger()->GetState(state); }
};