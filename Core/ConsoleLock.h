#pragma once
#include "stdafx.h"

class Console;
class Debugger;

class ConsoleLock
{
private:
	Console *_console = nullptr;
	shared_ptr<Debugger> _debugger;

public:
	ConsoleLock(Console *console);
	~ConsoleLock();
};