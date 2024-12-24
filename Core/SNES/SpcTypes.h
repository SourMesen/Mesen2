#pragma once
#include "pch.h"
#include "SNES/SpcTimer.h"
#include "SNES/SnesCpuTypes.h"
#include "Shared/BaseState.h"

struct SpcState : BaseState
{
	uint64_t Cycle = 0;
	uint16_t PC = 0;
	uint8_t A = 0;
	uint8_t X = 0;
	uint8_t Y = 0;
	uint8_t SP = 0;
	uint8_t PS = 0;

	bool WriteEnabled = false;
	bool RomEnabled = false;
	uint8_t InternalSpeed = 0;
	uint8_t ExternalSpeed = 0;
	bool TimersEnabled = false;
	bool TimersDisabled = false;
	SnesCpuStopState StopState = {};

	uint8_t DspReg = 0;
	uint8_t OutputReg[4] = {};
	uint8_t RamReg[2] = {};
	uint8_t CpuRegs[4] = {};
	uint8_t NewCpuRegs[4] = {};

	SpcTimer<128> Timer0;
	SpcTimer<128> Timer1;
	SpcTimer<16> Timer2;
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