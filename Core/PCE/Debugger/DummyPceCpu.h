#pragma once

#include "stdafx.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define PceCpu DummyPceCpu
#include "PCE/PceCpu.h"
#undef PceCpu
#undef DUMMYCPU
