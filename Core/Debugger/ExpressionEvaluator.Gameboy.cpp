#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Gameboy/Debugger/GbDebugger.h"
#include "Gameboy/GbTypes.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetGameboyTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "a", EvalValues::RegA },
		{ "b", EvalValues::RegB },
		{ "c", EvalValues::RegC },
		{ "d", EvalValues::RegD },
		{ "e", EvalValues::RegE },
		{ "f", EvalValues::RegF },
		{ "h", EvalValues::RegH },
		{ "l", EvalValues::RegL },
		{ "af", EvalValues::RegAF },
		{ "bc", EvalValues::RegBC },
		{ "de", EvalValues::RegDE },
		{ "hl", EvalValues::RegHL },
		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },

		{ "frame", EvalValues::PpuFrameCount },
		{ "cycle", EvalValues::PpuCycle },
		{ "scanline", EvalValues::PpuScanline }, 
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetGameboyTokenValue(int64_t token, EvalResultType& resultType)
{
	auto ppu = [this]() -> GbPpuState {
		GbPpuState ppu;
		((GbDebugger*)_cpuDebugger)->GetPpuState(ppu);
		return ppu;
	};

	GbCpuState& s = (GbCpuState&)((GbDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegB: return s.B;
		case EvalValues::RegC: return s.C;
		case EvalValues::RegD: return s.D;
		case EvalValues::RegE: return s.E;
		case EvalValues::RegF: return s.Flags;
		case EvalValues::RegH: return s.H;
		case EvalValues::RegL: return s.L;
		case EvalValues::RegAF: return (s.A << 8) | s.Flags;
		case EvalValues::RegBC: return (s.B << 8) | s.C;
		case EvalValues::RegDE: return (s.D << 8) | s.E;
		case EvalValues::RegHL: return (s.H << 8) | s.L;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPC: return s.PC;

		case EvalValues::PpuFrameCount: return ppu().FrameCount;
		case EvalValues::PpuCycle: return ppu().Cycle;
		case EvalValues::PpuScanline: return ppu().Scanline;

		default: return 0;
	}
}