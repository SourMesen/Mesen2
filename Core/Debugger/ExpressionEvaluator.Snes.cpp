#include "stdafx.h"
#include "ExpressionEvaluator.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesPpuTypes.h"
#include "SNES/Debugger/SnesDebugger.h"

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
		{ "nmi", EvalValues::Nmi },
		{ "frame", EvalValues::PpuFrameCount },
		{ "hclock", EvalValues::PpuHClock },
		{ "cycle", EvalValues::PpuCycle },
		{ "scanline", EvalValues::PpuScanline },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetSnesTokenValue(int64_t token, EvalResultType& resultType)
{
	auto getPpuState = [this]() -> SnesPpuState {
		SnesPpuState ppu;
		((SnesDebugger*)_cpuDebugger)->GetPpuState(ppu);
		return ppu;
	};

	SnesCpuState& s = (SnesCpuState&)((SnesDebugger*)_cpuDebugger)->GetState();
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
		
		case EvalValues::PpuFrameCount: return getPpuState().FrameCount;
		case EvalValues::PpuCycle: return getPpuState().Cycle;
		case EvalValues::PpuHClock: return getPpuState().HClock;
		case EvalValues::PpuScanline: return getPpuState().Scanline;

		default: return 0;
	}
}