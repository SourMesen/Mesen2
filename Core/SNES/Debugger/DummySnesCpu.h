#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "SNES/MemoryMappings.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Coprocessors/SA1/Sa1.h"

#define DUMMYCPU
#define SnesCpu DummySnesCpu
#include "SNES/SnesCpu.h"
#undef SnesCpu
#undef DUMMYCPU
