#include "stdafx.h"
#include "ScriptManager.h"
#include "ScriptHost.h"
#include "DebugBreakHelper.h"
#include "Debugger.h"
#include "CpuTypes.h"
#include "Emulator.h"
#include "DebugHud.h"

ScriptManager::ScriptManager(Debugger* debugger)
{
	_debugger = debugger;
	_hasScript = false;
	_nextScriptId = 1;
}

int ScriptManager::LoadScript(string name, string content, int32_t scriptId)
{
	DebugBreakHelper helper(_debugger);
	auto lock = _scriptLock.AcquireSafe();

	if(scriptId < 0) {
		shared_ptr<ScriptHost> script(new ScriptHost(_nextScriptId++));
		script->LoadScript(name, content, _debugger);
		_scripts.push_back(script);
		_hasScript = true;
		return script->GetScriptId();
	} else {
		auto result = std::find_if(_scripts.begin(), _scripts.end(), [=](shared_ptr<ScriptHost> &script) {
			return script->GetScriptId() == scriptId;
		});
		if(result != _scripts.end()) {
			//Send a ScriptEnded event before reloading the code
			(*result)->ProcessEvent(EventType::ScriptEnded);

			(*result)->LoadScript(name, content, _debugger);
			return scriptId;
		}
	}

	return -1;
}

void ScriptManager::RemoveScript(int32_t scriptId)
{
	DebugBreakHelper helper(_debugger);
	auto lock = _scriptLock.AcquireSafe();
	_scripts.erase(std::remove_if(_scripts.begin(), _scripts.end(), [=](const shared_ptr<ScriptHost>& script) {
		if(script->GetScriptId() == scriptId) {
			//Send a ScriptEnded event before unloading the script
			script->ProcessEvent(EventType::ScriptEnded);
			_debugger->GetEmulator()->GetDebugHud()->ClearScreen();
			return true;
		}
		return false;
	}), _scripts.end());
	_hasScript = _scripts.size() > 0;
}

const char* ScriptManager::GetScriptLog(int32_t scriptId)
{
	auto lock = _scriptLock.AcquireSafe();
	for(shared_ptr<ScriptHost> &script : _scripts) {
		if(script->GetScriptId() == scriptId) {
			return script->GetLog();
		}
	}
	return "";
}

void ScriptManager::ProcessEvent(EventType type)
{
	if(_hasScript) {
		for(shared_ptr<ScriptHost> &script : _scripts) {
			script->ProcessEvent(type);
		}
	}
}

void ScriptManager::ProcessMemoryOperation(uint32_t address, uint8_t &value, MemoryOperationType type, CpuType cpuType)
{
	if(_hasScript) {
		for(shared_ptr<ScriptHost> &script : _scripts) {
			script->ProcessMemoryOperation(address, value, type, cpuType);
		}
	}
}
