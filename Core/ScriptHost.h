#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "EventType.h"
#include "MemoryOperationType.h"

class ScriptingContext;
class Debugger;

class ScriptHost
{
private:
	shared_ptr<ScriptingContext> _context;
	int _scriptId;

public:
	ScriptHost(int scriptId);

	int GetScriptId();
	const char* GetLog();

	bool LoadScript(string scriptName, string scriptContent, Debugger* debugger);

	void ProcessMemoryOperation(uint32_t addr, uint8_t &value, MemoryOperationType type, CpuType cpuType);
	void ProcessEvent(EventType eventType);
	bool ProcessSavestate();

	bool CheckStateLoadedFlag();
};