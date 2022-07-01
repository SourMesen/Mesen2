#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "ScriptingContext.h"
#include "EventType.h"
#include "Utilities/Timer.h"

struct lua_State;
struct lua_Debug;
class Debugger;
class EmuSettings;

class LuaScriptingContext : public ScriptingContext
{
private:
	static LuaScriptingContext* _context;
	lua_State* _lua = nullptr;
	Timer _timer;
	EmuSettings* _settings = nullptr;

	static void ExecutionCountHook(lua_State* lua, lua_Debug* ar);

	void LuaOpenLibs(lua_State* L, bool allowIoOsAccess);

protected:
	void InternalCallMemoryCallback(uint32_t addr, uint8_t &value, CallbackType type, CpuType cpuType) override;
	int InternalCallEventCallback(EventType type) override;

public:
	LuaScriptingContext(Debugger* debugger);
	virtual ~LuaScriptingContext();

	bool LoadScript(string scriptName, string scriptContent, Debugger* debugger) override;
	
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, CpuType cpuType, int reference) override;
	void UnregisterEventCallback(EventType type, int reference) override;
};
