#pragma once
#include "stdafx.h"

struct CpuState
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

enum class AddrMode : uint8_t
{
	Imm8,
	Imm16,
	ImmX,
	ImmM,
	Abs,
	AbsIdxXInd, //JMP/JSR only
	AbsIdxX,
	AbsIdxY,
	AbsInd, //JMP only
	AbsIndLng, //JML only
	AbsLngIdxX,
	AbsLng,
	AbsJmp, //JSR/JMP only
	AbsLngJmp, //JSL/JMP only
	Acc,
	BlkMov,
	DirIdxIndX,
	DirIdxX,
	DirIdxY,
	DirIndIdxY,
	DirIndLngIdxY,
	DirIndLng,
	DirInd,
	Dir,
	Imp,
	RelLng,
	Rel,
	Stk,
	StkRel,
	StkRelIndIdxY
};

enum class MemoryOperationType
{
	Read = 0,
	Write = 1,
	ExecOpCode = 2,
	ExecOperand = 3,
	DmaRead = 4,
	DmaWrite = 5
};

enum class IrqSource
{
	None = 0,
	Ppu = 1,
};