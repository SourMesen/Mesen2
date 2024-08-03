#include "pch.h"
#include <algorithm>
#include <regex>
#include "Lua/lua.hpp"
#include "Lua/luasocket.hpp"
#include "Debugger/ScriptingContext.h"
#include "Debugger/LuaApi.h"
#include "Debugger/LuaCallHelper.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/ScriptManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/EventType.h"
#include "Shared/SaveStateManager.h"
#include "Utilities/magic_enum.hpp"
#include "Utilities/StringUtilities.h"

ScriptingContext* ScriptingContext::_context = nullptr;

ScriptingContext::ScriptingContext(Debugger *debugger)
{
	_debugger = debugger;
	_settings = debugger->GetEmulator()->GetSettings();
	_defaultCpuType = debugger->GetEmulator()->GetCpuTypes()[0];
	_defaultMemType = DebugUtilities::GetCpuMemoryType(_defaultCpuType);
}

ScriptingContext::~ScriptingContext()
{
	if(_lua) {
		//Cleanup all references, this is required to prevent crashes that can occur when calling lua_close
		std::unordered_set<int> references;
		for(int i = (int)CallbackType::Read; i <= (int)CallbackType::Exec; i++) {
			for(MemoryCallback& callback : _callbacks[i]) {
				references.emplace(callback.Reference);
			}
		}

		for(auto& entry : magic_enum::enum_entries<EventType>()) {
			for(int& ref : _eventCallbacks[(int)entry.first]) {
				references.emplace(ref);
			}
		}

		for(const int& ref : references) {
			luaL_unref(_lua, LUA_REGISTRYINDEX, ref);
		}

		lua_close(_lua);
		_lua = nullptr;
	}
}

bool ScriptingContext::LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger)
{
	_scriptName = scriptName;

	int iErr = 0;
	_lua = luaL_newstate();

	_context = this;
	LuaApi::SetContext(this);

	EmuSettings* settings = debugger->GetEmulator()->GetSettings();
	bool allowIoOsAccess = settings->GetDebugConfig().ScriptAllowIoOsAccess;
	LuaOpenLibs(_lua, allowIoOsAccess);

	//Prevent lua code from loading any files
	SANDBOX_ALLOW_LOADFILE = allowIoOsAccess ? 1 : 0;

	//Load LuaSocket into Lua core
	if(allowIoOsAccess && settings->GetDebugConfig().ScriptAllowNetworkAccess) {
		lua_getglobal(_lua, "package");
		lua_getfield(_lua, -1, "preload");
		lua_pushcfunction(_lua, luaopen_socket_core);
		lua_setfield(_lua, -2, "socket.core");
		lua_pushcfunction(_lua, luaopen_mime_core);
		lua_setfield(_lua, -2, "mime.core");
		lua_pop(_lua, 2);
	}

	if(allowIoOsAccess) {
		//Escape backslashes
		std::regex r("\\\\");
		path = std::regex_replace(path, r, "\\\\");

		//Add path for the current Lua script to package.path to allow
		//using require() without specifying an absolute path, etc.
		string cmd = "package.path = package.path .. ';" + path + "?.lua'";
		luaL_dostring(_lua, cmd.c_str());
	}

	luaL_requiref(_lua, "emu", LuaApi::GetLibrary, 1);
	Log("Loading script...");
	if((iErr = luaL_loadbufferx(_lua, scriptContent.c_str(), scriptContent.size(), ("@" + scriptName).c_str(), nullptr)) == 0) {
		_timer.Reset();
		lua_setwatchdogtimer(_lua, ScriptingContext::ExecutionCountHook, 1000);
		if((iErr = lua_pcall(_lua, 0, LUA_MULTRET, 0)) == 0) {
			//Script loaded properly
			Log("Script loaded successfully.");
			_initDone = true;
			return true;
		}
	}

	if(lua_isstring(_lua, -1)) {
		ProcessLuaError();
	}
	return false;
}

void ScriptingContext::ProcessLuaError()
{
	string errorMsg = lua_tostring(_lua, -1);
	if(StringUtilities::Contains(errorMsg, "attempt to call a nil value (global 'require')") || StringUtilities::Contains(errorMsg, "attempt to index a nil value (global 'os')") || StringUtilities::Contains(errorMsg, "attempt to index a nil value (global 'io')")) {
		Log("I/O and OS libraries are disabled by default for security.\nYou can enable them here:\nScript->Settings->Script Window->Restrictions->Allow access to I/O and OS functions.");
	} else if(StringUtilities::Contains(errorMsg, "module 'socket.core' not found")) {
		Log("Lua sockets are disabled by default for security.\nYou can enable them here:\nScript->Settings->Script Window->Restrictions->Allow network access.");
	} else {
		Log(errorMsg);
	}
}

void ScriptingContext::ExecutionCountHook(lua_State* lua)
{
	uint32_t timeout = _context->_settings->GetDebugConfig().ScriptTimeout;
	if(_context->_timer.GetElapsedMS() > timeout * 1000) {
		luaL_error(lua, (std::string("Maximum execution time (") + std::to_string(timeout) + " seconds) exceeded.").c_str());
	}
	lua_setwatchdogtimer(lua, ScriptingContext::ExecutionCountHook, 1000);
}

void ScriptingContext::LuaOpenLibs(lua_State* L, bool allowIoOsAccess)
{
	constexpr luaL_Reg loadedlibs[] = {
	  {"_G", luaopen_base},
	  {LUA_LOADLIBNAME, luaopen_package},
	  {LUA_COLIBNAME, luaopen_coroutine},
	  {LUA_TABLIBNAME, luaopen_table},
	  {LUA_IOLIBNAME, luaopen_io},
	  {LUA_OSLIBNAME, luaopen_os},
	  {LUA_STRLIBNAME, luaopen_string},
	  {LUA_MATHLIBNAME, luaopen_math},
	  {LUA_UTF8LIBNAME, luaopen_utf8},
	  {LUA_DBLIBNAME, luaopen_debug},
	  {NULL, NULL}
	};

	const luaL_Reg* lib;
	/* "require" functions from 'loadedlibs' and set results to global table */
	for(lib = loadedlibs; lib->func; lib++) {
		if(!allowIoOsAccess) {
			//Skip loading IO, OS and Package lib when sandboxed
			if(strcmp(lib->name, LUA_IOLIBNAME) == 0 || strcmp(lib->name, LUA_OSLIBNAME) == 0 || strcmp(lib->name, LUA_LOADLIBNAME) == 0) {
				continue;
			}
		}
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);  /* remove lib */
	}
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

template<typename T>
void ScriptingContext::CallMemoryCallback(AddressInfo relAddr, T &value, CallbackType type, CpuType cpuType)
{
	_allowSaveState = type == CallbackType::Exec && cpuType == _defaultCpuType;
	InternalCallMemoryCallback(relAddr, value, type, cpuType);
	_allowSaveState = false;
}

bool ScriptingContext::CheckInitDone()
{
	return _initDone;
}

bool ScriptingContext::IsSaveStateAllowed()
{
	return _allowSaveState;
}

void ScriptingContext::RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference)
{
	if(endAddr < startAddr) {
		return;
	}

	MemoryCallback callback;
	callback.StartAddress = (uint32_t)startAddr;
	callback.EndAddress = (uint32_t)endAddr;
	callback.Reference = reference;
	callback.Cpu = cpuType;
	callback.MemType = memType;

	if(DebugUtilities::IsPpuMemory(memType)) {
		_debugger->GetScriptManager()->EnablePpuMemoryCallbacks();
	} else {
		_debugger->GetScriptManager()->EnableCpuMemoryCallbacks();
	}

	_callbacks[(int)type].push_back(callback);
}

void ScriptingContext::RefreshMemoryCallbackFlags()
{
	for(int i = (int)CallbackType::Read; i <= (int)CallbackType::Exec; i++) {
		for(size_t j = 0, len = _callbacks[i].size(); j < len; j++) {
			if(DebugUtilities::IsPpuMemory(_callbacks[i][j].MemType)) {
				_debugger->GetScriptManager()->EnablePpuMemoryCallbacks();
			} else {
				_debugger->GetScriptManager()->EnableCpuMemoryCallbacks();
			}
		}
	}
}

void ScriptingContext::UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference)
{
	if(endAddr < startAddr) {
		return;
	}

	for(size_t i = 0; i < _callbacks[(int)type].size(); i++) {
		MemoryCallback &callback = _callbacks[(int)type][i];
		bool isMatch = (
			callback.Reference == reference &&
			callback.Cpu == cpuType &&
			callback.MemType == memType &&
			(int)callback.StartAddress == startAddr &&
			(int)callback.EndAddress == endAddr
		);

		if(isMatch) {
			_callbacks[(int)type].erase(_callbacks[(int)type].begin() + i);
			break;
		}
	}

	luaL_unref(_lua, LUA_REGISTRYINDEX, reference);
}

void ScriptingContext::RegisterEventCallback(EventType type, int reference)
{
	_eventCallbacks[(int)type].push_back(reference);
}

void ScriptingContext::UnregisterEventCallback(EventType type, int reference)
{
	vector<int> &callbacks = _eventCallbacks[(int)type];
	callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), reference), callbacks.end());
	luaL_unref(_lua, LUA_REGISTRYINDEX, reference);
}

bool ScriptingContext::IsAddressMatch(MemoryCallback& callback, AddressInfo addr)
{
	return addr.Type == callback.MemType && addr.Address >= (int32_t)callback.StartAddress && addr.Address <= (int32_t)callback.EndAddress;
}

template<typename T>
void ScriptingContext::InternalCallMemoryCallback(AddressInfo relAddr, T& value, CallbackType type, CpuType cpuType)
{
	if(_callbacks[(int)type].empty()) {
		return;
	}

	_context = this;
	bool needTimerReset = true;
	lua_setwatchdogtimer(_lua, ScriptingContext::ExecutionCountHook, 1000);
	LuaApi::SetContext(this);
	for(MemoryCallback& callback : _callbacks[(int)type]) {
		if(callback.Cpu != cpuType) {
			continue;
		} 

		if(DebugUtilities::IsRelativeMemory(callback.MemType)) {
			if(!IsAddressMatch(callback, relAddr)) {
				continue;
			}
		} else {
			if(!IsAddressMatch(callback, _debugger->GetAbsoluteAddress(relAddr))) {
				continue;
			}
		}

		if(needTimerReset) {
			_timer.Reset();
			needTimerReset = false;
		}

		int top = lua_gettop(_lua);
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, callback.Reference);
		lua_pushinteger(_lua, relAddr.Address);
		lua_pushinteger(_lua, value);
		if(lua_pcall(_lua, 2, LUA_MULTRET, 0) != 0) {
			ProcessLuaError();
		} else {
			int returnParamCount = lua_gettop(_lua) - top;
			if(returnParamCount && lua_isinteger(_lua, -1)) {
				int newValue = (int)lua_tointeger(_lua, -1);
				value = (T)newValue;
			}
			lua_settop(_lua, top);
		}
	}
}

int ScriptingContext::CallEventCallback(EventType type, CpuType cpuType)
{
	if(_eventCallbacks[(int)type].empty()) {
		return 0;
	}

	_timer.Reset();
	_context = this;
	lua_setwatchdogtimer(_lua, ScriptingContext::ExecutionCountHook, 1000);
	LuaApi::SetContext(this);
	LuaCallHelper l(_lua);
	for(int& ref : _eventCallbacks[(int)type]) {
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, ref);
		lua_pushinteger(_lua, (int)cpuType);
		if(lua_pcall(_lua, 1, 0, 0) != 0) {
			ProcessLuaError();
		}
	}
	return l.ReturnCount();
}

template void ScriptingContext::CallMemoryCallback<uint8_t>(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType);
template void ScriptingContext::CallMemoryCallback<uint16_t>(AddressInfo relAddr, uint16_t& value, CallbackType type, CpuType cpuType);
template void ScriptingContext::CallMemoryCallback<uint32_t>(AddressInfo relAddr, uint32_t& value, CallbackType type, CpuType cpuType);
