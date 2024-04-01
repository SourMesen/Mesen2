#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define GbaCpu DummyGbaCpu
#include "GBA/GbaCpu.h"
#undef GbaCpu
#undef DUMMYCPU
