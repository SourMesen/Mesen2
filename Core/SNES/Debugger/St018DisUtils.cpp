#include "pch.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Debugger/St018DisUtils.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "SNES/Coprocessors/ST018/ArmV3Cpu.h"
#include "SNES/Coprocessors/ST018/St018.h"
#include "SNES/Debugger/DummyArmV3Cpu.h"
#include "Shared/ArmEnums.h"
#include "Debugger/DisassemblyInfo.h"

EffectiveAddressInfo St018DisUtils::GetEffectiveAddress(DisassemblyInfo& info, SnesConsole* console, ArmV3CpuState& state)
{
	St018* st018 = console->GetCartridge()->GetSt018();
	uint32_t opCode = st018->DebugCpuRead(ArmV3AccessMode::Word, state.Pipeline.Execute.Address);
	state.Pipeline.Execute.OpCode = opCode;

	ArmOpCategory category = ArmV3Cpu::GetArmOpCategory(opCode);
	switch(category) {
		case ArmOpCategory::InvalidOp:
		case ArmOpCategory::BlockDataTransfer:
			return {};
	}

	DummyArmV3Cpu dummyCpu;
	dummyCpu.Init(console->GetEmulator(), st018);
	dummyCpu.SetDummyState(state);
	dummyCpu.Exec();

	uint32_t count = dummyCpu.GetOperationCount();
	for(int i = count - 1; i >= 0; i--) {
		MemoryOperationInfo opInfo = dummyCpu.GetOperationInfo(i);
		if(opInfo.Type != MemoryOperationType::ExecOperand) {
			EffectiveAddressInfo result;
			result.Address = opInfo.Address;
			result.Type = opInfo.MemType;
			result.ValueSize = 1;

			switch(dummyCpu.GetOperationMode(i) & (ArmV3AccessMode::Byte | ArmV3AccessMode::Word)) {
				case ArmV3AccessMode::Byte: result.ValueSize = 1; break;
				case ArmV3AccessMode::Word: result.ValueSize = 4; break;
			}

			result.ShowAddress = true;
			return result;
		}
	}

	return {};
}
