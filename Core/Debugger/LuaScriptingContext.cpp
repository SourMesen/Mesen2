#include "stdafx.h"

#ifndef LIBRETRO
#include "../Lua/lua.hpp"
#include "../Lua/luasocket.hpp"
#include "LuaScriptingContext.h"
#include "LuaApi.h"
#include "LuaCallHelper.h"
#include "DebugTypes.h"
#include "Debugger.h"
#include "EventType.h"

LuaScriptingContext* LuaScriptingContext::_context = nullptr;
uint32_t LuaScriptingContext::_timeout = 1000;

LuaScriptingContext::LuaScriptingContext(Debugger* debugger) : ScriptingContext(debugger)
{
}

LuaScriptingContext::~LuaScriptingContext()
{
	if(_lua) {
		//Cleanup all references, this is required to prevent crashes that can occur when calling lua_close
		std::unordered_set<int> references;
		for(int i = (int)CallbackType::CpuRead; i <= (int)CallbackType::CpuExec; i++) {
			for(MemoryCallback &callback : _callbacks[i]) {
				references.emplace(callback.Reference);
			}
		}

		for(int i = (int)EventType::Reset; i < (int)EventType::EventTypeSize; i++) {
			for(int &ref : _eventCallbacks[i]) {
				references.emplace(ref);
			}
		}

		for(const int &ref : references) {
			luaL_unref(_lua, LUA_REGISTRYINDEX, ref);
		}

		lua_close(_lua);
		_lua = nullptr;
	}
}

void LuaScriptingContext::SetScriptTimeout(uint32_t timeout)
{
	_timeout = timeout;
}

void LuaScriptingContext::ExecutionCountHook(lua_State *lua, lua_Debug *ar)
{
	if(_context->_timer.GetElapsedMS() > _timeout) {
		luaL_error(lua, (std::string("Maximum execution time (") + std::to_string(_timeout) + " ms) exceeded.").c_str());
	}
}

bool LuaScriptingContext::LoadScript(string scriptName, string scriptContent, Debugger* debugger)
{
	_scriptName = scriptName;

	int iErr = 0;
	_lua = luaL_newstate();
	
	_context = this;
	LuaApi::SetContext(this);

	luaL_openlibs(_lua);

	//Load LuaSocket into Lua core
	lua_getglobal(_lua, "package");
	lua_getfield(_lua, -1, "preload");
	lua_pushcfunction(_lua, luaopen_socket_core);
	lua_setfield(_lua, -2, "socket.core");
	lua_pushcfunction(_lua, luaopen_mime_core);
	lua_setfield(_lua, -2, "mime.core");
	lua_pop(_lua, 2);

	luaL_requiref(_lua, "emu", LuaApi::GetLibrary, 1);
	Log("Loading script...");
	if((iErr = luaL_loadbufferx(_lua, scriptContent.c_str(), scriptContent.size(), ("@" + scriptName).c_str(), nullptr)) == 0) {
		_timer.Reset();
		lua_sethook(_lua, LuaScriptingContext::ExecutionCountHook, LUA_MASKCOUNT, 1000);
		if((iErr = lua_pcall(_lua, 0, LUA_MULTRET, 0)) == 0) {
			//Script loaded properly
			Log("Script loaded successfully.");
			_initDone = true;
			return true;
		}
	}

	if(lua_isstring(_lua, -1)) {
		Log(lua_tostring(_lua, -1));
	}
	return false;
}

void LuaScriptingContext::UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, CpuType cpuType, int reference)
{
	ScriptingContext::UnregisterMemoryCallback(type, startAddr, endAddr, cpuType, reference);
	luaL_unref(_lua, LUA_REGISTRYINDEX, reference);
}

void LuaScriptingContext::UnregisterEventCallback(EventType type, int reference)
{
	ScriptingContext::UnregisterEventCallback(type, reference);
	luaL_unref(_lua, LUA_REGISTRYINDEX, reference);
}

void LuaScriptingContext::InternalCallMemoryCallback(uint32_t addr, uint8_t &value, CallbackType type, CpuType cpuType)
{
	if(_callbacks[(int)type].empty()) {
		return;
	}

	_timer.Reset();
	_context = this;
	lua_sethook(_lua, LuaScriptingContext::ExecutionCountHook, LUA_MASKCOUNT, 1000); 
	LuaApi::SetContext(this);
	for(MemoryCallback &callback: _callbacks[(int)type]) {
		if(callback.Type != cpuType || addr < callback.StartAddress || addr > callback.EndAddress) {
			continue;
		}

		int top = lua_gettop(_lua);
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, callback.Reference);
		lua_pushinteger(_lua, addr);
		lua_pushinteger(_lua, value);
		if(lua_pcall(_lua, 2, LUA_MULTRET, 0) != 0) {
			Log(lua_tostring(_lua, -1));
		} else {
			int returnParamCount = lua_gettop(_lua) - top;
			if(returnParamCount && lua_isinteger(_lua, -1)) {
				int newValue = (int)lua_tointeger(_lua, -1);
				value = (uint8_t)newValue;
			}
			lua_settop(_lua, top);
		}
	}
}

int LuaScriptingContext::InternalCallEventCallback(EventType type)
{
	if(_eventCallbacks[(int)type].empty()) {
		return 0;
	}

	_timer.Reset();
	_context = this;
	lua_sethook(_lua, LuaScriptingContext::ExecutionCountHook, LUA_MASKCOUNT, 1000); 
	LuaApi::SetContext(this);
	LuaCallHelper l(_lua);
	for(int &ref : _eventCallbacks[(int)type]) {
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, ref);
		if(lua_pcall(_lua, 0, 0, 0) != 0) {
			Log(lua_tostring(_lua, -1));
		}
	}
	return l.ReturnCount();
}
#endif