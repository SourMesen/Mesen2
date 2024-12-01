#include "pch.h"
#include "LuaApi.h"
#include "Lua/lua.hpp"
#include "Debugger/LuaCallHelper.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/ScriptingContext.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/CdlManager.h"
#include "Debugger/LabelManager.h"
#include "Shared/SystemActionManager.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/MessageManager.h"
#include "Shared/CheatManager.h"
#include "Shared/RewindManager.h"
#include "Shared/SaveStateManager.h"
#include "Shared/Emulator.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/Video/DrawScreenBufferCommand.h"
#include "Shared/Video/DrawStringCommand.h"
#include "Shared/KeyManager.h"
#include "Shared/Interfaces/IConsole.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/ControllerHub.h"
#include "Shared/BaseControlManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/magic_enum.hpp"
#include "Shared/MemoryOperationType.h"

#ifdef _MSC_VER
//TODO MSVC seems to trigger this by mistake because of the macros?
#pragma warning ( disable : 4702 ) //unreachable code
#endif

#define lua_pushintvalue(name, value) lua_pushliteral(lua, #name); lua_pushinteger(lua, (int)value); lua_settable(lua, -3);
#define lua_pushdoublevalue(name, value) lua_pushliteral(lua, #name); lua_pushnumber(lua, (double)value); lua_settable(lua, -3);
#define lua_pushboolvalue(name, value) lua_pushliteral(lua, #name); lua_pushboolean(lua, (int)value); lua_settable(lua, -3);
#define lua_pushstringvalue(name, value) lua_pushliteral(lua, #name); lua_pushstring(lua, value.c_str()); lua_settable(lua, -3);
#define lua_pusharrayvalue(index, value) lua_pushinteger(lua, index); lua_pushinteger(lua, value); lua_settable(lua, -3);

#define lua_starttable(name) lua_pushliteral(lua, #name); lua_newtable(lua);
#define lua_endtable() lua_settable(lua, -3);
#define lua_readint(name, dest) lua_getfield(lua, -1, #name); dest = l.ReadInteger();
#define lua_readbool(name, dest) lua_getfield(lua, -1, #name); dest = l.ReadBool();
#define error(text) luaL_error(lua, text); return 0;
#define errorCond(cond, text) if(cond) { luaL_error(lua, text); return 0; }
#define checkEnum(enumType, enumValue, text) if(!magic_enum::enum_contains<enumType>(enumValue)) { luaL_error(lua, text); return 0; }

#define checkparams() if(!l.CheckParamCount()) { return 0; }
#define checkminparams(x) if(!l.CheckParamCount(x)) { return 0; }
#define checkinitdone() if(!_context->CheckInitDone()) { error("This function cannot be called outside a callback"); }
#define checksavestateconditions() if(!_context->IsSaveStateAllowed()) { error("This function must be called inside an exec memory operation callback for the main CPU"); }

Debugger* LuaApi::_debugger = nullptr;
Emulator* LuaApi::_emu = nullptr;
MemoryDumper* LuaApi::_memoryDumper = nullptr;
ScriptingContext* LuaApi::_context = nullptr;

enum class AccessCounterType
{
	ReadCount,
	WriteCount,
	ExecCount,
	LastReadClock,
	LastWriteClock,
	LastExecClock
};

void LuaApi::SetContext(ScriptingContext* context)
{
	_context = context;
	_debugger = _context->GetDebugger();
	_memoryDumper = _debugger->GetMemoryDumper();
	_emu = _debugger->GetEmulator();
}

void LuaApi::LuaPushIntValue(lua_State* lua, string name, int value)
{
	lua_pushstring(lua, name.c_str());
	lua_pushinteger(lua, value);
	lua_settable(lua, -3);
}

int LuaApi::GetLibrary(lua_State *lua)
{
	static const luaL_Reg apilib[] = {
		{ "getMemorySize", LuaApi::GetMemorySize },

		{ "read", LuaApi::ReadMemory },
		{ "write", LuaApi::WriteMemory },
		{ "read16", LuaApi::ReadMemory16 },
		{ "write16", LuaApi::WriteMemory16 },
		{ "read32", LuaApi::ReadMemory32 },
		{ "write32", LuaApi::WriteMemory32 },

		{ "readWord", LuaApi::ReadMemory16 }, //for backward compatibility
		{ "writeWord", LuaApi::WriteMemory16 }, //for backward compatibility
		
		{ "convertAddress", LuaApi::ConvertAddress },
		{ "getLabelAddress", LuaApi::GetLabelAddress },

		{ "addMemoryCallback", LuaApi::RegisterMemoryCallback },
		{ "removeMemoryCallback", LuaApi::UnregisterMemoryCallback },
		{ "addEventCallback", LuaApi::RegisterEventCallback },
		{ "removeEventCallback", LuaApi::UnregisterEventCallback },

		{ "measureString", LuaApi::MeasureString },
		{ "drawString", LuaApi::DrawString },

		{ "drawPixel", LuaApi::DrawPixel },
		{ "drawLine", LuaApi::DrawLine },
		{ "drawRectangle", LuaApi::DrawRectangle },
		{ "clearScreen", LuaApi::ClearScreen },

		{ "getScreenSize", LuaApi::GetScreenSize },
		{ "getDrawSurfaceSize", LuaApi::GetDrawSurfaceSize },

		{ "getScreenBuffer", LuaApi::GetScreenBuffer },
		{ "setScreenBuffer", LuaApi::SetScreenBuffer },
		{ "getPixel", LuaApi::GetPixel },

		{ "getMouseState", LuaApi::GetMouseState },
		{ "log", LuaApi::Log },
		{ "displayMessage", LuaApi::DisplayMessage },

		{ "reset", LuaApi::Reset },
		{ "stop", LuaApi::Stop },
		{ "breakExecution", LuaApi::BreakExecution },
		{ "resume", LuaApi::Resume },
		{ "step", LuaApi::Step },
		{ "rewind", LuaApi::Rewind },

		{ "takeScreenshot", LuaApi::TakeScreenshot },

		{ "isKeyPressed", LuaApi::IsKeyPressed },
		{ "getInput", LuaApi::GetInput },
		{ "setInput", LuaApi::SetInput },

		{ "getAccessCounters", LuaApi::GetAccessCounters },
		{ "resetAccessCounters", LuaApi::ResetAccessCounters },

		{ "getCdlData", LuaApi::GetCdlData},

		{ "addCheat", LuaApi::AddCheat },
		{ "clearCheats", LuaApi::ClearCheats },

		{ "createSavestate", LuaApi::CreateSavestate },
		{ "loadSavestate", LuaApi::LoadSavestate },

		{ "getState", LuaApi::GetState },
		{ "setState", LuaApi::SetState },

		{ "selectDrawSurface", LuaApi::SelectDrawSurface },

		{ "getScriptDataFolder", LuaApi::GetScriptDataFolder },
		{ "getRomInfo", LuaApi::GetRomInfo },
		{ "getLogWindowLog", LuaApi::GetLogWindowLog },
		{ NULL,NULL }
	};

	luaL_newlib(lua, apilib);

	//Expose MemoryType enum as "emu.memType"
	lua_pushliteral(lua, "memType");
	lua_newtable(lua);
	for(auto& entry : magic_enum::enum_entries<MemoryType>()) {
		string name = string(entry.second);
		name[0] = ::tolower(name[0]);
		if(DebugUtilities::IsRelativeMemory(entry.first)) {
			string debugName = name.substr(0, name.size() - 6) + "Debug";
			LuaPushIntValue(lua, debugName, (int)entry.first | 0x100);
		}
		LuaPushIntValue(lua, name, (int)entry.first);
	}
	lua_settable(lua, -3);

	GenerateEnumDefinition<CallbackType>(lua, "callbackType");
	GenerateEnumDefinition<CheatType>(lua, "cheatType");
	GenerateEnumDefinition<AccessCounterType>(lua, "counterType");
	GenerateEnumDefinition<CpuType>(lua, "cpuType");
	GenerateEnumDefinition<ScriptDrawSurface>(lua, "drawSurface");
	GenerateEnumDefinition<EventType>(lua, "eventType", { EventType::LastValue });
	GenerateEnumDefinition<StepType>(lua, "stepType", { StepType::StepBack });

	return 1;
}

template<typename T>
void LuaApi::GenerateEnumDefinition(lua_State* lua, string enumName, unordered_set<T> excludedValues)
{
	lua_pushstring(lua, enumName.c_str());
	lua_newtable(lua);
	for(auto& entry : magic_enum::enum_entries<T>()) {
		if(excludedValues.find(entry.first) == excludedValues.end()) {
			string name = string(entry.second);
			name[0] = ::tolower(name[0]);
			LuaPushIntValue(lua, name, (int)entry.first);
		}
	}
	lua_settable(lua, -3);
}

DebugHud* LuaApi::GetHud()
{
	if(_context->GetDrawSurface() == ScriptDrawSurface::ConsoleScreen) {
		return _emu->GetDebugHud();
	} else {
		return _emu->GetScriptHud();
	}
}

int LuaApi::SelectDrawSurface(lua_State* lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(2);
	int surfaceScale = l.ReadInteger(-1);
	ScriptDrawSurface surface = (ScriptDrawSurface)l.ReadInteger();
	checkminparams(1);
	if(surfaceScale != -1) {
		errorCond(surface == ScriptDrawSurface::ConsoleScreen && surfaceScale != 1, "scale for the console screen must be 1");
		errorCond(surface == ScriptDrawSurface::ScriptHud && (surfaceScale < 1 || surfaceScale > 4), "scale for the script HUD must be between 1 and 4");
	}
	checkEnum(ScriptDrawSurface, surface, "invalid draw surface value");
	_context->SetDrawSurface(surface);

	if(surfaceScale != -1 && surface == ScriptDrawSurface::ScriptHud) {
		_emu->GetVideoRenderer()->SetScriptHudScale(surfaceScale);
	}
	return 0;
}

int LuaApi::GetMemorySize(lua_State* lua)
{
	LuaCallHelper l(lua);
	MemoryType memType = (MemoryType)l.ReadInteger();
	checkEnum(MemoryType, memType, "invalid memory type");
	l.Return(_memoryDumper->GetMemorySize(memType));
	return l.ReturnCount();
}

int LuaApi::ReadMemory(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(3);
	bool returnSignedValue = l.ReadBool();
	int type = l.ReadInteger();
	bool disableSideEffects = (type & 0x100) == 0x100;
	MemoryType memType = (MemoryType)(type & 0xFF);
	int address = l.ReadInteger();
	checkminparams(2);
	errorCond(address < 0, "address must be >= 0");
	checkEnum(MemoryType, memType, "invalid memory type");
	uint8_t value = _memoryDumper->GetMemoryValue(memType, address, disableSideEffects);
	l.Return(returnSignedValue ? (int8_t)value : value);
	return l.ReturnCount();
}

int LuaApi::WriteMemory(lua_State *lua)
{
	LuaCallHelper l(lua);
	int type = l.ReadInteger();
	bool disableSideEffects = (type & 0x100) == 0x100;
	MemoryType memType = (MemoryType)(type & 0xFF);
	int value = l.ReadInteger();
	int address = l.ReadInteger();
	checkparams();
	errorCond(value > 255 || value < -128, "value out of range");
	errorCond(address < 0, "address must be >= 0");
	checkEnum(MemoryType, memType, "invalid memory type");
	_memoryDumper->SetMemoryValue(memType, address, value, disableSideEffects);
	return l.ReturnCount();
}

int LuaApi::ReadMemory16(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(3);
	bool returnSignedValue = l.ReadBool();
	int type = l.ReadInteger();
	bool disableSideEffects = (type & 0x100) == 0x100;
	MemoryType memType = (MemoryType)(type & 0xFF);
	int address = l.ReadInteger();
	checkminparams(2);
	errorCond(address < 0, "address must be >= 0");
	checkEnum(MemoryType, memType, "invalid memory type");
	uint16_t value = _memoryDumper->GetMemoryValue16(memType, address, disableSideEffects);
	l.Return(returnSignedValue ? (int16_t)value : value);
	return l.ReturnCount();
}

int LuaApi::ReadMemory32(lua_State* lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(3);
	bool returnSignedValue = l.ReadBool();
	int type = l.ReadInteger();
	bool disableSideEffects = (type & 0x100) == 0x100;
	MemoryType memType = (MemoryType)(type & 0xFF);
	int address = l.ReadInteger();
	checkminparams(2);
	errorCond(address < 0, "address must be >= 0");
	checkEnum(MemoryType, memType, "invalid memory type");
	uint32_t value = _memoryDumper->GetMemoryValue32(memType, address, disableSideEffects);
	l.Return(returnSignedValue ? (int32_t)value : value);
	return l.ReturnCount();
}

int LuaApi::WriteMemory16(lua_State *lua)
{
	LuaCallHelper l(lua);
	int type = l.ReadInteger();
	bool disableSideEffects = (type & 0x100) == 0x100;
	MemoryType memType = (MemoryType)(type & 0xFF);
	int value = l.ReadInteger();
	int address = l.ReadInteger();
	checkparams();
	errorCond(value > 65535 || value < -32768, "value out of range");
	errorCond(address < 0, "address must be >= 0");
	checkEnum(MemoryType, memType, "invalid memory type");
	_memoryDumper->SetMemoryValue16(memType, address, value, disableSideEffects);
	return l.ReturnCount();
}

int LuaApi::WriteMemory32(lua_State* lua)
{
	LuaCallHelper l(lua);
	int type = l.ReadInteger();
	bool disableSideEffects = (type & 0x100) == 0x100;
	MemoryType memType = (MemoryType)(type & 0xFF);
	int value = l.ReadInteger();
	int address = l.ReadInteger();
	checkparams();
	errorCond(address < 0, "address must be >= 0");
	checkEnum(MemoryType, memType, "invalid memory type");
	_memoryDumper->SetMemoryValue32(memType, address, value, disableSideEffects);
	return l.ReturnCount();
}

int LuaApi::ConvertAddress(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(3);
	CpuType cpuType = (CpuType)l.ReadInteger((uint32_t)_context->GetDefaultCpuType());
	MemoryType memType = (MemoryType)l.ReadInteger((uint32_t)_context->GetDefaultMemType());
	uint32_t address = l.ReadInteger();
	checkminparams(1);

	checkEnum(CpuType, cpuType, "invalid cpu type");
	checkEnum(MemoryType, memType, "invalid memory type");
	errorCond(address < 0 || address >= _memoryDumper->GetMemorySize(memType), "address is out of range");

	AddressInfo src { (int32_t)address, memType };
	AddressInfo result;
	if(DebugUtilities::IsRelativeMemory(memType)) {
		result = _debugger->GetAbsoluteAddress(src);
	} else {
		result = _debugger->GetRelativeAddress(src, cpuType);
	}

	if(result.Address < 0) {
		lua_pushnil(lua);
	} else {
		lua_newtable(lua);
		lua_pushintvalue(address, result.Address);
		lua_pushintvalue(memType, result.Type);
	}
	return 1;
}

int LuaApi::GetLabelAddress(lua_State* lua)
{
	LuaCallHelper l(lua);
	string label = l.ReadString();
	checkparams();
	errorCond(label.length() == 0, "label cannot be empty");

	LabelManager* labelManager = _debugger->GetLabelManager();
	AddressInfo addr = labelManager->GetLabelAbsoluteAddress(label);
	if(addr.Address < 0) {
		//Check to see if the label is a multi-byte label instead
		string mbLabel = label + "+0";
		addr = labelManager->GetLabelAbsoluteAddress(mbLabel);
	}

	if(addr.Address < 0) {
		lua_pushnil(lua);
	} else {
		lua_newtable(lua);
		lua_pushintvalue(address, addr.Address);
		lua_pushintvalue(memType, addr.Type);
	}
	return 1;
}

int LuaApi::RegisterMemoryCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(6);

	MemoryType memType = (MemoryType)l.ReadInteger((int)_context->GetDefaultMemType());
	CpuType cpuType = (CpuType)l.ReadInteger((int)_context->GetDefaultCpuType());
	int32_t endAddr = l.ReadInteger(-1);
	uint32_t startAddr = l.ReadInteger();
	CallbackType callbackType = (CallbackType)l.ReadInteger();
	int reference = l.GetReference();

	checkminparams(3);

	if(endAddr == -1) {
		endAddr = startAddr;
	}

	errorCond(startAddr < 0, "start address must be >= 0");
	errorCond(startAddr > (uint32_t)endAddr, "start address must be <= end address");
	checkEnum(CallbackType, callbackType, "invalid callback type");
	checkEnum(MemoryType, memType, "invalid memory type");
	checkEnum(CpuType, cpuType, "invalid cpu type");
	errorCond(reference == LUA_NOREF, "callback function could not be found");

	_context->RegisterMemoryCallback(callbackType, startAddr, endAddr, memType, cpuType, reference);
	_context->Log("Registered memory callback from $" + HexUtilities::ToHex((uint32_t)startAddr) + " to $" + HexUtilities::ToHex((uint32_t)endAddr));
	l.Return(reference);
	return l.ReturnCount();
}

int LuaApi::UnregisterMemoryCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(6);
	
	MemoryType memType = (MemoryType)l.ReadInteger((int)_context->GetDefaultMemType());
	CpuType cpuType = (CpuType)l.ReadInteger((int)_context->GetDefaultCpuType());
	int endAddr = l.ReadInteger(-1);
	int startAddr = l.ReadInteger();
	CallbackType callbackType = (CallbackType)l.ReadInteger();
	int reference = l.ReadInteger();

	checkminparams(3);

	if(endAddr == -1) {
		endAddr = startAddr;
	}

	errorCond(startAddr < 0, "start address must be >= 0");
	errorCond(startAddr > endAddr, "start address must be <= end address");
	checkEnum(CallbackType, callbackType, "invalid callback type");
	checkEnum(MemoryType, memType, "invalid memory type");
	checkEnum(CpuType, cpuType, "invalid cpu type");
	errorCond(reference == LUA_NOREF, "callback function could not be found");

	_context->UnregisterMemoryCallback(callbackType, startAddr, endAddr, memType, cpuType, reference);
	return l.ReturnCount();
}

int LuaApi::RegisterEventCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	EventType type = (EventType)l.ReadInteger();
	int reference = l.GetReference();
	checkparams();
	checkEnum(EventType, type, "invalid event type");
	errorCond(reference == LUA_NOREF, "callback function could not be found");
	_context->RegisterEventCallback(type, reference);
	l.Return(reference);
	return l.ReturnCount();
}

int LuaApi::UnregisterEventCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	EventType type = (EventType)l.ReadInteger();
	int reference = l.ReadInteger();
	checkparams();

	checkEnum(EventType, type, "invalid event type");
	errorCond(reference == LUA_NOREF, "callback function could not be found");
	_context->UnregisterEventCallback(type, reference);
	return l.ReturnCount();
}

int LuaApi::MeasureString(lua_State* lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(2);
	int maxWidth = l.ReadInteger(0);
	string text = l.ReadString();
	checkminparams(1);

	TextSize size = DrawStringCommand::MeasureString(text, maxWidth);
	lua_newtable(lua);
	lua_pushintvalue(width, size.X);
	lua_pushintvalue(height, size.Y);
	return 1;
}

int LuaApi::DrawString(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(8);
	int displayDelay = l.ReadInteger(0);
	int frameCount = l.ReadInteger(1);
	int maxWidth = l.ReadInteger(0);
	int backColor = l.ReadInteger(0);
	int color = l.ReadInteger(0xFFFFFF);
	string text = l.ReadString();
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkminparams(3);

	int startFrame = _emu->GetFrameCount() + displayDelay;
	GetHud()->DrawString(x, y, text, color, backColor, frameCount, startFrame, maxWidth);

	return l.ReturnCount();
}

int LuaApi::DrawLine(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(7);
	int displayDelay = l.ReadInteger(0);
	int frameCount = l.ReadInteger(1);
	int color = l.ReadInteger(0xFFFFFF);
	int y2 = l.ReadInteger();
	int x2 = l.ReadInteger();
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkminparams(4);

	int startFrame = _emu->GetFrameCount() + displayDelay;
	GetHud()->DrawLine(x, y, x2, y2, color, frameCount, startFrame);

	return l.ReturnCount();
}

int LuaApi::DrawPixel(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(5);
	int displayDelay = l.ReadInteger(0);
	int frameCount = l.ReadInteger(1);
	int color = l.ReadInteger();
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkminparams(3);

	int startFrame = _emu->GetFrameCount() + displayDelay;
	GetHud()->DrawPixel(x, y, color, frameCount, startFrame);

	return l.ReturnCount();
}

int LuaApi::DrawRectangle(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(8);
	int displayDelay = l.ReadInteger(0);
	int frameCount = l.ReadInteger(1);
	bool fill = l.ReadBool(false);
	int color = l.ReadInteger(0xFFFFFF);
	int height = l.ReadInteger();
	int width = l.ReadInteger();
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkminparams(4);

	int startFrame = _emu->GetFrameCount() + displayDelay;
	GetHud()->DrawRectangle(x, y, width, height, color, fill, frameCount, startFrame);

	return l.ReturnCount();
}

int LuaApi::ClearScreen(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();

	_emu->GetDebugHud()->ClearScreen();
	_emu->GetScriptHud()->ClearScreen();
	return l.ReturnCount();
}

FrameInfo LuaApi::InternalGetScreenSize()
{
	PpuFrameInfo frame = _emu->GetPpuFrame();
	FrameInfo frameSize;
	frameSize.Height = frame.Height;
	frameSize.Width = frame.Width;

	unique_ptr<BaseVideoFilter> filter(_emu->GetVideoFilter());
	filter->SetBaseFrameInfo(frameSize);
	filter->SetOverscan({});
	return filter->GetFrameInfo((uint16_t*)frame.FrameBuffer, false);
}

int LuaApi::GetScreenSize(lua_State* lua)
{
	LuaCallHelper l(lua);

	FrameInfo size = InternalGetScreenSize();
	lua_newtable(lua);
	lua_pushintvalue(width, size.Width);
	lua_pushintvalue(height, size.Height);
	return 1;
}

int LuaApi::GetDrawSurfaceSize(lua_State* lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(1);
	ScriptDrawSurface surface = (ScriptDrawSurface)l.ReadInteger((uint32_t)_context->GetDrawSurface());
	checkEnum(ScriptDrawSurface, surface, "invalid draw surface");

	FrameInfo size;
	OverscanDimensions overscan;
	if(surface == ScriptDrawSurface::ConsoleScreen) {
		size = _emu->GetVideoDecoder()->GetBaseFrameInfo(true);
		overscan = _emu->GetSettings()->GetOverscan();
	} else {
		std::tie(size, overscan) = _emu->GetVideoRenderer()->GetScriptHudSize();
	}

	lua_newtable(lua);
	lua_pushintvalue(width, size.Width + overscan.Left + overscan.Right);
	lua_pushintvalue(height, size.Height + overscan.Top + overscan.Bottom);
	lua_pushintvalue(visibleWidth, size.Width);
	lua_pushintvalue(visibleHeight, size.Height);

	lua_starttable(overscan);
	lua_pushintvalue(top, overscan.Top);
	lua_pushintvalue(bottom, overscan.Bottom);
	lua_pushintvalue(left, overscan.Left);
	lua_pushintvalue(right, overscan.Right);
	lua_endtable();

	return 1;
}

std::pair<unique_ptr<BaseVideoFilter>, FrameInfo> LuaApi::GetRenderedFrame()
{
	PpuFrameInfo frame = _emu->GetPpuFrame();
	FrameInfo frameSize;
	frameSize.Height = frame.Height;
	frameSize.Width = frame.Width;

	unique_ptr<BaseVideoFilter> filter(_emu->GetVideoFilter());
	filter->SetBaseFrameInfo(frameSize);
	frameSize = filter->SendFrame((uint16_t*)frame.FrameBuffer, _emu->GetFrameCount(), _emu->GetFrameCount() & 0x01, nullptr, false);
	return std::make_pair(std::move(filter), frameSize);
}

int LuaApi::GetScreenBuffer(lua_State *lua)
{
	LuaCallHelper l(lua);

	auto [filter, frameSize] = GetRenderedFrame();
	uint32_t* rgbBuffer = filter->GetOutputBuffer();

	lua_createtable(lua, frameSize.Height*frameSize.Width, 0);
	for(int32_t i = 0, len = frameSize.Height * frameSize.Width; i < len; i++) {
		lua_pushinteger(lua, rgbBuffer[i] & 0xFFFFFF);
		lua_rawseti(lua, -2, i + 1);
	}

	return 1;
}

int LuaApi::SetScreenBuffer(lua_State *lua)
{
	LuaCallHelper l(lua);
	
	FrameInfo size = InternalGetScreenSize();

	int startFrame = _emu->GetFrameCount();
	unique_ptr<DrawScreenBufferCommand> cmd(new DrawScreenBufferCommand(size.Width, size.Height, startFrame));

	luaL_checktype(lua, 1, LUA_TTABLE);
	for(int i = 0, len = size.Height * size.Width; i < len; i++) {
		lua_rawgeti(lua, 1, i+1);
		uint32_t color = (uint32_t)lua_tointeger(lua, -1);
		lua_pop(lua, 1);
		cmd->SetPixel(i, color ^ 0xFF000000);
	}
	
	_emu->GetDebugHud()->AddCommand(std::move(cmd));
	return l.ReturnCount();
}

int LuaApi::GetPixel(lua_State *lua)
{
	LuaCallHelper l(lua);
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkparams();

	auto [filter, frameSize] = GetRenderedFrame();
	errorCond(x < 0 || x >= (int)frameSize.Width || y < 0 || y >= (int)frameSize.Height, "invalid x,y coordinates");

	uint32_t* rgbBuffer = filter->GetOutputBuffer();
	l.Return(rgbBuffer[y * frameSize.Width + x]);
	return l.ReturnCount();
}

int LuaApi::GetMouseState(lua_State *lua)
{
	LuaCallHelper l(lua);
	MousePosition pos = KeyManager::GetMousePosition();
	checkparams();
	lua_newtable(lua);
	lua_pushintvalue(x, pos.X);
	lua_pushintvalue(y, pos.Y);
	lua_pushdoublevalue(relativeX, pos.RelativeX);
	lua_pushdoublevalue(relativeY, pos.RelativeY);
	
	lua_pushboolvalue(left, KeyManager::IsMouseButtonPressed(MouseButton::LeftButton));
	lua_pushboolvalue(middle, KeyManager::IsMouseButtonPressed(MouseButton::MiddleButton));
	lua_pushboolvalue(right, KeyManager::IsMouseButtonPressed(MouseButton::RightButton));
	return 1;
}

int LuaApi::Log(lua_State *lua)
{
	LuaCallHelper l(lua);
	string text = l.ReadString();
	checkparams();
	_context->Log(text);
	return l.ReturnCount();
}

int LuaApi::DisplayMessage(lua_State *lua)
{
	LuaCallHelper l(lua);
	string text = l.ReadString();
	string category = l.ReadString();
	checkparams();
	MessageManager::DisplayMessage(category, text);
	return l.ReturnCount();
}

int LuaApi::Reset(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	checkinitdone();
	_emu->GetSystemActionManager()->Reset();
	return l.ReturnCount();
}

int LuaApi::Stop(lua_State* lua)
{
	LuaCallHelper l(lua);
	int32_t stopCode = l.ReadInteger(0);
	checkminparams(0);
	_emu->SetStopCode(stopCode);
	return l.ReturnCount();
}

int LuaApi::BreakExecution(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	checkinitdone();
	_debugger->Step(_context->GetDefaultCpuType(), 1, StepType::Step);
	return l.ReturnCount();
}

int LuaApi::Resume(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	checkinitdone();
	_debugger->Run();
	return l.ReturnCount();
}

int LuaApi::Step(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(3);
	CpuType cpuType = (CpuType)l.ReadInteger((uint32_t)_context->GetDefaultCpuType());
	StepType stepType = (StepType)l.ReadInteger();
	int count = l.ReadInteger();
	checkminparams(2);
	checkinitdone();

	errorCond(count <= 0, "count must be >= 1");
	checkEnum(StepType, stepType, "invalid step type");
	checkEnum(CpuType, cpuType, "invalid cpu type");

	_debugger->Step(cpuType, count, stepType);

	return l.ReturnCount();
}

int LuaApi::Rewind(lua_State *lua)
{
	LuaCallHelper l(lua);
	int seconds = l.ReadInteger();
	checkparams();
	checksavestateconditions();
	errorCond(seconds <= 0, "seconds must be >= 1");
	_emu->GetRewindManager()->RewindSeconds(seconds);
	return l.ReturnCount();
}

int LuaApi::TakeScreenshot(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	stringstream ss;
	_emu->GetVideoDecoder()->TakeScreenshot(ss);
	l.Return(ss.str());
	return l.ReturnCount();
}

int LuaApi::IsKeyPressed(lua_State *lua)
{
	LuaCallHelper l(lua);
	string keyName = l.ReadString();
	checkparams();
	uint32_t keyCode = KeyManager::GetKeyCode(keyName);
	errorCond(keyCode == 0, "Invalid key name");
	l.Return(KeyManager::IsKeyPressed(keyCode));
	return l.ReturnCount();
}

int LuaApi::GetInput(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(2);
	int subport = l.ReadInteger(0);
	int port = l.ReadInteger();
	checkminparams(1);

	errorCond(port < 0 || port > 5, "Invalid port number - must be between 0 to 4");
	errorCond(subport < 0 || subport > IControllerHub::MaxSubPorts, "Invalid subport number");

	shared_ptr<BaseControlDevice> controller = _emu->GetConsoleUnsafe()->GetControlManager()->GetControlDevice(port, subport);

	lua_newtable(lua);

	if(controller) {
		vector<DeviceButtonName> buttons = controller->GetKeyNameAssociations();
		for(DeviceButtonName& btn : buttons) {
			lua_pushstring(lua, btn.Name.c_str());
			if(btn.IsNumeric) {
				if(btn.ButtonId == BaseControlDevice::DeviceXCoordButtonId) {
					lua_pushinteger(lua, controller->GetCoordinates().X);
				} else if(btn.ButtonId == BaseControlDevice::DeviceYCoordButtonId) {
					lua_pushinteger(lua, controller->GetCoordinates().Y);
				}
			} else {
				lua_pushboolean(lua, controller->IsPressed(btn.ButtonId));
			}
			lua_settable(lua, -3);
		}
	}

	return 1;
}

int LuaApi::SetInput(lua_State* lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(3);
	lua_settop(lua, 4);

	int subport = l.ReadInteger(0);
	int port = l.ReadInteger();

	errorCond(port < 0 || port > 5, "Invalid port number - must be between 0 to 4");
	errorCond(subport < 0 || subport > IControllerHub::MaxSubPorts, "Invalid subport number");

	shared_ptr<BaseControlDevice> controller = _emu->GetConsoleUnsafe()->GetControlManager()->GetControlDevice(port, subport);
	if(!controller) {
		return 0;
	}

	luaL_checktype(lua, 1, LUA_TTABLE);

	vector<DeviceButtonName> buttons = controller->GetKeyNameAssociations();
	for(DeviceButtonName& btn : buttons) {
		lua_getfield(lua, 1, btn.Name.c_str());
		if(btn.IsNumeric) {
			Nullable<int32_t> btnState = l.ReadOptionalInteger();
			if(btnState.HasValue) {
				if(btn.ButtonId == BaseControlDevice::DeviceXCoordButtonId) {
					MousePosition pos = controller->GetCoordinates();
					pos.X = (int16_t)btnState.Value;
					controller->SetCoordinates(pos);
				} else if(btn.ButtonId == BaseControlDevice::DeviceYCoordButtonId) {
					MousePosition pos = controller->GetCoordinates();
					pos.Y = (int16_t)btnState.Value;
					controller->SetCoordinates(pos);
				}
			}
		} else {
			Nullable<bool> btnState = l.ReadOptionalBool();
			if(btnState.HasValue) {
				controller->SetBitValue(btn.ButtonId, btnState.Value);
			}
		}
	}
	
	lua_pop(lua, 1);

	return l.ReturnCount();
}

int LuaApi::GetAccessCounters(lua_State *lua)
{
	LuaCallHelper l(lua);
	AccessCounterType counterType = (AccessCounterType)l.ReadInteger();
	MemoryType memoryType = (MemoryType)l.ReadInteger();
	checkEnum(MemoryType, memoryType, "Invalid memory type");
	checkEnum(AccessCounterType, counterType, "Invalid counter type");
	checkparams();

	uint32_t size = _memoryDumper->GetMemorySize(memoryType);
	vector<AddressCounters> counts;
	counts.resize(size, {});
	_debugger->GetMemoryAccessCounter()->GetAccessCounts(0, size, memoryType, counts.data());

	auto getValue = [&](AddressCounters& counter) -> uint64_t {
		switch(counterType) {
			default:
			case AccessCounterType::ReadCount: return counter.ReadCounter;
			case AccessCounterType::WriteCount: return counter.WriteCounter;
			case AccessCounterType::ExecCount: return counter.ExecCounter;
			case AccessCounterType::LastReadClock: return counter.ReadStamp;
			case AccessCounterType::LastWriteClock: return counter.WriteStamp;
			case AccessCounterType::LastExecClock: return counter.ExecStamp;
		}
	};

	lua_newtable(lua);
	for(uint32_t i = 0; i < size; i++) {
		lua_pushinteger(lua, getValue(counts[i]));
		lua_rawseti(lua, -2, i);
	}

	return 1;
}

int LuaApi::ResetAccessCounters(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	_debugger->GetMemoryAccessCounter()->ResetCounts();
	return l.ReturnCount();
}

int LuaApi::GetCdlData(lua_State* lua)
{
	LuaCallHelper l(lua);
	MemoryType memoryType = (MemoryType)l.ReadInteger();
	checkEnum(MemoryType, memoryType, "Invalid memory type");
	checkparams();

	if(!_debugger->GetCdlManager()->GetCodeDataLogger(memoryType)) {
		error("This memory type does not support CDL data (only some ROM memory types support it)");
	}

	uint32_t size = _memoryDumper->GetMemorySize(memoryType);
	vector<uint8_t> cdlData;
	cdlData.resize(size, {});
	_debugger->GetCdlManager()->GetCdlData(0, size, memoryType, cdlData.data());

	lua_newtable(lua);
	for(uint32_t i = 0; i < size; i++) {
		lua_pushinteger(lua, cdlData[i]);
		lua_rawseti(lua, -2, i);
	}

	return 1;
}

int LuaApi::GetScriptDataFolder(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	if(_emu->GetSettings()->GetDebugConfig().ScriptAllowIoOsAccess) {
		string baseFolder = FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "LuaScriptData");
		FolderUtilities::CreateFolder(baseFolder);
		string scriptFolder = FolderUtilities::CombinePath(baseFolder, FolderUtilities::GetFilename(_context->GetScriptName(), false));
		FolderUtilities::CreateFolder(scriptFolder);
		l.Return(scriptFolder);
	} else {
		l.Return("");
	}
	return l.ReturnCount();
}

int LuaApi::GetRomInfo(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();

	RomInfo romInfo = _emu->GetRomInfo();

	lua_newtable(lua);
	lua_pushstringvalue(name, romInfo.RomFile.GetFileName());
	lua_pushstringvalue(path, romInfo.RomFile.GetFilePath());
	lua_pushstringvalue(fileSha1Hash, _emu->GetHash(HashType::Sha1));

	return 1;
}

int LuaApi::GetLogWindowLog(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	
	l.Return(MessageManager::GetLog());
	return l.ReturnCount();
}

int LuaApi::AddCheat(lua_State* lua)
{
	LuaCallHelper l(lua);
	string code = l.ReadString();
	CheatType cheatType = (CheatType)l.ReadInteger();
	checkparams();

	checkEnum(CheatType, cheatType, "invalid cheat type");
	errorCond(code.length() > 15, "codes must be 15 characters or less");

	CheatCode cheatCode = {};
	cheatCode.Type = cheatType;
	memcpy(cheatCode.Code, code.c_str(), code.length());
	if(!_emu->GetCheatManager()->AddCheat(cheatCode)) {
		error("invalid cheat code")
	}
	return l.ReturnCount();
}

int LuaApi::ClearCheats(lua_State* lua)
{
	LuaCallHelper l(lua);
	checkparams();
	_emu->GetCheatManager()->InternalClearCheats();
	return l.ReturnCount();
}

int LuaApi::CreateSavestate(lua_State* lua)
{
	LuaCallHelper l(lua);
	checksavestateconditions();
	stringstream ss;
	_emu->GetSaveStateManager()->SaveState(ss);
	l.Return(ss.str());
	return l.ReturnCount();
}

int LuaApi::LoadSavestate(lua_State* lua)
{
	LuaCallHelper l(lua);
	string savestate = l.ReadString();
	checkparams();
	checksavestateconditions();
	
	stringstream ss;
	ss << savestate;
	bool result = _emu->GetSaveStateManager()->LoadState(ss);
	l.Return(result);
	return l.ReturnCount();
}

int LuaApi::GetState(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();

	Serializer s(0, true, SerializeFormat::Map);
	s.Stream(*_emu->GetConsole().get(), "", -1);
	
	//Add some more Lua-specific values
	uint32_t frameCount = _emu->GetFrameCount();
	uint32_t masterClock = _emu->GetMasterClock();
	uint32_t clockRate = _emu->GetMasterClockRate();
	string consoleType = string(magic_enum::enum_name<ConsoleType>(_emu->GetConsoleType()));
	string region = string(magic_enum::enum_name<ConsoleRegion>(_emu->GetRegion()));
	
	SV(clockRate);
	SV(consoleType);
	SV(region);
	SV(frameCount);
	SV(masterClock);

	unordered_map<string, SerializeMapValue>& values = s.GetMapValues();

	lua_newtable(lua);
	for(auto& kvp : values) {
		lua_pushstring(lua, kvp.first.c_str());
		switch(kvp.second.Format) {
			case SerializeMapValueFormat::Integer: lua_pushinteger(lua, kvp.second.Value.Integer); break;
			case SerializeMapValueFormat::Double: lua_pushnumber(lua, kvp.second.Value.Double); break;
			case SerializeMapValueFormat::Bool: lua_pushboolean(lua, kvp.second.Value.Bool); break;
			case SerializeMapValueFormat::String: lua_pushstring(lua, kvp.second.StringValue.c_str()); break;
		}
		lua_settable(lua, -3);
	}
	return 1;
}

int LuaApi::SetState(lua_State* lua)
{
	LuaCallHelper l(lua);
	lua_settop(lua, 1);
	luaL_checktype(lua, -1, LUA_TTABLE);

	unordered_map<string, SerializeMapValue> map;

	lua_pushnil(lua);  /* first key */
	while(lua_next(lua, -2) != 0) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if(lua_type(lua, -2) == LUA_TSTRING) {
			size_t len = 0;
			const char* cstr = lua_tolstring(lua, -2, &len);
			string key = string(cstr, len);

			switch(lua_type(lua, -1)) {
				case LUA_TBOOLEAN: {
					map.try_emplace(key, SerializeMapValueFormat::Bool, (bool)lua_toboolean(lua, -1));
					break;
				}

				case LUA_TNUMBER: {
					if(lua_isinteger(lua, -1)) {
						map.try_emplace(key, SerializeMapValueFormat::Integer, (int64_t)lua_tointeger(lua, -1));
					} else if(lua_isnumber(lua, -1)) {
						map.try_emplace(key, SerializeMapValueFormat::Double, (double)lua_tonumber(lua, -1));
					}
					break;
				}
			}
		}
		
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(lua, 1);
	}

	Serializer s(0, false, SerializeFormat::Map);
	s.LoadFromMap(map);

	s.Stream(*_emu->GetConsole().get(), "", -1);
	unordered_map<string, SerializeMapValue>& values = s.GetMapValues();

	lua_newtable(lua);
	for(auto& kvp : values) {
		lua_pushstring(lua, kvp.first.c_str());
		switch(kvp.second.Format) {
			case SerializeMapValueFormat::Integer: lua_pushinteger(lua, kvp.second.Value.Integer); break;
			case SerializeMapValueFormat::Double: lua_pushnumber(lua, kvp.second.Value.Double); break;
			case SerializeMapValueFormat::Bool: lua_pushboolean(lua, kvp.second.Value.Bool); break;
		}
		lua_settable(lua, -3);
	}
	return 1;
}