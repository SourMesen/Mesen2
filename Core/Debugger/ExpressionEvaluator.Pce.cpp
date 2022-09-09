#include "pch.h"
#include "ExpressionEvaluator.h"
#include "PCE/PceTypes.h"
#include "PCE/Debugger/PceDebugger.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetPceTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "a", EvalValues::RegA },
		{ "x", EvalValues::RegX },
		{ "y", EvalValues::RegY },
		{ "ps", EvalValues::RegPS },
		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },
		/*{ "irq", EvalValues::Irq },
		{ "nmi", EvalValues::Nmi },*/
		{ "frame", EvalValues::PpuFrameCount },
		{ "cycle", EvalValues::PpuCycle },
		{ "scanline", EvalValues::PpuScanline },
		{ "sprite0hit", EvalValues::Sprite0Hit },
		{ "verticalblank", EvalValues::VerticalBlank },
		{ "spriteoverflow", EvalValues::SpriteOverflow },
		{ "pscarry", EvalValues::RegPS_Carry },
		{ "pszero", EvalValues::RegPS_Zero },
		{ "psinterrupt", EvalValues::RegPS_Interrupt },
		{ "psdecimal", EvalValues::RegPS_Decimal },
		{ "psmemory", EvalValues::RegPS_Memory },
		{ "psoverflow", EvalValues::RegPS_Overflow },
		{ "psnegative", EvalValues::RegPS_Negative },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetPceTokenValue(int64_t token, EvalResultType& resultType)
{
	auto ppu = [this]() -> PceVideoState {
		PceVideoState ppu;
		((PceDebugger*)_cpuDebugger)->GetPpuState(ppu);
		return ppu;
	};

	PceCpuState& s = (PceCpuState&)((PceDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegX: return s.X;
		case EvalValues::RegY: return s.Y;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPS: return s.PS;
		case EvalValues::RegPC: return s.PC;
		//TODO
		/*case EvalValues::Nmi: return ReturnBool(s.NMIFlag, resultType);
		case EvalValues::Irq:  return ReturnBool(s.IRQFlag, resultType);*/

		case EvalValues::PpuFrameCount: return ppu().Vdc.FrameCount;
		case EvalValues::PpuCycle: return ppu().Vdc.HClock;
		case EvalValues::PpuScanline: return ppu().Vdc.Scanline;

		case EvalValues::Sprite0Hit: return ReturnBool(ppu().Vdc.Sprite0Hit, resultType);
		case EvalValues::SpriteOverflow: return ReturnBool(ppu().Vdc.SpriteOverflow, resultType);
		case EvalValues::VerticalBlank: return ReturnBool(ppu().Vdc.VerticalBlank, resultType);

		case EvalValues::RegPS_Carry: return ReturnBool(s.PS & PceCpuFlags::Carry, resultType);
		case EvalValues::RegPS_Zero: return ReturnBool(s.PS & PceCpuFlags::Zero, resultType);
		case EvalValues::RegPS_Interrupt: return ReturnBool(s.PS & PceCpuFlags::Interrupt, resultType);
		case EvalValues::RegPS_Decimal: return ReturnBool(s.PS & PceCpuFlags::Decimal, resultType);
		case EvalValues::RegPS_Memory: return ReturnBool(s.PS & PceCpuFlags::Memory, resultType);
		case EvalValues::RegPS_Overflow: return ReturnBool(s.PS & PceCpuFlags::Overflow, resultType);
		case EvalValues::RegPS_Negative: return ReturnBool(s.PS & PceCpuFlags::Negative, resultType);

		default: return 0;
	}
}
