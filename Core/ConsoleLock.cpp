#include "stdafx.h"
#include "ConsoleLock.h"
#include "Console.h"

ConsoleLock::ConsoleLock(Console *console)
{
	_console = console;
	_console->Lock();
}

ConsoleLock::~ConsoleLock()
{
	_console->Unlock();
}
