#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/DebuggerRequest.h"

DebuggerRequest::DebuggerRequest(Emulator* emu)
{
	if(emu) {
		_emu = emu;
		_debugger = _emu->_debugger.lock();
		_emu->_debugRequestCount++;
	}
}

DebuggerRequest::DebuggerRequest(const DebuggerRequest& copy)
{
	_emu = copy._emu;
	_debugger = copy._emu->_debugger.lock();
	_emu->_debugRequestCount++;
}

DebuggerRequest::~DebuggerRequest()
{
	if(_emu) {
		_emu->_debugRequestCount--;
	}
}
