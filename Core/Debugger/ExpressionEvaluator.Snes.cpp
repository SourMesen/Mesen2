#include "stdafx.h"
#include "ExpressionEvaluator.h"
#include "SNES/SnesCpuTypes.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetSnesTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "a", EvalValues::RegA },
		{ "x", EvalValues::RegX },
		{ "y", EvalValues::RegY },
		{ "ps", EvalValues::RegPS },
		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },
		{ "irq", EvalValues::Irq },
		{ "nmi", EvalValues::Nmi }
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetSnesTokenValue(int64_t token, EvalResultType& resultType, SnesCpuState& s)
{
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegX: return s.X;
		case EvalValues::RegY: return s.Y;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPS: return s.PS;
		case EvalValues::RegPC: return (s.K << 16) | s.PC;
		case EvalValues::Nmi:
			resultType = EvalResultType::Boolean;
			return s.NmiFlag;
		case EvalValues::Irq:
			resultType = EvalResultType::Boolean;
			return s.IrqSource != 0;

		default: return 0;
	}
}