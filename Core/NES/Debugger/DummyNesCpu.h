#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define NesCpu DummyNesCpu
#include "NES/NesCpu.h"
#undef NesCpu
#undef DUMMYCPU
