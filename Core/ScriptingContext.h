#pragma once
#include "stdafx.h"
#include <deque>
#include "../Utilities/SimpleLock.h"
#include "EventType.h"
#include "DebugTypes.h"

class Debugger;

enum class CallbackType
{
	CpuRead = 0,
	CpuWrite = 1,
	CpuExec = 2
};

struct MemoryCallback
{
	uint32_t StartAddress;
	uint32_t EndAddress;
	int Reference;
};

class ScriptingContext
{
private:
	//Must be static to be thread-safe when switching game
	//UI updates all script windows in a single thread, so this is safe
	static string _log;

	std::deque<string> _logRows;
	SimpleLock _logLock;
	bool _inStartFrameEvent = false;
	bool _inExecOpEvent = false;

	Debugger* _debugger = nullptr;

	std::unordered_map<int32_t, string> _saveSlotData;
	int32_t _saveSlot = -1;
	int32_t _loadSlot = -1;
	bool _stateLoaded = false;

protected:
	string _scriptName;
	bool _initDone = false;

	vector<MemoryCallback> _callbacks[3];
	vector<int> _eventCallbacks[(int)EventType::EventTypeSize];

	virtual void InternalCallMemoryCallback(uint32_t addr, uint8_t &value, CallbackType type) = 0;
	virtual int InternalCallEventCallback(EventType type) = 0;

public:
	ScriptingContext(Debugger* debugger);
	virtual ~ScriptingContext() {}
	virtual bool LoadScript(string scriptName, string scriptContent, Debugger* debugger) = 0;

	void Log(string message);
	const char* GetLog();

	Debugger* GetDebugger();
	string GetScriptName();

	void RequestSaveState(int slot);
	bool RequestLoadState(int slot);
	void SaveState();
	bool LoadState();
	bool LoadState(string stateData);
	string GetSavestateData(int slot);
	void ClearSavestateData(int slot);
	bool ProcessSavestate();

	void CallMemoryCallback(uint32_t addr, uint8_t &value, CallbackType type);
	int CallEventCallback(EventType type);
	bool CheckInitDone();
	bool CheckInStartFrameEvent();
	bool CheckInExecOpEvent();
	bool CheckStateLoadedFlag();
	
	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, int reference);
	virtual void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, int reference);
	void RegisterEventCallback(EventType type, int reference);
	virtual void UnregisterEventCallback(EventType type, int reference);
};
