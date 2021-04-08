#pragma once
#include "stdafx.h"
#include "SpcTimer.h"
#include "CpuTypes.h"
#include "BaseState.h"

struct SpcState : BaseState
{
	uint64_t Cycle;
	uint16_t PC;
	uint8_t A;
	uint8_t X;
	uint8_t Y;
	uint8_t SP;
	uint8_t PS;

	bool WriteEnabled;
	bool RomEnabled;
	uint8_t InternalSpeed;
	uint8_t ExternalSpeed;
	bool TimersEnabled;
	bool TimersDisabled;
	CpuStopState StopState;

	uint8_t DspReg;
	uint8_t OutputReg[4];
	uint8_t RamReg[2];
	uint8_t CpuRegs[4];

	SpcTimer<128> Timer0;
	SpcTimer<128> Timer1;
	SpcTimer<16> Timer2;
};

struct DspState
{
	uint8_t Regs[128];
};

namespace SpcFlags {
	enum SpcFlags : uint8_t
	{
		Carry = 0x01,
		Zero = 0x02,
		IrqEnable = 0x04, //unused
		HalfCarry = 0x08,
		Break = 0x10, /* Set by the BRK instruction */
		DirectPage = 0x20, /* Selects page 0 or 1 for direct mode addressing */
		Overflow = 0x40,
		Negative = 0x80
	};
}

namespace SpcControlFlags {
	enum SpcControlFlags : uint8_t
	{
		Timer0 = 0x01,
		Timer1 = 0x02,
		Timer2 = 0x04,

		ClearPortsA = 0x10,
		ClearPortsB = 0x20,

		EnableRom = 0x80
	};
}

namespace SpcTestFlags {
	enum SpcTestFlags : uint8_t
	{
		TimersDisabled = 0x01,
		WriteEnabled = 0x02,
		Unknown = 0x04,
		TimersEnabled = 0x08,

		ExternalSpeed = 0x30,
		InternalSpeed = 0xC0,
	};
}

enum class SpcOpStep : uint8_t
{
	ReadOpCode = 0,
	Addressing = 1,
	AfterAddressing = 2,
	Operation = 3
};