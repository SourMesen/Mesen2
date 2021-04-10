#pragma once
#include "stdafx.h"

class Emulator;

class EmulatorLock
{
private:
	Emulator* _emu = nullptr;

public:
	EmulatorLock(Emulator* emulator);
	~EmulatorLock();
};