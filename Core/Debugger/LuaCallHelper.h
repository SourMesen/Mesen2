#pragma once
#include "pch.h"
#include "Lua/lua.hpp"

template<typename T>
struct Nullable
{
	bool HasValue = false;
	T Value = {};
};

class LuaCallHelper
{
private:
	int _stackSize = 0;
	int _paramCount = 0;
	int _returnCount = 0;
	lua_State* _lua;

public:
	LuaCallHelper(lua_State* lua);

	void ForceParamCount(int paramCount);
	bool CheckParamCount(int minParamCount = -1);

	double ReadDouble();
	bool ReadBool(bool defaultValue = false);
	uint32_t ReadInteger(uint32_t defaultValue = 0);
	string ReadString();
	int GetReference();

	Nullable<bool> ReadOptionalBool();
	Nullable<int32_t> ReadOptionalInteger();

	void Return(bool value);
	void Return(int value);
	void Return(uint32_t value);
	void Return(string value);

	int ReturnCount();
};