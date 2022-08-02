#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/ScriptHost.h"
#include "Utilities/SimpleLock.h"
#include "EventType.h"

class Debugger;
enum class MemoryOperationType;

class ScriptManager
{
private:
	Debugger *_debugger = nullptr;
	bool _hasScript = false;
	SimpleLock _scriptLock;
	int _nextScriptId = 0;
	bool _isCpuMemoryCallbackEnabled = false;
	bool _isPpuMemoryCallbackEnabled = false;
	vector<unique_ptr<ScriptHost>> _scripts;
	
	void RefreshMemoryCallbackFlags();

public:
	ScriptManager(Debugger *debugger);

	__forceinline bool HasScript() { return _hasScript; }
	int32_t LoadScript(string name, string content, int32_t scriptId);
	void RemoveScript(int32_t scriptId);
	string GetScriptLog(int32_t scriptId);
	void ProcessEvent(EventType type);

	void EnableCpuMemoryCallbacks() { _isCpuMemoryCallbackEnabled = true; }
	bool HasCpuMemoryCallbacks() { return _scripts.size() && _isCpuMemoryCallbackEnabled; }

	void EnablePpuMemoryCallbacks() { _isPpuMemoryCallbackEnabled = true; }
	bool HasPpuMemoryCallbacks() { return _scripts.size() && _isPpuMemoryCallbackEnabled; }
	
	__forceinline void ProcessMemoryOperation(AddressInfo relAddr, uint8_t& value, MemoryOperationType type, CpuType cpuType)
	{
		for(unique_ptr<ScriptHost>& script : _scripts) {
			script->ProcessMemoryOperation(relAddr, value, type, cpuType);
		}
	}
};