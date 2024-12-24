#pragma once
#include "pch.h"
#include "Shared/BaseState.h"

enum class SnesCpuStopState : uint8_t
{
	Running = 0,
	Stopped = 1,
	WaitingForIrq = 2
};

struct SnesCpuState : BaseState
{
	uint64_t CycleCount;

	uint16_t A;
	uint16_t X;
	uint16_t Y;
	
	/* 16-bit stack pointer */
	uint16_t SP;

	/* 16-bit  Direct  Register  provides  an address  offset  for  all  instructions  using  direct  addressing */
	uint16_t D;

	/* 16-bit Program Counter */
	uint16_t PC;

	/* 8-bit Program Bank Register holds the bank address for all instruction fetches. */
	uint8_t K;

	/* 8-bit Data Bank Register holds the bank address for memory transfers. The 24-bit address
	is composed of the 16-bit instruction effective address and the 8-bit Data Bank address */
	uint8_t DBR;

	/* 8-bit status flags */
	uint8_t PS;

	/* 6502 emulation mode */
	bool EmulationMode;

	/* Misc internal state */
	uint8_t NmiFlagCounter;

	bool IrqLock;
	bool NeedNmi;

	uint8_t IrqSource;
	uint8_t PrevIrqSource;
	SnesCpuStopState StopState;
};

namespace ProcFlags
{
	enum ProcFlags : uint8_t
	{
		Carry = 0x01,
		Zero = 0x02,
		IrqDisable = 0x04,
		Decimal = 0x08,

		/* Use 8-bit operations on indexes */
		IndexMode8 = 0x10,

		/* Use 8-bit operations on memory accesses and accumulator */
		MemoryMode8 = 0x20,

		Overflow = 0x40,
		Negative = 0x80
	};
}

enum class SnesIrqSource
{
	None = 0,
	Ppu = 1,
	Coprocessor = 2
};

