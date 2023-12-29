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

class IScriptingContext
{
public:
	virtual ~IScriptingContext() {}

	virtual bool LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger) = 0;

	virtual void Log(string message) = 0;
	virtual string GetLog() = 0;

	virtual Debugger* GetDebugger() = 0;
	virtual string GetScriptName() = 0;

	virtual void SetDrawSurface(ScriptDrawSurface surface) = 0;
	virtual ScriptDrawSurface GetDrawSurface() = 0;

	virtual void CallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType) = 0;
	virtual void CallMemoryCallback(AddressInfo relAddr, uint16_t& value, CallbackType type, CpuType cpuType) = 0;
	virtual void CallMemoryCallback(AddressInfo relAddr, uint32_t& value, CallbackType type, CpuType cpuType) = 0;

	virtual int CallEventCallback(EventType type, CpuType cpuType) = 0;
	virtual bool CheckInitDone() = 0;
	virtual bool IsSaveStateAllowed() = 0;

	virtual CpuType GetDefaultCpuType() = 0;
	virtual MemoryType GetDefaultMemType() = 0;

	virtual void RefreshMemoryCallbackFlags() = 0;

	virtual void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference) = 0;
	virtual void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference) = 0;
	virtual void RegisterEventCallback(EventType type, int reference) = 0;
	virtual void UnregisterEventCallback(EventType type, int reference) = 0;
};

class ScriptingContext : public IScriptingContext
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
	bool LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger) override;

	void Log(string message) override;
	string GetLog() override;

	Debugger* GetDebugger() override;
	string GetScriptName() override;

	void SetDrawSurface(ScriptDrawSurface surface) override { _drawSurface = surface; }
	ScriptDrawSurface GetDrawSurface() override { return _drawSurface; }

	void CallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType) override;
	void CallMemoryCallback(AddressInfo relAddr, uint16_t& value, CallbackType type, CpuType cpuType) override;
	void CallMemoryCallback(AddressInfo relAddr, uint32_t& value, CallbackType type, CpuType cpuType) override;

	int CallEventCallback(EventType type, CpuType cpuType) override;
	bool CheckInitDone() override;
	bool IsSaveStateAllowed() override;

	CpuType GetDefaultCpuType() override { return _defaultCpuType; }
	MemoryType GetDefaultMemType() override { return _defaultMemType; }
	
	void RefreshMemoryCallbackFlags() override;

	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference) override;
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference) override;
	void RegisterEventCallback(EventType type, int reference) override;
	void UnregisterEventCallback(EventType type, int reference) override;
};

struct PythonState;

class PythonScriptingContext : public IScriptingContext
{
protected:
	string _scriptName;
	deque<string> _logRows;
	SimpleLock _logLock;

	Debugger* _debugger = nullptr;
	EmuSettings* _settings = nullptr;
	CpuType _defaultCpuType = {};
	MemoryType _defaultMemType = {};

	PythonState *_python = nullptr;

	bool _allowSaveState = true;

	ScriptDrawSurface _drawSurface = ScriptDrawSurface::ConsoleScreen;

	void LogError();
	void InitializePython();
	string ReadFileContents(const string &path);

public:
	PythonScriptingContext(Debugger* debugger);
	~PythonScriptingContext();
	bool LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger) override;

	void Log(string message) override;
	string GetLog() override;

	Debugger* GetDebugger() override;
	string GetScriptName() override;

	void SetDrawSurface(ScriptDrawSurface surface) override { _drawSurface = surface; }
	ScriptDrawSurface GetDrawSurface() override { return _drawSurface; }

	void CallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType) override;
	void CallMemoryCallback(AddressInfo relAddr, uint16_t& value, CallbackType type, CpuType cpuType) override;
	void CallMemoryCallback(AddressInfo relAddr, uint32_t& value, CallbackType type, CpuType cpuType) override;

	int CallEventCallback(EventType type, CpuType cpuType) override;
	bool CheckInitDone() override;
	bool IsSaveStateAllowed() override;

	CpuType GetDefaultCpuType() override { return _defaultCpuType; }
	MemoryType GetDefaultMemType() override { return _defaultMemType; }

	void RefreshMemoryCallbackFlags() override;

	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference) override;
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference) override;
	void RegisterEventCallback(EventType type, int reference) override;
	void UnregisterEventCallback(EventType type, int reference) override;
};
