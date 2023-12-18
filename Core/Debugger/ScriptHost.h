#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/ScriptingContext.h"
#include "Shared/EventType.h"
#include "Shared/MemoryOperationType.h"
#include "Utilities/safe_ptr.h"

class Debugger;

class ScriptHost
{
private:
	safe_ptr<ScriptingContext> _context;
	int _scriptId = 0;

public:
	ScriptHost(int scriptId);

	int GetScriptId();
	string GetLog();

	bool LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger);
	void RefreshMemoryCallbackFlags() { _context->RefreshMemoryCallbackFlags(); }

	void ProcessEvent(EventType eventType, CpuType cpuType);

	template<typename T>
	__forceinline void CallMemoryCallback(AddressInfo relAddr, T& value, CallbackType callbackType, CpuType cpuType)
	{
		_context->CallMemoryCallback(relAddr, value, callbackType, cpuType);
	}
};