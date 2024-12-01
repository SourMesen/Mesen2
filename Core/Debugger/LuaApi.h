#pragma once
#include "pch.h"
#include "Shared/CpuType.h"
#include "Shared/SettingTypes.h"
#include "Shared/MemoryType.h"

struct lua_State;
class ScriptingContext;
class Debugger;
class Emulator;
class MemoryDumper;
class DebugHud;
class BaseVideoFilter;

class LuaApi
{
public:
	static void SetContext(ScriptingContext *context);
	static int GetLibrary(lua_State *lua);

	static void LuaPushIntValue(lua_State* lua, string name, int value);

	static DebugHud* GetHud();

	static int SelectDrawSurface(lua_State* lua);

	static int GetMemorySize(lua_State* lua);

	static int ReadMemory(lua_State *lua);
	static int WriteMemory(lua_State *lua);
	static int ReadMemory16(lua_State *lua);
	static int WriteMemory16(lua_State *lua);
	static int ReadMemory32(lua_State* lua);
	static int WriteMemory32(lua_State* lua);

	static int GetLabelAddress(lua_State* lua);
	static int ConvertAddress(lua_State *lua);

	static int RegisterMemoryCallback(lua_State *lua);
	static int UnregisterMemoryCallback(lua_State *lua);
	static int RegisterEventCallback(lua_State *lua);
	static int UnregisterEventCallback(lua_State *lua);

	static int MeasureString(lua_State* lua);
	static int DrawString(lua_State *lua);

	static int DrawLine(lua_State *lua);
	static int DrawPixel(lua_State *lua);
	static int DrawRectangle(lua_State *lua);
	static int ClearScreen(lua_State *lua);
	
	static int GetScreenSize(lua_State* lua);
	static int GetDrawSurfaceSize(lua_State* lua);
	static int GetScreenBuffer(lua_State *lua);
	static int SetScreenBuffer(lua_State *lua);

	static int GetPixel(lua_State *lua);
	static int GetMouseState(lua_State *lua);

	static int Log(lua_State *lua);
	static int DisplayMessage(lua_State *lua);
	
	static int Reset(lua_State *lua);
	static int Stop(lua_State *lua);
	static int BreakExecution(lua_State *lua);
	static int Resume(lua_State *lua);
	static int Step(lua_State *lua);
	static int Rewind(lua_State *lua);

	static int TakeScreenshot(lua_State *lua);
	
	static int CreateSavestate(lua_State *lua);
	static int LoadSavestate(lua_State *lua);

	static int IsKeyPressed(lua_State *lua);

	static int GetInput(lua_State *lua);
	static int SetInput(lua_State *lua);

	static int AddCheat(lua_State *lua);
	static int ClearCheats(lua_State *lua);

	static int GetScriptDataFolder(lua_State *lua);
	static int GetRomInfo(lua_State *lua);
	static int GetLogWindowLog(lua_State *lua);

	static int SetState(lua_State *lua);
	static int GetState(lua_State *lua);

	static int GetAccessCounters(lua_State *lua);
	static int ResetAccessCounters(lua_State *lua);

	static int GetCdlData(lua_State* lua);

private:
	static FrameInfo InternalGetScreenSize();

	static Emulator* _emu;
	static Debugger* _debugger;
	static MemoryDumper* _memoryDumper;
	static ScriptingContext* _context;
	
	static std::pair<unique_ptr<BaseVideoFilter>, FrameInfo> GetRenderedFrame();
	template<typename T> static void GenerateEnumDefinition(lua_State* lua, string enumName, unordered_set<T> excludedValues = {});
};
