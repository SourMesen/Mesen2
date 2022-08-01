#pragma once
#include "stdafx.h"
#include <deque>
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"
#include "Debugger/DebugTypes.h"
#include "EventType.h"

class Debugger;
struct lua_State;

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
	CpuType Type;
	int Reference;
};

class ScriptingContext
{
private:
	static ScriptingContext* _context;
	lua_State* _lua = nullptr;
	Timer _timer;
	EmuSettings* _settings = nullptr;

	deque<string> _logRows;
	SimpleLock _logLock;
	bool _inStartFrameEvent = false;
	bool _inExecOpEvent = false;

	Debugger* _debugger = nullptr;
	CpuType _defaultCpuType = {};
	MemoryType _defaultMemType = {};

	std::unordered_map<int32_t, string> _saveSlotData;
	int32_t _saveSlot = -1;
	int32_t _loadSlot = -1;
	bool _stateLoaded = false;

	static void ExecutionCountHook(lua_State* lua);
	void LuaOpenLibs(lua_State* L, bool allowIoOsAccess);

protected:
	string _scriptName;
	bool _initDone = false;

	vector<MemoryCallback> _callbacks[3];
	vector<int> _eventCallbacks[(int)EventType::LastValue + 1];

	void InternalCallMemoryCallback(uint32_t addr, uint8_t& value, CallbackType type, CpuType cpuType);
	int InternalCallEventCallback(EventType type);

public:
	ScriptingContext(Debugger* debugger);
	~ScriptingContext();
	bool LoadScript(string scriptName, string scriptContent, Debugger* debugger);

	void Log(string message);
	string GetLog();

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

	void CallMemoryCallback(uint32_t addr, uint8_t &value, CallbackType type, CpuType cpuType);
	int CallEventCallback(EventType type);
	bool CheckInitDone();
	bool CheckInStartFrameEvent();
	bool CheckInExecOpEvent();
	bool CheckStateLoadedFlag();

	CpuType GetDefaultCpuType() { return _defaultCpuType; }
	MemoryType GetDefaultMemType() { return _defaultMemType; }
	
	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, CpuType cpuType, int reference);
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, CpuType cpuType, int reference);
	void RegisterEventCallback(EventType type, int reference);
	void UnregisterEventCallback(EventType type, int reference);
};
