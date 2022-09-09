#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/Debugger/GsuDebugger.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetGsuTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "r0", EvalValues::R0 },
		{ "r1", EvalValues::R1 },
		{ "r2", EvalValues::R2 },
		{ "r3", EvalValues::R3 },
		{ "r4", EvalValues::R4 },
		{ "r5", EvalValues::R5 },
		{ "r6", EvalValues::R6 },
		{ "r7", EvalValues::R7 },
		{ "r8", EvalValues::R8 },
		{ "r9", EvalValues::R9 },
		{ "r10", EvalValues::R10 },
		{ "r11", EvalValues::R11 },
		{ "r12", EvalValues::R12 },
		{ "r13", EvalValues::R13 },
		{ "r14", EvalValues::R14 },
		{ "r15", EvalValues::R15 },
		{ "srcreg", EvalValues::SrcReg },
		{ "dstreg", EvalValues::DstReg },
		{ "sfr", EvalValues::SFR },
		{ "pbr", EvalValues::PBR },
		{ "rombr", EvalValues::RomBR },
		{ "rambr", EvalValues::RamBR },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetGsuTokenValue(int64_t token, EvalResultType& resultType)
{
	GsuState& s = (GsuState&)((GsuDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::R0: return s.R[0];
		case EvalValues::R1: return s.R[1];
		case EvalValues::R2: return s.R[2];
		case EvalValues::R3: return s.R[3];
		case EvalValues::R4: return s.R[4];
		case EvalValues::R5: return s.R[5];
		case EvalValues::R6: return s.R[6];
		case EvalValues::R7: return s.R[7];
		case EvalValues::R8: return s.R[8];
		case EvalValues::R9: return s.R[9];
		case EvalValues::R10: return s.R[10];
		case EvalValues::R11: return s.R[11];
		case EvalValues::R12: return s.R[12];
		case EvalValues::R13: return s.R[13];
		case EvalValues::R14: return s.R[14];
		case EvalValues::R15: return s.R[15];

		case EvalValues::SrcReg: return s.SrcReg;
		case EvalValues::DstReg: return s.DestReg;

		case EvalValues::SFR: return (s.SFR.GetFlagsHigh() << 8) | s.SFR.GetFlagsLow();
		case EvalValues::PBR: return s.ProgramBank;
		case EvalValues::RomBR: return s.RomBank;
		case EvalValues::RamBR: return s.RamBank;

		default: return 0;
	}
}