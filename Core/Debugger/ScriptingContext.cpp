#include "stdafx.h"
#include <algorithm>
#include "Debugger/ScriptingContext.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Shared/Emulator.h"
#include "Shared/SaveStateManager.h"

ScriptingContext::ScriptingContext(Debugger *debugger)
{
	_debugger = debugger;
}

void ScriptingContext::Log(string message)
{
	auto lock = _logLock.AcquireSafe();
	_logRows.push_back(message);
	if(_logRows.size() > 500) {
		_logRows.pop_front();
	}
}

string ScriptingContext::GetLog()
{
	auto lock = _logLock.AcquireSafe();
	stringstream ss;
	for(string &msg : _logRows) {
		ss << msg << "\n";
	}
	return ss.str();
}

Debugger* ScriptingContext::GetDebugger()
{
	return _debugger;
}

string ScriptingContext::GetScriptName()
{
	return _scriptName;
}

void ScriptingContext::CallMemoryCallback(uint32_t addr, uint8_t &value, CallbackType type, CpuType cpuType)
{
	_inExecOpEvent = type == CallbackType::CpuExec;
	InternalCallMemoryCallback(addr, value, type, cpuType);
	_inExecOpEvent = false;
}

int ScriptingContext::CallEventCallback(EventType type)
{
	_inStartFrameEvent = type == EventType::StartFrame;
	int returnValue = InternalCallEventCallback(type);
	_inStartFrameEvent = false;

	return returnValue;
}

bool ScriptingContext::CheckInitDone()
{
	return _initDone;
}

bool ScriptingContext::CheckInStartFrameEvent()
{
	return _inStartFrameEvent;
}

bool ScriptingContext::CheckInExecOpEvent()
{
	return _inExecOpEvent;
}

bool ScriptingContext::CheckStateLoadedFlag()
{
	bool stateLoaded = _stateLoaded;
	_stateLoaded = false;
	return stateLoaded;
}

void ScriptingContext::RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, CpuType cpuType, int reference)
{
	if(endAddr < startAddr) {
		return;
	}

	if(startAddr == 0 && endAddr == 0) {
		endAddr = 0xFFFFFF;
	}

	MemoryCallback callback;
	callback.StartAddress = (uint32_t)startAddr;
	callback.EndAddress = (uint32_t)endAddr;
	callback.Reference = reference;
	callback.Type = cpuType;
	_callbacks[(int)type].push_back(callback);
}

void ScriptingContext::UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, CpuType cpuType, int reference)
{
	if(endAddr < startAddr) {
		return;
	}

	if(startAddr == 0 && endAddr == 0) {
		endAddr = 0xFFFFFF;
	}

	for(size_t i = 0; i < _callbacks[(int)type].size(); i++) {
		MemoryCallback &callback = _callbacks[(int)type][i];
		if(callback.Reference == reference && callback.Type == cpuType && (int)callback.StartAddress == startAddr && (int)callback.EndAddress == endAddr) {
			_callbacks[(int)type].erase(_callbacks[(int)type].begin() + i);
			break;
		}
	}
}

void ScriptingContext::RegisterEventCallback(EventType type, int reference)
{
	_eventCallbacks[(int)type].push_back(reference);
}

void ScriptingContext::UnregisterEventCallback(EventType type, int reference)
{
	vector<int> &callbacks = _eventCallbacks[(int)type];
	callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), reference), callbacks.end());
}

void ScriptingContext::RequestSaveState(int slot)
{
	_saveSlot = slot;
	if(_inExecOpEvent) {
		SaveState();
	} else {
		_saveSlotData.erase(slot);
	}
}

bool ScriptingContext::RequestLoadState(int slot)
{
	if(_saveSlotData.find(slot) != _saveSlotData.end()) {
		_loadSlot = slot;
		if(_inExecOpEvent) {
			return LoadState();
		} else {
			return true;
		}
	}
	return false;
}

void ScriptingContext::SaveState()
{
	if(_saveSlot >= 0) {
		stringstream ss;
		_debugger->GetEmulator()->GetSaveStateManager()->SaveState(ss);
		_saveSlotData[_saveSlot] = ss.str();
		_saveSlot = -1;
	}
}

bool ScriptingContext::LoadState()
{
	if(_loadSlot >= 0 && _saveSlotData.find(_loadSlot) != _saveSlotData.end()) {
		stringstream ss;
		ss << _saveSlotData[_loadSlot];
		bool result = _debugger->GetEmulator()->GetSaveStateManager()->LoadState(ss);
		_loadSlot = -1;
		if(result) {
			_stateLoaded = true;
		}
		return result;
	}
	return false;
}

bool ScriptingContext::LoadState(string stateData)
{
	stringstream ss;
	ss << stateData;
	bool result = _debugger->GetEmulator()->GetSaveStateManager()->LoadState(ss);
	if(result) {
		_stateLoaded = true;
	}
	return result;
}

bool ScriptingContext::ProcessSavestate()
{
	SaveState();
	return LoadState();
}

string ScriptingContext::GetSavestateData(int slot)
{
	if(slot >= 0) {
		auto result = _saveSlotData.find(slot);
		if(result != _saveSlotData.end()) {
			return result->second;
		}
	}

	return "";
}

void ScriptingContext::ClearSavestateData(int slot)
{
	if(slot >= 0) {
		_saveSlotData.erase(slot);
	}
}