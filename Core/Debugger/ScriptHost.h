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

	void ProcessEvent(EventType eventType);
	bool ProcessSavestate();

	bool CheckStateLoadedFlag();
	void RefreshMemoryCallbackFlags() { _context->RefreshMemoryCallbackFlags(); }

	__forceinline void ProcessMemoryOperation(AddressInfo relAddr, uint8_t& value, MemoryOperationType type, CpuType cpuType)
	{
		switch(type) {
			case MemoryOperationType::Read:
			case MemoryOperationType::DmaRead:
			case MemoryOperationType::PpuRenderingRead:
			case MemoryOperationType::DummyRead:
				_context->CallMemoryCallback(relAddr, value, CallbackType::Read, cpuType);
				break;

			case MemoryOperationType::Write:
			case MemoryOperationType::DummyWrite:
			case MemoryOperationType::DmaWrite:
				_context->CallMemoryCallback(relAddr, value, CallbackType::Write, cpuType);
				break;

			case MemoryOperationType::ExecOpCode:
			case MemoryOperationType::ExecOperand:
				_context->CallMemoryCallback(relAddr, value, CallbackType::Exec, cpuType);
				break;

			default: break;
		}
	}
};