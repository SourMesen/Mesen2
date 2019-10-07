#pragma once
#include "stdafx.h"

class Console;
class Debugger;

class ConsoleLock
{
private:
	Console *_console = nullptr;

public:
	ConsoleLock(Console *console);
	~ConsoleLock();
};