#include "stdafx.h"
#include "ConsoleLock.h"
#include "Console.h"
#include "Debugger.h"

ConsoleLock::ConsoleLock(Console *console)
{
	_console = console;
	_console->Lock();
}

ConsoleLock::~ConsoleLock()
{
	_console->Unlock();
}
