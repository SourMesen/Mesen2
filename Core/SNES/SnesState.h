#include "stdafx.h"
#include "Shared/BaseState.h"
#include "SNES/CpuTypes.h"
#include "SNES/PpuTypes.h"
#include "SNES/SpcTypes.h"
#include "SNES/Coprocessors/DSP/NecDspTypes.h"
#include "SNES/Coprocessors/SA1/Sa1Types.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"
#include "SNES/DmaControllerTypes.h"
#include "SNES/InternalRegisterTypes.h"
#include "SNES/AluMulDiv.h"

struct SnesState : public BaseState
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

	DmaChannelConfig DmaChannels[8];
	InternalRegisterState InternalRegs;
	AluState Alu;
};