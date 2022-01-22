#include "stdafx.h"

#ifndef LIBRETRO
#include "LuaApi.h"
#include "../Lua/lua.hpp"
#include "Debugger/LuaCallHelper.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/ScriptingContext.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesControlManager.h"
#include "SNES/Input/SnesController.h"
#include "SNES/SnesPpu.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/LabelManager.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/MessageManager.h"
#include "Shared/RewindManager.h"
#include "Shared/SaveStateManager.h"
#include "Shared/Emulator.h"
#include "Shared/KeyManager.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "MemoryOperationType.h"

#define lua_pushintvalue(name, value) lua_pushliteral(lua, #name); lua_pushinteger(lua, (int)value); lua_settable(lua, -3);
#define lua_pushdoublevalue(name, value) lua_pushliteral(lua, #name); lua_pushnumber(lua, (double)value); lua_settable(lua, -3);
#define lua_pushboolvalue(name, value) lua_pushliteral(lua, #name); lua_pushboolean(lua, (int)value); lua_settable(lua, -3);
#define lua_pushstringvalue(name, value) lua_pushliteral(lua, #name); lua_pushstring(lua, value.c_str()); lua_settable(lua, -3);
#define lua_pusharrayvalue(index, value) lua_pushinteger(lua, index); lua_pushinteger(lua, value); lua_settable(lua, -3);

#define lua_starttable(name) lua_pushliteral(lua, name); lua_newtable(lua);
#define lua_endtable() lua_settable(lua, -3);
#define lua_readint(name, dest) lua_getfield(lua, -1, #name); dest = l.ReadInteger();
#define lua_readbool(name, dest) lua_getfield(lua, -1, #name); dest = l.ReadBool();
#define error(text) luaL_error(lua, text); return 0;
#define errorCond(cond, text) if(cond) { luaL_error(lua, text); return 0; }
#define checkparams() if(!l.CheckParamCount()) { return 0; }
#define checkminparams(x) if(!l.CheckParamCount(x)) { return 0; }
#define checkinitdone() if(!_context->CheckInitDone()) { error("This function cannot be called outside a callback"); return 0; }
#define checksavestateconditions() if(!_context->CheckInStartFrameEvent() && !_context->CheckInExecOpEvent()) { error("This function must be called inside a StartFrame event callback or a CpuExec memory operation callback"); return 0; }

Debugger* LuaApi::_debugger = nullptr;
Emulator* LuaApi::_emu = nullptr;
SnesPpu* LuaApi::_ppu = nullptr;
MemoryDumper* LuaApi::_memoryDumper = nullptr;
ScriptingContext* LuaApi::_context = nullptr;

void LuaApi::SetContext(ScriptingContext* context)
{
	_context = context;
	_debugger = _context->GetDebugger();
	_memoryDumper = _debugger->GetMemoryDumper();
	_emu = _debugger->GetEmulator();

	//TODO
	//_ppu = _debugger->GetConsole()->GetPpu().get();
}

int LuaApi::GetLibrary(lua_State *lua)
{
	static const luaL_Reg apilib[] = {
		{ "read", LuaApi::ReadMemory },
		{ "write", LuaApi::WriteMemory },
		{ "readWord", LuaApi::ReadMemoryWord },
		{ "writeWord", LuaApi::WriteMemoryWord },
		{ "getPrgRomOffset", LuaApi::GetPrgRomOffset },
		{ "addMemoryCallback", LuaApi::RegisterMemoryCallback },
		{ "removeMemoryCallback", LuaApi::UnregisterMemoryCallback },
		{ "addEventCallback", LuaApi::RegisterEventCallback },
		{ "removeEventCallback", LuaApi::UnregisterEventCallback },
		{ "drawString", LuaApi::DrawString },
		{ "drawPixel", LuaApi::DrawPixel },
		{ "drawLine", LuaApi::DrawLine },
		{ "drawRectangle", LuaApi::DrawRectangle },
		{ "clearScreen", LuaApi::ClearScreen },
		{ "getScreenBuffer", LuaApi::GetScreenBuffer },
		{ "setScreenBuffer", LuaApi::SetScreenBuffer },
		{ "getPixel", LuaApi::GetPixel },
		{ "getMouseState", LuaApi::GetMouseState },
		{ "log", LuaApi::Log },
		{ "displayMessage", LuaApi::DisplayMessage },
		{ "reset", LuaApi::Reset },
		{ "breakExecution", LuaApi::Break },
		{ "resume", LuaApi::Resume },
		{ "execute", LuaApi::Execute },
		{ "rewind", LuaApi::Rewind },
		{ "takeScreenshot", LuaApi::TakeScreenshot },
		{ "isKeyPressed", LuaApi::IsKeyPressed },
		{ "getInput", LuaApi::GetInput },
		{ "getAccessCounters", LuaApi::GetAccessCounters },
		{ "resetAccessCounters", LuaApi::ResetAccessCounters },
		{ "getState", LuaApi::GetState },
		{ "getScriptDataFolder", LuaApi::GetScriptDataFolder },
		{ "getRomInfo", LuaApi::GetRomInfo },
		{ "getLogWindowLog", LuaApi::GetLogWindowLog },
		{ "getLabelAddress", LuaApi::GetLabelAddress },
		{ NULL,NULL }
	};

	luaL_newlib(lua, apilib);

	//Expose MemoryType enum as "emu.memType"
	lua_pushliteral(lua, "memType");
	lua_newtable(lua);
	lua_pushintvalue(cpu, MemoryType::SnesMemory);
	lua_pushintvalue(spc, MemoryType::SpcMemory);
	lua_pushintvalue(sa1, MemoryType::Sa1Memory);
	lua_pushintvalue(gsu, MemoryType::GsuMemory);
	lua_pushintvalue(cx4, MemoryType::Cx4Memory);
	lua_pushintvalue(gameboy, MemoryType::GameboyMemory);
	lua_pushintvalue(cgram, MemoryType::SnesCgRam);
	lua_pushintvalue(vram, MemoryType::SnesVideoRam);
	lua_pushintvalue(oam, MemoryType::SnesSpriteRam);
	lua_pushintvalue(prgRom, MemoryType::SnesPrgRom);
	lua_pushintvalue(workRam, MemoryType::SnesWorkRam);
	lua_pushintvalue(saveRam, MemoryType::SnesSaveRam);
	lua_pushintvalue(gbPrgRom, MemoryType::GbPrgRom);
	lua_pushintvalue(gbWorkRam, MemoryType::GbWorkRam);
	lua_pushintvalue(gbCartRam, MemoryType::GbCartRam);
	lua_pushintvalue(gbVideoRam, MemoryType::GbVideoRam);
	lua_pushintvalue(cpuDebug, MemoryType::SnesMemory | 0x100);
	lua_pushintvalue(spcDebug, MemoryType::SpcMemory | 0x100);
	lua_pushintvalue(sa1Debug, MemoryType::Sa1Memory | 0x100);
	lua_pushintvalue(gsuDebug, MemoryType::GsuMemory | 0x100);
	lua_pushintvalue(cx4Debug, MemoryType::Cx4Memory | 0x100);
	lua_pushintvalue(gameboyDebug, MemoryType::GameboyMemory | 0x100);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "memCallbackType");
	lua_newtable(lua);
	lua_pushintvalue(read, CallbackType::CpuRead);
	lua_pushintvalue(write, CallbackType::CpuWrite);
	lua_pushintvalue(exec, CallbackType::CpuExec);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "counterMemType");
	lua_newtable(lua);
	lua_pushintvalue(prgRom, MemoryType::SnesPrgRom);
	lua_pushintvalue(workRam, MemoryType::SnesWorkRam);
	lua_pushintvalue(saveRam, MemoryType::SnesSaveRam);
	lua_pushintvalue(videoRam, MemoryType::SnesVideoRam);
	lua_pushintvalue(spriteRam, MemoryType::SnesSpriteRam);
	lua_pushintvalue(cgRam, MemoryType::SnesCgRam);
	lua_pushintvalue(spcRam, MemoryType::SpcRam);
	lua_pushintvalue(spcRom, MemoryType::SpcRom);
	lua_pushintvalue(dspProgramRom, MemoryType::DspProgramRom);
	lua_pushintvalue(dspDataRom, MemoryType::DspDataRom);
	lua_pushintvalue(dspDataRam, MemoryType::DspDataRam);
	lua_pushintvalue(sa1InternalRam, MemoryType::Sa1InternalRam);
	lua_pushintvalue(gsuWorkRam, MemoryType::GsuWorkRam);
	lua_pushintvalue(cx4DataRam, MemoryType::Cx4DataRam);
	lua_pushintvalue(bsxPsRam, MemoryType::BsxPsRam);
	lua_pushintvalue(bsxMemoryPack, MemoryType::BsxMemoryPack);
	lua_pushintvalue(gbPrgRom, MemoryType::GbPrgRom);
	lua_pushintvalue(gbWorkRam, MemoryType::GbWorkRam);
	lua_pushintvalue(gbCartRam, MemoryType::GbCartRam);
	lua_pushintvalue(gbVideoRam, MemoryType::GbVideoRam);
	lua_pushintvalue(gbHighRam, MemoryType::GbHighRam);
	lua_pushintvalue(gbBootRom, MemoryType::GbBootRom);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "counterOpType");
	lua_newtable(lua);
	lua_pushintvalue(read, MemoryOperationType::Read);
	lua_pushintvalue(write, MemoryOperationType::Write);
	lua_pushintvalue(exec, MemoryOperationType::ExecOpCode);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "eventType");
	lua_newtable(lua);
	lua_pushintvalue(reset, EventType::Reset);
	lua_pushintvalue(nmi, EventType::Nmi);
	lua_pushintvalue(irq, EventType::Irq);
	lua_pushintvalue(startFrame, EventType::StartFrame);
	lua_pushintvalue(endFrame, EventType::EndFrame);
	lua_pushintvalue(inputPolled, EventType::InputPolled);
	lua_pushintvalue(scriptEnded, EventType::ScriptEnded);
	lua_pushintvalue(stateLoaded, EventType::StateLoaded);
	lua_pushintvalue(stateSaved, EventType::StateSaved);
	lua_pushintvalue(gbStartFrame, EventType::GbStartFrame);
	lua_pushintvalue(gbEndFrame, EventType::GbEndFrame);
	//TODO
	/*lua_pushintvalue(codeBreak, EventType::CodeBreak);
	*/
	lua_settable(lua, -3);

	lua_pushliteral(lua, "stepType");
	lua_newtable(lua);
	lua_pushintvalue(cpuInstructions, StepType::Step);
	lua_pushintvalue(ppuCycles, StepType::PpuStep);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "cpuType");
	lua_newtable(lua);
	lua_pushintvalue(cpu, CpuType::Snes);
	lua_pushintvalue(spc, CpuType::Spc);
	lua_pushintvalue(dsp, CpuType::NecDsp);
	lua_pushintvalue(sa1, CpuType::Sa1);
	lua_pushintvalue(gsu, CpuType::Gsu);
	lua_pushintvalue(cx4, CpuType::Cx4);
	lua_pushintvalue(gameboy, CpuType::Gameboy);
	lua_settable(lua, -3);

	return 1;
}

int LuaApi::GetLabelAddress(lua_State *lua)
{
	LuaCallHelper l(lua);
	string label = l.ReadString();
	checkparams();
	errorCond(label.length() == 0, "label cannot be empty");

	LabelManager* lblMan = _debugger->GetLabelManager();
	int32_t value = lblMan->GetLabelRelativeAddress(label);
	if(value == -2) {
		//Check to see if the label is a multi-byte label instead
		string mbLabel = label + "+0";
		value = lblMan->GetLabelRelativeAddress(mbLabel);
	}
	errorCond(value == -1, "label out of scope (not mapped to CPU memory)");
	errorCond(value <= -2, "label not found");

	l.Return(value);
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
	_memoryDumper->SetMemoryValue(memType, address, value, disableSideEffects);
	return l.ReturnCount();
}

int LuaApi::ReadMemoryWord(lua_State *lua)
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
	uint16_t value = _memoryDumper->GetMemoryValueWord(memType, address, disableSideEffects);
	l.Return(returnSignedValue ? (int16_t)value : value);
	return l.ReturnCount();
}

int LuaApi::WriteMemoryWord(lua_State *lua)
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
	_memoryDumper->SetMemoryValueWord(memType, address, value, disableSideEffects);
	return l.ReturnCount();
}

int LuaApi::GetPrgRomOffset(lua_State *lua)
{
	LuaCallHelper l(lua);
	int address = l.ReadInteger();
	checkminparams(1);
	errorCond(address < 0 || address > 0xFFFF, "address must be between 0 and $FFFF");
	
	AddressInfo relAddress { address, MemoryType::SnesMemory };
	int32_t prgRomOffset = _debugger->GetAbsoluteAddress(relAddress).Address;
	l.Return(prgRomOffset);
	return l.ReturnCount();
}

int LuaApi::RegisterMemoryCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(5);
	CpuType cpuType = (CpuType)l.ReadInteger((int)CpuType::Snes);
	int32_t endAddr = l.ReadInteger(-1);
	int32_t startAddr = l.ReadInteger();
	CallbackType callbackType = (CallbackType)l.ReadInteger();
	int reference = l.GetReference();
	checkminparams(3);

	if(endAddr == -1) {
		endAddr = startAddr;
	}

	errorCond(startAddr > endAddr, "start address must be <= end address");
	errorCond(callbackType < CallbackType::CpuRead || callbackType > CallbackType::CpuExec, "the specified type is invalid");
	errorCond(cpuType < CpuType::Snes || cpuType > CpuType::Gameboy, "the cpu type is invalid");
	errorCond(reference == LUA_NOREF, "the specified function could not be found");
	_context->RegisterMemoryCallback(callbackType, startAddr, endAddr, cpuType, reference);
	_context->Log("Registered memory callback from $" + HexUtilities::ToHex((uint32_t)startAddr) + " to $" + HexUtilities::ToHex((uint32_t)endAddr));
	l.Return(reference);
	return l.ReturnCount();
}

int LuaApi::UnregisterMemoryCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(5);

	CpuType cpuType = (CpuType)l.ReadInteger((int)CpuType::Snes);
	int endAddr = l.ReadInteger(-1);
	int startAddr = l.ReadInteger();
	CallbackType type = (CallbackType)l.ReadInteger();
	int reference = l.ReadInteger();

	checkminparams(3);

	if(endAddr == -1) {
		endAddr = startAddr;
	}

	errorCond(startAddr > endAddr, "start address must be <= end address");
	errorCond(type < CallbackType::CpuRead || type > CallbackType::CpuExec, "the specified type is invalid");
	errorCond(reference == LUA_NOREF, "function reference is invalid");
	_context->UnregisterMemoryCallback(type, startAddr, endAddr, cpuType, reference);
	return l.ReturnCount();
}

int LuaApi::RegisterEventCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	EventType type = (EventType)l.ReadInteger();
	int reference = l.GetReference();
	checkparams();
	errorCond(type < EventType::Nmi || type >= EventType::EventTypeSize, "the specified type is invalid");
	errorCond(reference == LUA_NOREF, "the specified function could not be found");
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
	errorCond(type < EventType::Nmi || type >= EventType::EventTypeSize, "the specified type is invalid");
	errorCond(reference == LUA_NOREF, "function reference is invalid");
	_context->UnregisterEventCallback(type, reference);
	return l.ReturnCount();
}

int LuaApi::DrawString(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(7);
	int displayDelay = l.ReadInteger(0);
	int frameCount = l.ReadInteger(1);
	int backColor = l.ReadInteger(0);
	int color = l.ReadInteger(0xFFFFFF);
	string text = l.ReadString();
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkminparams(3);

	int startFrame = _emu->GetFrameCount() + displayDelay;
	_emu->GetDebugHud()->DrawString(x, y, text, color, backColor, frameCount, startFrame);

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
	_emu->GetDebugHud()->DrawLine(x, y, x2, y2, color, frameCount, startFrame);

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
	_emu->GetDebugHud()->DrawPixel(x, y, color, frameCount, startFrame);

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
	_emu->GetDebugHud()->DrawRectangle(x, y, width, height, color, fill, frameCount, startFrame);

	return l.ReturnCount();
}

int LuaApi::ClearScreen(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();

	_emu->GetDebugHud()->ClearScreen();
	return l.ReturnCount();
}

int LuaApi::GetScreenBuffer(lua_State *lua)
{
	LuaCallHelper l(lua);

	int multiplier = _ppu->IsHighResOutput() ? 2 : 1;

	lua_newtable(lua);
	for(int y = 0; y < 239; y++) {
		for(int x = 0; x < 256; x++) {
			//TODO
			//lua_pushinteger(lua, DefaultVideoFilter::ToArgb(*(_ppu->GetScreenBuffer() + y * 256 * multiplier * multiplier + x * multiplier)) & 0xFFFFFF);
			lua_rawseti(lua, -2, (y << 8) + x);
		}
	}

	return 1;
}

int LuaApi::SetScreenBuffer(lua_State *lua)
{
	LuaCallHelper l(lua);
	uint32_t pixels[256*239] = {};
	luaL_checktype(lua, 1, LUA_TTABLE);
	for(int i = 0; i < 256*239; i++) {
		lua_rawgeti(lua, 1, i);
		pixels[i] = l.ReadInteger() ^ 0xFF000000;
	}
	
	int startFrame = _emu->GetFrameCount();
	_emu->GetDebugHud()->DrawScreenBuffer(pixels, startFrame);

	return l.ReturnCount();
}

int LuaApi::GetPixel(lua_State *lua)
{
	LuaCallHelper l(lua);
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkparams();
	errorCond(x < 0 || x > 255 || y < 0 || y > 238, "invalid x,y coordinates (must be between 0-255, 0-238)");

	int multiplier = _ppu->IsHighResOutput() ? 2 : 1;

	//Ignores intensify & grayscale bits
	//TODO
	//l.Return(DefaultVideoFilter::ToArgb(*(_ppu->GetScreenBuffer() + y * 256 * multiplier * multiplier + x * multiplier)) & 0xFFFFFF);
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
	_emu->Reset();
	return l.ReturnCount();
}

int LuaApi::Break(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	checkinitdone();
	_debugger->Step(CpuType::Snes, 1, StepType::Step);
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

int LuaApi::Execute(lua_State *lua)
{
	LuaCallHelper l(lua);
	StepType type = (StepType)l.ReadInteger();
	int count = l.ReadInteger();
	checkparams();
	checkinitdone();
	errorCond(count <= 0, "count must be >= 1");
	errorCond(type != StepType::Step && type != StepType::PpuStep, "type is invalid");

	_debugger->Step(CpuType::Snes, count, type);

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
	int port = l.ReadInteger();
	checkparams();
	errorCond(port < 0 || port > 4, "Invalid port number - must be between 0 to 4");

	//TODO
	shared_ptr<SnesController> controller = std::dynamic_pointer_cast<SnesController>(_emu->GetControlManager()->GetControlDevice(port));
	errorCond(controller == nullptr, "Input port must be connected to a standard controller");

	lua_newtable(lua);
	lua_pushboolvalue(a, controller->IsPressed(SnesController::Buttons::A));
	lua_pushboolvalue(b, controller->IsPressed(SnesController::Buttons::B));
	lua_pushboolvalue(x, controller->IsPressed(SnesController::Buttons::X));
	lua_pushboolvalue(y, controller->IsPressed(SnesController::Buttons::Y));
	lua_pushboolvalue(l, controller->IsPressed(SnesController::Buttons::L));
	lua_pushboolvalue(r, controller->IsPressed(SnesController::Buttons::R));
	lua_pushboolvalue(start, controller->IsPressed(SnesController::Buttons::Start));
	lua_pushboolvalue(select, controller->IsPressed(SnesController::Buttons::Select));
	lua_pushboolvalue(up, controller->IsPressed(SnesController::Buttons::Up));
	lua_pushboolvalue(down, controller->IsPressed(SnesController::Buttons::Down));
	lua_pushboolvalue(left, controller->IsPressed(SnesController::Buttons::Left));
	lua_pushboolvalue(right, controller->IsPressed(SnesController::Buttons::Right));
	return 1;
}

int LuaApi::GetAccessCounters(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(2);
	MemoryOperationType operationType = (MemoryOperationType)l.ReadInteger();
	MemoryType memoryType = (MemoryType)l.ReadInteger();
	errorCond(operationType >= MemoryOperationType::ExecOperand, "Invalid operation type");
	errorCond(memoryType >= MemoryType::Register, "Invalid memory type");
	checkparams();

	uint32_t size = 0;
	vector<AddressCounters> counts;
	counts.resize(_memoryDumper->GetMemorySize(memoryType), {});
	_debugger->GetMemoryAccessCounter()->GetAccessCounts(0, size, memoryType, counts.data());

	lua_newtable(lua);
	switch(operationType) {
		default:
		case MemoryOperationType::Read: 
			for(uint32_t i = 0; i < size; i++) {
				lua_pushinteger(lua, counts[i].ReadCount);
				lua_rawseti(lua, -2, i);
			}
			break;

		case MemoryOperationType::Write:
			for(uint32_t i = 0; i < size; i++) {
				lua_pushinteger(lua, counts[i].WriteCount);
				lua_rawseti(lua, -2, i);
			}
			break;

		case MemoryOperationType::ExecOpCode:
			for(uint32_t i = 0; i < size; i++) {
				lua_pushinteger(lua, counts[i].ExecCount);
				lua_rawseti(lua, -2, i);
			}
			break;
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

int LuaApi::GetScriptDataFolder(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	string baseFolder = FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "LuaScriptData");
	FolderUtilities::CreateFolder(baseFolder);
	string scriptFolder = FolderUtilities::CombinePath(baseFolder, FolderUtilities::GetFilename(_context->GetScriptName(), false));
	FolderUtilities::CreateFolder(scriptFolder);
	l.Return(scriptFolder);
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

int LuaApi::GetState(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();

	//TODO
	/*
	lua_newtable(lua);
	lua_pushintvalue(masterClock, state.MasterClock);

	lua_starttable("cpu");
	lua_pushintvalue(a, state.Cpu.A);
	lua_pushintvalue(cycleCount, state.Cpu.CycleCount);
	lua_pushboolvalue(emulationMode, state.Cpu.EmulationMode);
	lua_pushintvalue(irqFlag, state.Cpu.IrqSource);
	lua_pushboolvalue(nmiFlag, state.Cpu.NmiFlag);
	lua_pushintvalue(k, state.Cpu.K);
	lua_pushintvalue(pc, state.Cpu.PC);
	lua_pushintvalue(status, state.Cpu.PS);
	lua_pushintvalue(sp, state.Cpu.SP);
	lua_pushintvalue(x, state.Cpu.X);
	lua_pushintvalue(y, state.Cpu.Y);
	lua_pushintvalue(d, state.Cpu.D);
	lua_pushintvalue(db, state.Cpu.DBR);
	lua_endtable(); //end cpu

	lua_starttable("ppu");
	lua_pushintvalue(cycle, state.Ppu.Cycle);
	lua_pushintvalue(frameCount, state.Ppu.FrameCount);
	lua_pushintvalue(scanline, state.Ppu.Scanline);
	lua_pushintvalue(hClock, state.Ppu.HClock);
	lua_pushboolvalue(forcedVblank, state.Ppu.ForcedVblank);
	lua_pushintvalue(screenBrightness, state.Ppu.ScreenBrightness);
	
	lua_starttable("mode7");
	
	lua_starttable("matrix");
	lua_pusharrayvalue(0, state.Ppu.Mode7.Matrix[0]);
	lua_pusharrayvalue(1, state.Ppu.Mode7.Matrix[0]);
	lua_pusharrayvalue(2, state.Ppu.Mode7.Matrix[0]);
	lua_pusharrayvalue(3, state.Ppu.Mode7.Matrix[0]);
	lua_endtable();

	lua_pushintvalue(hScroll, state.Ppu.Mode7.HScroll);
	lua_pushintvalue(vScroll, state.Ppu.Mode7.VScroll);
	lua_pushintvalue(centerX, state.Ppu.Mode7.CenterX);
	lua_pushintvalue(centerY, state.Ppu.Mode7.CenterY);
	lua_pushintvalue(valueLatch, state.Ppu.Mode7.ValueLatch);
	lua_pushboolvalue(largeMap, state.Ppu.Mode7.LargeMap);
	lua_pushboolvalue(fillWithTile0, state.Ppu.Mode7.FillWithTile0);
	lua_pushboolvalue(horizontalMirroring, state.Ppu.Mode7.HorizontalMirroring);
	lua_pushboolvalue(verticalMirroring, state.Ppu.Mode7.VerticalMirroring);
	lua_endtable(); //end mode7

	lua_pushintvalue(bgMode, state.Ppu.BgMode);
	lua_pushboolvalue(mode1Bg3Priority, state.Ppu.Mode1Bg3Priority);
	lua_pushintvalue(mainScreenLayers, state.Ppu.MainScreenLayers);
	lua_pushintvalue(subScreenLayers, state.Ppu.SubScreenLayers);
	
	lua_starttable("layers")
	for(int i = 0; i < 4; i++) {
		lua_pushinteger(lua, i);
		
		lua_newtable(lua);
		lua_pushintvalue(tilemapAddress, state.Ppu.Layers[i].TilemapAddress);
		lua_pushintvalue(chrAddress, state.Ppu.Layers[i].ChrAddress);
		lua_pushintvalue(hScroll, state.Ppu.Layers[i].HScroll);
		lua_pushintvalue(vScroll, state.Ppu.Layers[i].VScroll);
		lua_pushintvalue(doubleWidth, state.Ppu.Layers[i].DoubleWidth);
		lua_pushintvalue(doubleHeight, state.Ppu.Layers[i].DoubleHeight);
		lua_pushintvalue(largeTiles, state.Ppu.Layers[i].LargeTiles);		

		lua_settable(lua, -3);
	}
	lua_endtable(); //end layers

	lua_starttable("windows")
	for(int i = 0; i < 2; i++) {
		lua_pushinteger(lua, i);

		lua_newtable(lua);
		lua_pushintvalue(activeLayers, (
			(uint8_t)state.Ppu.Window[i].ActiveLayers[0] |
			((uint8_t)state.Ppu.Window[i].ActiveLayers[1] << 1) |
			((uint8_t)state.Ppu.Window[i].ActiveLayers[2] << 2) |
			((uint8_t)state.Ppu.Window[i].ActiveLayers[3] << 3) |
			((uint8_t)state.Ppu.Window[i].ActiveLayers[4] << 4) |
			((uint8_t)state.Ppu.Window[i].ActiveLayers[5] << 5)
		));

		lua_pushintvalue(invertedLayers, (
			(uint8_t)state.Ppu.Window[i].InvertedLayers[0] |
			((uint8_t)state.Ppu.Window[i].InvertedLayers[1] << 1) |
			((uint8_t)state.Ppu.Window[i].InvertedLayers[2] << 2) |
			((uint8_t)state.Ppu.Window[i].InvertedLayers[3] << 3) |
			((uint8_t)state.Ppu.Window[i].InvertedLayers[4] << 4) |
			((uint8_t)state.Ppu.Window[i].InvertedLayers[5] << 5)
		));

		lua_pushintvalue(left, state.Ppu.Window[i].Left);
		lua_pushintvalue(right, state.Ppu.Window[i].Right);

		lua_settable(lua, -3);
	}
	lua_endtable(); //end windows
	
	lua_pushboolvalue(windowMaskLogicBg0, (int)state.Ppu.MaskLogic[0]);
	lua_pushboolvalue(windowMaskLogicBg1, (int)state.Ppu.MaskLogic[1]);
	lua_pushboolvalue(windowMaskLogicBg2, (int)state.Ppu.MaskLogic[2]);
	lua_pushboolvalue(windowMaskLogicBg3, (int)state.Ppu.MaskLogic[3]);
	lua_pushboolvalue(windowMaskLogicSprites, (int)state.Ppu.MaskLogic[4]);
	lua_pushboolvalue(windowMaskLogicColor, (int)state.Ppu.MaskLogic[5]);

	lua_pushboolvalue(windowMaskMainBg0, state.Ppu.WindowMaskMain[0]);
	lua_pushboolvalue(windowMaskMainBg1, state.Ppu.WindowMaskMain[1]);
	lua_pushboolvalue(windowMaskMainBg2, state.Ppu.WindowMaskMain[2]);
	lua_pushboolvalue(windowMaskMainBg3, state.Ppu.WindowMaskMain[3]);
	lua_pushboolvalue(windowMaskMainSprites, state.Ppu.WindowMaskMain[4]);

	lua_pushboolvalue(windowMaskSubBg0, state.Ppu.WindowMaskSub[0]);
	lua_pushboolvalue(windowMaskSubBg1, state.Ppu.WindowMaskSub[1]);
	lua_pushboolvalue(windowMaskSubBg2, state.Ppu.WindowMaskSub[2]);
	lua_pushboolvalue(windowMaskSubBg3, state.Ppu.WindowMaskSub[3]);
	lua_pushboolvalue(windowMaskSubSprites, state.Ppu.WindowMaskSub[4]);

	lua_pushintvalue(vramAddress, state.Ppu.VramAddress);
	lua_pushintvalue(vramIncrementValue, state.Ppu.VramIncrementValue);
	lua_pushintvalue(vramAddressRemapping, state.Ppu.VramAddressRemapping);
	lua_pushboolvalue(vramAddrIncrementOnSecondReg, state.Ppu.VramAddrIncrementOnSecondReg);
	lua_pushintvalue(vramReadBuffer, state.Ppu.VramReadBuffer);

	lua_pushintvalue(ppu1OpenBus, state.Ppu.Ppu1OpenBus);
	lua_pushintvalue(ppu2OpenBus, state.Ppu.Ppu2OpenBus);
	
	lua_pushintvalue(cgramAddress, state.Ppu.CgramAddress);
	lua_pushintvalue(cgramWriteBuffer, state.Ppu.CgramWriteBuffer);
	lua_pushboolvalue(cgramAddressLatch, state.Ppu.CgramAddressLatch);

	lua_pushintvalue(mosaicSize, state.Ppu.MosaicSize);
	lua_pushintvalue(mosaicEnabled, state.Ppu.MosaicEnabled);

	lua_pushintvalue(oamRamAddress, state.Ppu.OamRamAddress);
	lua_pushintvalue(oamMode, state.Ppu.OamMode);
	lua_pushintvalue(oamBaseAddress, state.Ppu.OamBaseAddress);
	lua_pushintvalue(oamAddressOffset, state.Ppu.OamAddressOffset);
	lua_pushboolvalue(enableOamPriority, state.Ppu.EnableOamPriority);

	lua_pushboolvalue(extBgEnabled, state.Ppu.ExtBgEnabled);
	lua_pushboolvalue(hiResMode, state.Ppu.HiResMode);
	lua_pushboolvalue(screenInterlace, state.Ppu.ScreenInterlace);
	lua_pushboolvalue(objInterlace, state.Ppu.ObjInterlace);
	lua_pushboolvalue(overscanMode, state.Ppu.OverscanMode);
	lua_pushboolvalue(directColorMode, state.Ppu.DirectColorMode);

	lua_pushintvalue(colorMathClipMode, (int)state.Ppu.ColorMathClipMode);
	lua_pushintvalue(colorMathPreventMode, (int)state.Ppu.ColorMathPreventMode);
	lua_pushboolvalue(colorMathAddSubscreen, state.Ppu.ColorMathAddSubscreen);
	lua_pushintvalue(colorMathEnabled, state.Ppu.ColorMathEnabled);
	lua_pushboolvalue(colorMathSubstractMode, state.Ppu.ColorMathSubstractMode);
	lua_pushboolvalue(colorMathHalveResult, state.Ppu.ColorMathHalveResult);
	lua_pushintvalue(fixedColor, state.Ppu.FixedColor);

	lua_endtable(); //end ppu

	lua_starttable("spc");
	lua_pushintvalue(a, state.Spc.A);
	lua_pushintvalue(pc, state.Spc.PC);
	lua_pushintvalue(status, state.Spc.PS);
	lua_pushintvalue(sp, state.Spc.SP);
	lua_pushintvalue(x, state.Spc.X);
	lua_pushintvalue(y, state.Spc.Y);
	lua_endtable(); //end spc
	*/
	return 1;
}
#endif