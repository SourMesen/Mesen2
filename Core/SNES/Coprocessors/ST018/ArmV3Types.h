#pragma once
#include "pch.h"
#include "Shared/MemoryType.h"
#include "Shared/BaseState.h"
#include "Utilities/Serializer.h"

enum class ArmV3CpuMode : uint8_t
{
	User = 0b10000,
	Fiq = 0b10001,
	Irq = 0b10010,
	Supervisor = 0b10011,
	Abort = 0b10111,
	Undefined = 0b11011,
	System = 0b11111,
};

enum class ArmV3CpuVector : uint32_t
{
	Undefined = 0x04,
	SoftwareIrq = 0x08,
	AbortPrefetch = 0x0C,
	AbortData = 0x10,
	Irq = 0x18,
	Fiq = 0x1C,
};

typedef uint8_t ArmV3AccessModeVal;

namespace ArmV3AccessMode
{
	enum Mode
	{
		Sequential = (1 << 0),
		Word = (1 << 1),
		Byte = (1 << 2),
		NoRotate = (1 << 3),
		Prefetch = (1 << 4)
	};
}

struct ArmV3CpuFlags
{
	ArmV3CpuMode Mode;
	bool FiqDisable;
	bool IrqDisable;

	bool Overflow;
	bool Carry;
	bool Zero;
	bool Negative;

	uint32_t ToInt32()
	{
		return (
			(Negative << 31) |
			(Zero << 30) |
			(Carry << 29) |
			(Overflow << 28) |

			(IrqDisable << 7) |
			(FiqDisable << 6) |
			(uint8_t)Mode
		);
	}
};

struct ArmV3InstructionData
{
	uint32_t Address;
	uint32_t OpCode;
};

struct ArmV3CpuPipeline
{
	ArmV3InstructionData Fetch;
	ArmV3InstructionData Decode;
	ArmV3InstructionData Execute;
	bool ReloadRequested;
	ArmV3AccessModeVal Mode;
};

struct ArmV3CpuState : BaseState
{
	ArmV3CpuPipeline Pipeline;
	uint32_t R[16];
	ArmV3CpuFlags CPSR;

	uint32_t UserRegs[7];
	uint32_t FiqRegs[7];
	uint32_t IrqRegs[2];

	uint32_t SupervisorRegs[2];
	uint32_t AbortRegs[2];
	uint32_t UndefinedRegs[2];

	ArmV3CpuFlags FiqSpsr;
	ArmV3CpuFlags IrqSpsr;
	ArmV3CpuFlags SupervisorSpsr;
	ArmV3CpuFlags AbortSpsr;
	ArmV3CpuFlags UndefinedSpsr;

	uint64_t CycleCount;
};
