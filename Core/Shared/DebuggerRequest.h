#pragma once
#include "pch.h"

class Debugger;
class Emulator;

class DebuggerRequest
{
private:
	shared_ptr<Debugger> _debugger;
	Emulator* _emu = nullptr;

public:
	DebuggerRequest(Emulator* emu);
	DebuggerRequest(const DebuggerRequest& copy);
	~DebuggerRequest();

	Debugger* GetDebugger() { return _debugger.get(); }
};
