#pragma once
#include "pch.h"

class Emulator;
class DebuggerRequest;
class DebugBreakHelper;

class EmulatorLock
{
private:
	Emulator* _emu = nullptr;
	unique_ptr<DebuggerRequest> _debugger;
	unique_ptr<DebugBreakHelper> _breakHelper;

public:
	EmulatorLock(Emulator* emulator, bool allowDebuggerLock);
	~EmulatorLock();
};