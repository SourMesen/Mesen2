#include "pch.h"
#include "EmulatorLock.h"
#include "Emulator.h"

EmulatorLock::EmulatorLock(Emulator *emu)
{
	_emu = emu;
	_emu->Lock();
}

EmulatorLock::~EmulatorLock()
{
	_emu->Unlock();
}
