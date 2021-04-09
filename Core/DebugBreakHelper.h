#pragma once
#include "stdafx.h"
#include "Debugger.h"
#include "Emulator.h"

class DebugBreakHelper
{
private:
	Debugger * _debugger;
	bool _needBreak = false;

public:
	DebugBreakHelper(Debugger* debugger)
	{
		_debugger = debugger;

		_needBreak = debugger->GetEmulator()->GetEmulationThreadId() != std::this_thread::get_id();

		if(_needBreak) {
			//Only attempt to break if this is done in a thread other than the main emulation thread (and the debugger is active)
			debugger->BreakRequest(false);
			if(!debugger->IsExecutionStopped()) {
				while(!debugger->IsExecutionStopped()) {}
			}
		}
	}

	~DebugBreakHelper()
	{
		if(_needBreak) {
			_debugger->BreakRequest(true);
		}
	}
};