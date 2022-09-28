#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "NES/NesTypes.h"
#include "NES/Debugger/NesDebugger.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetNesTokens()
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
		{ "cycle", EvalValues::PpuCycle },
		{ "scanline", EvalValues::PpuScanline },
		{ "v", EvalValues::PpuVramAddress },
		{ "t", EvalValues::PpuTmpVramAddress },
		{ "sprite0hit", EvalValues::Sprite0Hit },
		{ "verticalblank", EvalValues::VerticalBlank },
		{ "spriteoverflow", EvalValues::SpriteOverflow },
		{ "pscarry", EvalValues::RegPS_Carry },
		{ "pszero", EvalValues::RegPS_Zero },
		{ "psinterrupt", EvalValues::RegPS_Interrupt },
		{ "psdecimal", EvalValues::RegPS_Decimal },
		{ "psoverflow", EvalValues::RegPS_Overflow },
		{ "psnegative", EvalValues::RegPS_Negative },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetNesTokenValue(int64_t token, EvalResultType& resultType)
{
	auto ppu = [this]() -> NesPpuState {
		NesPpuState ppu;
		((NesDebugger*)_cpuDebugger)->GetPpuState(ppu);
		return ppu;
	};

	NesCpuState& s = (NesCpuState&)((NesDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegX: return s.X;
		case EvalValues::RegY: return s.Y;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPS: return s.PS;
		case EvalValues::RegPC: return s.PC;
		case EvalValues::Nmi: return ReturnBool(s.NmiFlag, resultType);
		case EvalValues::Irq:  return ReturnBool(s.IrqFlag, resultType);

		case EvalValues::PpuFrameCount: return ppu().FrameCount;
		case EvalValues::PpuCycle: return ppu().Cycle;
		case EvalValues::PpuScanline: return ppu().Scanline;
		
		case EvalValues::PpuVramAddress: return ppu().VideoRamAddr;
		case EvalValues::PpuTmpVramAddress: return ppu().TmpVideoRamAddr;

		case EvalValues::Sprite0Hit: return ReturnBool(ppu().StatusFlags.Sprite0Hit, resultType);
		case EvalValues::SpriteOverflow: return ReturnBool(ppu().StatusFlags.SpriteOverflow, resultType);
		case EvalValues::VerticalBlank: return ReturnBool(ppu().StatusFlags.VerticalBlank, resultType);

		case EvalValues::RegPS_Carry: return ReturnBool(s.PS & PSFlags::Carry, resultType);
		case EvalValues::RegPS_Zero: return ReturnBool(s.PS & PSFlags::Zero, resultType);
		case EvalValues::RegPS_Interrupt: return ReturnBool(s.PS & PSFlags::Interrupt, resultType);
		case EvalValues::RegPS_Decimal: return ReturnBool(s.PS & PSFlags::Decimal, resultType);
		case EvalValues::RegPS_Overflow: return ReturnBool(s.PS & PSFlags::Overflow, resultType);
		case EvalValues::RegPS_Negative: return ReturnBool(s.PS & PSFlags::Negative, resultType);

		default: return 0;
	}
}