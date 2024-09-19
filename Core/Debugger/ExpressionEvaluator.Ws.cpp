#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "WS/Debugger/WsDebugger.h"
#include "WS/WsTypes.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetWsTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "ax", EvalValues::RegAX },
		{ "bx", EvalValues::RegBX },
		{ "cx", EvalValues::RegCX },
		{ "dx", EvalValues::RegDX },

		{ "al", EvalValues::RegAL },
		{ "bl", EvalValues::RegBL },
		{ "cl", EvalValues::RegCL },
		{ "dl", EvalValues::RegDL },

		{ "ah", EvalValues::RegAH },
		{ "bh", EvalValues::RegBH },
		{ "ch", EvalValues::RegCH },
		{ "dh", EvalValues::RegDH },

		{ "cs", EvalValues::RegCS },
		{ "ds", EvalValues::RegDS },
		{ "es", EvalValues::RegES },
		{ "ss", EvalValues::RegSS },

		{ "si", EvalValues::RegSI },
		{ "di", EvalValues::RegDI },
		{ "bp", EvalValues::RegBP },
		{ "ip", EvalValues::RegIP },
		
		{ "f", EvalValues::RegF },

		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },

		{ "frame", EvalValues::PpuFrameCount },
		{ "cycle", EvalValues::PpuCycle },
		{ "scanline", EvalValues::PpuScanline },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetWsTokenValue(int64_t token, EvalResultType& resultType)
{
	auto ppu = [this]() -> WsPpuState {
		WsPpuState ppu;
		((WsDebugger*)_cpuDebugger)->GetPpuState(ppu);
		return ppu;
	};

	WsCpuState& s = (WsCpuState&)((WsDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::RegAX: return s.AX;
		case EvalValues::RegBX: return s.BX;
		case EvalValues::RegCX: return s.CX;
		case EvalValues::RegDX: return s.DX;
		case EvalValues::RegAL: return s.AX & 0xFF;
		case EvalValues::RegBL: return s.BX & 0xFF;
		case EvalValues::RegCL: return s.CX & 0xFF;
		case EvalValues::RegDL: return s.DX & 0xFF;
		case EvalValues::RegAH: return s.AX >> 8;
		case EvalValues::RegBH: return s.BX >> 8;
		case EvalValues::RegCH: return s.CX >> 8;
		case EvalValues::RegDH: return s.DX >> 8;

		case EvalValues::RegCS: return s.CS;
		case EvalValues::RegDS: return s.DS;
		case EvalValues::RegES: return s.ES;
		case EvalValues::RegSS: return s.SS;
		case EvalValues::RegSI: return s.SI;
		case EvalValues::RegDI: return s.DI;
		case EvalValues::RegBP: return s.BP;
		case EvalValues::RegIP: return s.IP;
		
		case EvalValues::RegF: return s.Flags.Get();

		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPC: return (s.CS << 4) + s.IP;

		case EvalValues::PpuFrameCount: return ppu().FrameCount;
		case EvalValues::PpuCycle: return ppu().Cycle;
		case EvalValues::PpuScanline: return ppu().Scanline;

		default: return 0;
	}
}