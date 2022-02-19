#pragma once

#include "stdafx.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define NesCpu DummyNesCpu
#include "NES/NesCpu.h"
#include "NES/NesCpu.cpp"
#undef NesCpu
#undef DUMMYCPU
