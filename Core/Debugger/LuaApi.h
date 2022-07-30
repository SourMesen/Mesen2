#pragma once
#include "stdafx.h"
#include "Shared/CpuType.h"
#include "MemoryType.h"

struct lua_State;
class ScriptingContext;
class Debugger;
class Emulator;
class SnesConsole;
class SnesPpu;
class MemoryDumper;

class LuaApi
{
public:
	static void SetContext(ScriptingContext *context);
	static int GetLibrary(lua_State *lua);

	static void LuaPushIntValue(lua_State* lua, string name, int value);

	static int GetLabelAddress(lua_State *lua);

	static int ReadMemory(lua_State *lua);
	static int WriteMemory(lua_State *lua);
	static int ReadMemoryWord(lua_State *lua);
	static int WriteMemoryWord(lua_State *lua);
	static int GetPrgRomOffset(lua_State *lua);
	//static int RevertPrgChrChanges(lua_State *lua);

	static int RegisterMemoryCallback(lua_State *lua);
	static int UnregisterMemoryCallback(lua_State *lua);
	static int RegisterEventCallback(lua_State *lua);
	static int UnregisterEventCallback(lua_State *lua);
	
	static int DrawString(lua_State *lua);
	static int DrawLine(lua_State *lua);
	static int DrawPixel(lua_State *lua);
	static int DrawRectangle(lua_State *lua);
	static int ClearScreen(lua_State *lua);
	static int GetScreenBuffer(lua_State *lua);
	static int SetScreenBuffer(lua_State *lua);
	static int GetPixel(lua_State *lua);
	static int GetMouseState(lua_State *lua);

	static int Log(lua_State *lua);
	static int DisplayMessage(lua_State *lua);
	
	static int Reset(lua_State *lua);
	//static int Stop(lua_State *lua);
	static int Break(lua_State *lua);
	static int Resume(lua_State *lua);
	static int Execute(lua_State *lua);
	static int Rewind(lua_State *lua);

	static int TakeScreenshot(lua_State *lua);
	/*static int SaveSavestate(lua_State *lua);
	static int LoadSavestate(lua_State *lua);

	static int SaveSavestateAsync(lua_State *lua);
	static int LoadSavestateAsync(lua_State *lua);
	static int GetSavestateData(lua_State *lua);
	static int ClearSavestateData(lua_State *lua);*/

	static int IsKeyPressed(lua_State *lua);

	static int GetInput(lua_State *lua);
	static int SetInput(lua_State *lua);

	/*static int AddCheat(lua_State *lua);
	static int ClearCheats(lua_State *lua);*/

	static int GetScriptDataFolder(lua_State *lua);
	static int GetRomInfo(lua_State *lua);
	static int GetLogWindowLog(lua_State *lua);

	//static int SetState(lua_State *lua);
	static int GetState(lua_State *lua);

	static int GetAccessCounters(lua_State *lua);
	static int ResetAccessCounters(lua_State *lua);

private:
	static Emulator* _emu;
	static SnesPpu* _ppu;
	static Debugger* _debugger;
	static MemoryDumper* _memoryDumper;
	static ScriptingContext* _context;

	static CpuType _defaultCpuType;
	static MemoryType _defaultMemType;
};
