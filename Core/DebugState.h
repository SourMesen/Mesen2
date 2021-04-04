#pragma once

#include "SNES/CpuTypes.h"
#include "SNES/PpuTypes.h"
#include "SNES/SpcTypes.h"
#include "SNES/NecDspTypes.h"
#include "SNES/GsuTypes.h"
#include "SNES/Cx4Types.h"
#include "SNES/Sa1Types.h"
#include "Gameboy/GbTypes.h"
#include "SNES/InternalRegisterTypes.h"
#include "SNES/DmaControllerTypes.h"

struct DebugState
{
	uint64_t MasterClock;
	CpuState Cpu;
	PpuState Ppu;
	SpcState Spc;
	DspState Dsp;
	NecDspState NecDsp;
	DebugSa1State Sa1;
	GsuState Gsu;
	Cx4State Cx4;

	GbState Gameboy;

	DmaChannelConfig DmaChannels[8];
	InternalRegisterState InternalRegs;
	AluState Alu;
};
