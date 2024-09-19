#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define WsCpu DummyWsCpu
#include "WS/WsCpu.h"
#undef WsCpu
#undef DUMMYCPU
