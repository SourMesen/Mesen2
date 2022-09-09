#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/Debugger/Cx4Debugger.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetCx4Tokens()
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
		{ "pb", EvalValues::RegPB },
		{ "pc", EvalValues::RegPC },
		{ "a", EvalValues::RegA },
		{ "p", EvalValues::RegP },
		{ "sp", EvalValues::RegSP },
		{ "mult", EvalValues::RegMult },
		{ "negative", EvalValues::RegPS_Negative },
		{ "zero", EvalValues::RegPS_Zero },
		{ "carry", EvalValues::RegPS_Carry },
		{ "overflow", EvalValues::RegPS_Overflow },
		{ "irq", EvalValues::RegPS_Interrupt },
		{ "mdr", EvalValues::RegMDR },
		{ "mar", EvalValues::RegMAR },
		{ "dpr", EvalValues::RegDPR },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetCx4TokenValue(int64_t token, EvalResultType& resultType)
{
	Cx4State& s = (Cx4State&)((Cx4Debugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::R0: return s.Regs[0];
		case EvalValues::R1: return s.Regs[1];
		case EvalValues::R2: return s.Regs[2];
		case EvalValues::R3: return s.Regs[3];
		case EvalValues::R4: return s.Regs[4];
		case EvalValues::R5: return s.Regs[5];
		case EvalValues::R6: return s.Regs[6];
		case EvalValues::R7: return s.Regs[7];
		case EvalValues::R8: return s.Regs[8];
		case EvalValues::R9: return s.Regs[9];
		case EvalValues::R10: return s.Regs[10];
		case EvalValues::R11: return s.Regs[11];
		case EvalValues::R12: return s.Regs[12];
		case EvalValues::R13: return s.Regs[13];
		case EvalValues::R14: return s.Regs[14];
		case EvalValues::R15: return s.Regs[15];

		case EvalValues::RegPB: return s.PB;
		case EvalValues::RegPC: return s.PC;
		case EvalValues::RegA: return s.A;
		case EvalValues::RegP: return s.P;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegMult: return s.Mult;

		case EvalValues::RegPS_Negative: return ReturnBool(s.Negative, resultType);
		case EvalValues::RegPS_Zero: return ReturnBool(s.Zero, resultType);
		case EvalValues::RegPS_Carry: return ReturnBool(s.Carry, resultType);
		case EvalValues::RegPS_Overflow: return ReturnBool(s.Overflow, resultType);
		case EvalValues::RegPS_Interrupt: return ReturnBool(s.IrqFlag, resultType);

		case EvalValues::RegMDR: return s.MemoryDataReg;
		case EvalValues::RegMAR: return s.MemoryAddressReg;
		case EvalValues::RegDPR: return s.DataPointerReg;

		default: return 0;
	}
}