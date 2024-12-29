#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define ArmV3Cpu DummyArmV3Cpu
#include "SNES/Coprocessors/ST018/ArmV3Cpu.h"
#undef ArmV3Cpu
#undef DUMMYCPU
