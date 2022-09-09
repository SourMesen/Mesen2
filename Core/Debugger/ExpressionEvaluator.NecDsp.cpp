#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/Coprocessors/DSP/NecDspTypes.h"
#include "SNES/Debugger/NecDspDebugger.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetNecDspTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "a", EvalValues::RegA },
		{ "b", EvalValues::RegB },
		{ "tr", EvalValues::RegTR },
		{ "trb", EvalValues::RegTRB },
		{ "rp", EvalValues::RegRP },
		{ "dp", EvalValues::RegDP },
		{ "dr", EvalValues::RegDR },
		{ "sr", EvalValues::RegSR },
		{ "k", EvalValues::RegK },
		{ "l", EvalValues::RegL },
		{ "m", EvalValues::RegM },
		{ "n", EvalValues::RegN },
		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetNecDspTokenValue(int64_t token, EvalResultType& resultType)
{
	NecDspState& s = (NecDspState&)((NecDspDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegB: return s.B;
		case EvalValues::RegTR: return s.TR;
		case EvalValues::RegTRB: return s.TRB;
		case EvalValues::RegRP: return s.RP;
		case EvalValues::RegDP: return s.DP;
		case EvalValues::RegDR: return s.DR;
		case EvalValues::RegSR: return s.SR;
		case EvalValues::RegK: return s.K;
		case EvalValues::RegL: return s.L;
		case EvalValues::RegM: return s.M;
		case EvalValues::RegN: return s.N;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPC: return s.PC;
		default: return 0;
	}
}