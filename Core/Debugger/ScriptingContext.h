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
	Read = 0,
	Write = 1,
	Exec = 2
};

struct MemoryCallback
{
	uint32_t StartAddress;
	uint32_t EndAddress;
	CpuType Cpu;
	MemoryType MemType;
	int Reference;
};

enum class ScriptDrawSurface
{
	ConsoleScreen,
	ScriptHud
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
	
	ScriptDrawSurface _drawSurface = ScriptDrawSurface::ConsoleScreen;

	static void ExecutionCountHook(lua_State* lua);
	void LuaOpenLibs(lua_State* L, bool allowIoOsAccess);

protected:
	string _scriptName;
	bool _initDone = false;

	vector<MemoryCallback> _callbacks[3];
	vector<int> _eventCallbacks[(int)EventType::LastValue + 1];

	void InternalCallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType);
	int InternalCallEventCallback(EventType type);
	bool IsAddressMatch(MemoryCallback& callback, AddressInfo addr);

public:
	ScriptingContext(Debugger* debugger);
	~ScriptingContext();
	bool LoadScript(string scriptName, string scriptContent, Debugger* debugger);

	void Log(string message);
	string GetLog();

	Debugger* GetDebugger();
	string GetScriptName();

	void SetDrawSurface(ScriptDrawSurface surface) { _drawSurface = surface; }
	ScriptDrawSurface GetDrawSurface() { return _drawSurface; }

	void RequestSaveState(int slot);
	bool RequestLoadState(int slot);
	void SaveState();
	bool LoadState();
	bool LoadState(string stateData);
	string GetSavestateData(int slot);
	void ClearSavestateData(int slot);
	bool ProcessSavestate();

	void CallMemoryCallback(AddressInfo relAddr, uint8_t &value, CallbackType type, CpuType cpuType);
	int CallEventCallback(EventType type);
	bool CheckInitDone();
	bool CheckInStartFrameEvent();
	bool CheckInExecOpEvent();
	bool CheckStateLoadedFlag();

	CpuType GetDefaultCpuType() { return _defaultCpuType; }
	MemoryType GetDefaultMemType() { return _defaultMemType; }
	
	void RefreshMemoryCallbackFlags();

	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference);
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference);
	void RegisterEventCallback(EventType type, int reference);
	void UnregisterEventCallback(EventType type, int reference);
};
