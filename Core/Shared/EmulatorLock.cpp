#include "pch.h"
#include "Shared/EmulatorLock.h"
#include "Shared/Emulator.h"

EmulatorLock::EmulatorLock(Emulator *emu)
{
	_emu = emu;
	_emu->Lock();
}

EmulatorLock::~EmulatorLock()
{
	_emu->Unlock();
}
