#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define GbCpu DummyGbCpu
#include "Gameboy/GbCpu.h"
#undef GbCpu
#undef DUMMYCPU
