#pragma once

#include "pch.h"
#include "Debugger/DebugTypes.h"

#define DUMMYCPU
#define SmsCpu DummySmsCpu
#include "SMS/SmsCpu.h"
#undef SmsCpu
#undef DUMMYCPU
