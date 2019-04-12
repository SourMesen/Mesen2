#include "stdafx.h"
#include "ConsoleLock.h"
#include "Console.h"
#include "Debugger.h"

ConsoleLock::ConsoleLock(Console *console)
{
	_console = console;

	_debugger = _console->GetDebugger(false);
	if(_debugger) {
		_debugger->SuspendDebugger(false);
	}
	_console->Lock();
}

ConsoleLock::~ConsoleLock()
{
	if(_debugger) {
		_debugger->SuspendDebugger(true);
	}
	_console->Unlock();
}
