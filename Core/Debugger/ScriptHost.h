#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/ScriptingContext.h"
#include "EventType.h"
#include "MemoryOperationType.h"
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

	bool LoadScript(string scriptName, string scriptContent, Debugger* debugger);
	void RefreshMemoryCallbackFlags() { _context->RefreshMemoryCallbackFlags(); }

	void ProcessEvent(EventType eventType);

	__forceinline void CallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType callbackType, CpuType cpuType)
	{
		_context->CallMemoryCallback(relAddr, value, callbackType, cpuType);
	}
};