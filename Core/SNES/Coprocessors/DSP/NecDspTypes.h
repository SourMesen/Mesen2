#pragma once
#include "pch.h"
#include "Shared/BaseState.h"

struct NecDspAccFlags
{
	bool Carry;
	bool Zero;
	bool Overflow0;
	bool Overflow1;
	bool Sign0;
	bool Sign1;
};

namespace NecDspStatusFlags
{
	enum NecDspStatusFlags
	{
		RequestForMaster = 0x8000,
		UserFlag1 = 0x4000,
		UserFlag0 = 0x2000,
		DataRegStatus = 0x1000,
		Dma = 0x0800,
		DataRegControl = 0x0400,
		SerialOutControl = 0x0200,
		SerialInControl = 0x0100,
		EnableInterrupt = 0x0080,
	};
}

struct NecDspState : BaseState
{
	uint64_t CycleCount;

	/* Accumulator A */
	uint16_t A;
	NecDspAccFlags FlagsA;

	/* Accumulator B */
	uint16_t B;
	NecDspAccFlags FlagsB;

	/* Temporary Register */
	uint16_t TR;

	/* Temporary Register B */
	uint16_t TRB;

	/* Program counter */
	uint16_t PC;

	/* ROM pointer */
	uint16_t RP;

	/* Data pointer */
	uint16_t DP;

	/* Data Register */
	uint16_t DR;

	/* Status Register */
	uint16_t SR;

	/* Multiplication registers */
	uint16_t K;
	uint16_t L;

	/* Multiplication output registers */
	uint16_t M;
	uint16_t N;

	/* Serial output - not emulated */
	uint16_t SerialOut;

	/* Serial input- not emulated */
	uint16_t SerialIn;

	/* Stack pointer */
	uint8_t SP;
};
