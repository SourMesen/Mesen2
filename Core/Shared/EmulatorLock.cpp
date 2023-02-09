#include "pch.h"
#include "Shared/EmulatorLock.h"
#include "Shared/Emulator.h"
#include "Shared/DebuggerRequest.h"
#include "Debugger/DebugBreakHelper.h"

EmulatorLock::EmulatorLock(Emulator *emu, bool allowDebuggerLock)
{
	_emu = emu;

	if(_emu->_runLock.IsLockedByCurrentThread()) {
		_emu->Lock();
	} else {
		if(allowDebuggerLock) {
			_debugger.reset(new DebuggerRequest(emu->GetDebugger(false)));
			if(_debugger->GetDebugger()) {
				_breakHelper.reset(new DebugBreakHelper(_debugger->GetDebugger(), true));
			} else {
				_debugger.reset();
				_emu->Lock();
			}
		} else {
			_emu->Lock();
		}
	}
}

EmulatorLock::~EmulatorLock()
{
	if(_debugger) {
		_breakHelper.reset();
	} else {
		_emu->Unlock();
	}
}
