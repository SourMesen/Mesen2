#include "stdafx.h"
#include "LuaApi.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FolderUtilities.h"
#include "../Lua/lua.hpp"
#include "LuaCallHelper.h"
#include "Debugger.h"
#include "MemoryDumper.h"
#include "MessageManager.h"
#include "ScriptingContext.h"
#include "DebugHud.h"
#include "VideoDecoder.h"
#include "RewindManager.h"
#include "SaveStateManager.h"
#include "Console.h"
#include "BaseCartridge.h"
#include "IKeyManager.h"
#include "ControlManager.h"
#include "SnesController.h"
#include "Ppu.h"
#include "KeyManager.h"
#include "MemoryAccessCounter.h"
#include "LabelManager.h"
#include "DefaultVideoFilter.h"

#define lua_pushintvalue(name, value) lua_pushliteral(lua, #name); lua_pushinteger(lua, (int)value); lua_settable(lua, -3);
#define lua_pushdoublevalue(name, value) lua_pushliteral(lua, #name); lua_pushnumber(lua, (double)value); lua_settable(lua, -3);
#define lua_pushboolvalue(name, value) lua_pushliteral(lua, #name); lua_pushboolean(lua, (int)value); lua_settable(lua, -3);
#define lua_pushstringvalue(name, value) lua_pushliteral(lua, #name); lua_pushstring(lua, value.c_str()); lua_settable(lua, -3);
#define lua_starttable(name) lua_pushliteral(lua, name); lua_newtable(lua);
#define lua_endtable() lua_settable(lua, -3);
#define lua_readint(name, dest) lua_getfield(lua, -1, #name); dest = l.ReadInteger();
#define lua_readbool(name, dest) lua_getfield(lua, -1, #name); dest = l.ReadBool();
#define error(text) luaL_error(lua, text); return 0;
#define errorCond(cond, text) if(cond) { luaL_error(lua, text); return 0; }
#define checkparams() if(!l.CheckParamCount()) { return 0; }
#define checkminparams(x) if(!l.CheckParamCount(x)) { return 0; }
#define checksavestateconditions() if(!_context->CheckInStartFrameEvent() && !_context->CheckInExecOpEvent()) { error("This function must be called inside a StartFrame event callback or a CpuExec memory operation callback"); return 0; }

Debugger* LuaApi::_debugger = nullptr;
Console* LuaApi::_console = nullptr;
Ppu* LuaApi::_ppu = nullptr;
MemoryDumper* LuaApi::_memoryDumper = nullptr;
ScriptingContext* LuaApi::_context = nullptr;

void LuaApi::SetContext(ScriptingContext* context)
{
	_context = context;
	_debugger = _context->GetDebugger();
	_memoryDumper = _debugger->GetMemoryDumper().get();
	_console = _debugger->GetConsole().get();
	_ppu = _console->GetPpu().get();
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
	   { "stop", LuaApi::Stop },
		{ "breakExecution", LuaApi::Break },
		{ "resume", LuaApi::Resume },
		{ "execute", LuaApi::Execute },
		{ "rewind", LuaApi::Rewind },
		{ "takeScreenshot", LuaApi::TakeScreenshot },
		{ "isKeyPressed", LuaApi::IsKeyPressed },
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

	//Expose SnesMemoryType enum as "emu.memType"
	lua_pushliteral(lua, "memType");
	lua_newtable(lua);
	lua_pushintvalue(cpu, SnesMemoryType::CpuMemory);
	lua_pushintvalue(spc, SnesMemoryType::SpcMemory);
	lua_pushintvalue(cgram, SnesMemoryType::CGRam);
	lua_pushintvalue(vram, SnesMemoryType::VideoRam);
	lua_pushintvalue(oam, SnesMemoryType::SpriteRam);
	lua_pushintvalue(prgRom, SnesMemoryType::PrgRom);
	lua_pushintvalue(workRam, SnesMemoryType::WorkRam);
	lua_pushintvalue(saveRam, SnesMemoryType::SaveRam);
	lua_pushintvalue(cpuDebug, SnesMemoryType::CpuMemory | 0x100);
	lua_pushintvalue(spcDebug, SnesMemoryType::SpcMemory | 0x100);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "memCallbackType");
	lua_newtable(lua);
	lua_pushintvalue(read, CallbackType::CpuRead);
	lua_pushintvalue(write, CallbackType::CpuWrite);
	lua_pushintvalue(exec, CallbackType::CpuExec);
	lua_settable(lua, -3);

	lua_pushliteral(lua, "counterMemType");
	lua_newtable(lua);
	lua_pushintvalue(prgRom, SnesMemoryType::PrgRom);
	lua_pushintvalue(workRam, SnesMemoryType::WorkRam);
	lua_pushintvalue(saveRam, SnesMemoryType::SaveRam);
	//TODO add more
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
	//TODO
	/*lua_pushintvalue(codeBreak, EventType::CodeBreak);
	lua_pushintvalue(stateLoaded, EventType::StateLoaded);
	lua_pushintvalue(stateSaved, EventType::StateSaved);
	*/
	lua_settable(lua, -3);

	lua_pushliteral(lua, "stepType");
	lua_newtable(lua);
	lua_pushintvalue(cpuInstructions, StepType::CpuStep);
	lua_pushintvalue(spcInstructions, StepType::SpcStep);
	lua_pushintvalue(ppuCycles, StepType::PpuStep);
	lua_settable(lua, -3);

	return 1;
}

int LuaApi::GetLabelAddress(lua_State *lua)
{
	LuaCallHelper l(lua);
	string label = l.ReadString();
	checkparams();
	errorCond(label.length() == 0, "label cannot be empty");

	std::shared_ptr<LabelManager> lblMan = _debugger->GetLabelManager();
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
	SnesMemoryType memType = (SnesMemoryType)(type & 0xFF);
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
	SnesMemoryType memType = (SnesMemoryType)(type & 0xFF);
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
	SnesMemoryType memType = (SnesMemoryType)(type & 0xFF);
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
	SnesMemoryType memType = (SnesMemoryType)(type & 0xFF);
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
	
	AddressInfo relAddress { address, SnesMemoryType::CpuMemory };
	int32_t prgRomOffset = _debugger->GetAbsoluteAddress(relAddress).Address;
	l.Return(prgRomOffset);
	return l.ReturnCount();
}

int LuaApi::RegisterMemoryCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(4);
	int32_t endAddr = l.ReadInteger(-1);
	int32_t startAddr = l.ReadInteger();
	CallbackType type = (CallbackType)l.ReadInteger();
	int reference = l.GetReference();
	checkminparams(3);

	if(endAddr == -1) {
		endAddr = startAddr;
	}

	errorCond(startAddr > endAddr, "start address must be <= end address");
	errorCond(type < CallbackType::CpuRead || type > CallbackType::CpuExec, "the specified type is invalid");
	errorCond(reference == LUA_NOREF, "the specified function could not be found");
	_context->RegisterMemoryCallback(type, startAddr, endAddr, reference);
	_context->Log("Registered memory callback from $" + HexUtilities::ToHex((uint32_t)startAddr) + " to $" + HexUtilities::ToHex((uint32_t)endAddr));
	l.Return(reference);
	return l.ReturnCount();
}

int LuaApi::UnregisterMemoryCallback(lua_State *lua)
{
	LuaCallHelper l(lua);
	l.ForceParamCount(4);

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
	_context->UnregisterMemoryCallback(type, startAddr, endAddr, reference);
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
	errorCond(type < EventType::Reset || type >= EventType::EventTypeSize, "the specified type is invalid");
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

	int startFrame = _ppu->GetFrameCount() + displayDelay;
	_console->GetDebugHud()->DrawString(x, y, text, color, backColor, frameCount, startFrame);

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

	int startFrame = _ppu->GetFrameCount() + displayDelay;
	_console->GetDebugHud()->DrawLine(x, y, x2, y2, color, frameCount, startFrame);

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

	int startFrame = _ppu->GetFrameCount() + displayDelay;
	_console->GetDebugHud()->DrawPixel(x, y, color, frameCount, startFrame);

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

	int startFrame = _ppu->GetFrameCount() + displayDelay;
	_console->GetDebugHud()->DrawRectangle(x, y, width, height, color, fill, frameCount, startFrame);

	return l.ReturnCount();
}

int LuaApi::ClearScreen(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();

	_console->GetDebugHud()->ClearScreen();
	return l.ReturnCount();
}

int LuaApi::GetScreenBuffer(lua_State *lua)
{
	LuaCallHelper l(lua);

	lua_newtable(lua);
	for(int y = 0; y < 239; y++) {
		for(int x = 0; x < 256; x++) {
			lua_pushinteger(lua, DefaultVideoFilter::ToArgb(*(_ppu->GetScreenBuffer() + y * 1024 + x * 2)) & 0xFFFFFF);
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
	
	int startFrame = _ppu->GetFrameCount();
	_console->GetDebugHud()->DrawScreenBuffer(pixels, startFrame);

	return l.ReturnCount();
}

int LuaApi::GetPixel(lua_State *lua)
{
	LuaCallHelper l(lua);
	int y = l.ReadInteger();
	int x = l.ReadInteger();
	checkparams();
	errorCond(x < 0 || x > 255 || y < 0 || y > 238, "invalid x,y coordinates (must be between 0-255, 0-238)");

	//Ignores intensify & grayscale bits
	l.Return(DefaultVideoFilter::ToArgb(*(_ppu->GetScreenBuffer() + y * 1024 + x * 2)) & 0xFFFFFF);
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
	_console->Reset();
	return l.ReturnCount();
}

int LuaApi::Stop(lua_State *lua)
{
	LuaCallHelper l(lua);
	int32_t stopCode = l.ReadInteger(0);
	checkminparams(0);
	_console->Stop(stopCode);
	return l.ReturnCount();
}

int LuaApi::Break(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	_debugger->Step(1);
	return l.ReturnCount();
}

int LuaApi::Resume(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	_debugger->Run();
	return l.ReturnCount();
}

int LuaApi::Execute(lua_State *lua)
{
	LuaCallHelper l(lua);
	StepType type = (StepType)l.ReadInteger();
	int count = l.ReadInteger();
	checkparams();
	errorCond(count <= 0, "count must be >= 1");
	errorCond(type != StepType::CpuStep && type != StepType::SpcStep && type != StepType::PpuStep, "type is invalid");

	_debugger->Step(count, type);

	return l.ReturnCount();
}

int LuaApi::Rewind(lua_State *lua)
{
	LuaCallHelper l(lua);
	int seconds = l.ReadInteger();
	checkparams();
	checksavestateconditions();
	errorCond(seconds <= 0, "seconds must be >= 1");
	_console->GetRewindManager()->RewindSeconds(seconds);
	return l.ReturnCount();
}

int LuaApi::TakeScreenshot(lua_State *lua)
{
	LuaCallHelper l(lua);
	checkparams();
	stringstream ss;
	_console->GetVideoDecoder()->TakeScreenshot(ss);
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
	errorCond(port < 0 || port > 3, "Invalid port number - must be between 0 to 3");

	shared_ptr<SnesController> controller = std::dynamic_pointer_cast<SnesController>(_console->GetControlManager()->GetControlDevice(port));
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
	SnesMemoryType memoryType = (SnesMemoryType)l.ReadInteger();
	errorCond(operationType >= MemoryOperationType::ExecOperand, "Invalid operation type");
	errorCond(memoryType >= SnesMemoryType::Register, "Invalid memory type");
	checkparams();

	uint32_t size = 0;
	vector<uint32_t> counts;
	counts.resize(_memoryDumper->GetMemorySize(memoryType), 0);
	_debugger->GetMemoryAccessCounter()->GetAccessCounts(0, size, memoryType, operationType, counts.data());

	lua_newtable(lua);
	for(uint32_t i = 0; i < size; i++) {
		lua_pushinteger(lua, counts[i]);
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

	RomInfo romInfo = _console->GetRomInfo();

	lua_newtable(lua);
	lua_pushstringvalue(name, romInfo.RomFile.GetFileName());
	lua_pushstringvalue(path, romInfo.RomFile.GetFilePath());
	lua_pushstringvalue(fileSha1Hash, _console->GetCartridge()->GetSha1Hash());

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
	DebugState state;
	_debugger->GetState(state);

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
	lua_pushintvalue(scanline, state.Ppu.HClock);
	lua_endtable(); //end ppu

	lua_starttable("spc");
	lua_pushintvalue(a, state.Spc.A);
	lua_pushintvalue(pc, state.Spc.PC);
	lua_pushintvalue(status, state.Spc.PS);
	lua_pushintvalue(sp, state.Spc.SP);
	lua_pushintvalue(x, state.Spc.X);
	lua_pushintvalue(y, state.Spc.Y);
	lua_endtable(); //end spc
	
	return 1;
}
