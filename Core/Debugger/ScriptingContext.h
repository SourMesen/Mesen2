#pragma once
#include "pch.h"
#include <deque>
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"
#include "Debugger/DebugTypes.h"
#include "Shared/EventType.h"

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
	bool _allowSaveState = false;

	Debugger* _debugger = nullptr;
	CpuType _defaultCpuType = {};
	MemoryType _defaultMemType = {};

	ScriptDrawSurface _drawSurface = ScriptDrawSurface::ConsoleScreen;

	static void ExecutionCountHook(lua_State* lua);
	void LuaOpenLibs(lua_State* L, bool allowIoOsAccess);
	void ProcessLuaError();

protected:
	string _scriptName;
	bool _initDone = false;

	vector<MemoryCallback> _callbacks[3];
	vector<int> _eventCallbacks[(int)EventType::LastValue + 1];

	template<typename T> void InternalCallMemoryCallback(AddressInfo relAddr, T& value, CallbackType type, CpuType cpuType);

	bool IsAddressMatch(MemoryCallback& callback, AddressInfo addr);

public:
	ScriptingContext(Debugger* debugger);
	~ScriptingContext();
	bool LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger);

	void Log(string message);
	string GetLog();

	Debugger* GetDebugger();
	string GetScriptName();

	void SetDrawSurface(ScriptDrawSurface surface) { _drawSurface = surface; }
	ScriptDrawSurface GetDrawSurface() { return _drawSurface; }

	template<typename T> void CallMemoryCallback(AddressInfo relAddr, T& value, CallbackType type, CpuType cpuType);
	int CallEventCallback(EventType type, CpuType cpuType);
	bool CheckInitDone();
	bool IsSaveStateAllowed();

	CpuType GetDefaultCpuType() { return _defaultCpuType; }
	MemoryType GetDefaultMemType() { return _defaultMemType; }
	
	void RefreshMemoryCallbackFlags();

	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference);
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference);
	void RegisterEventCallback(EventType type, int reference);
	void UnregisterEventCallback(EventType type, int reference);
};
