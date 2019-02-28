#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"

struct DebugState
{
	CpuState Cpu;
	PpuState Ppu;
};

enum class SnesMemoryType
{
	CpuMemory,
	PrgRom,
	WorkRam,
	SaveRam,
	VideoRam,
	SpriteRam,
	CGRam,
};

struct AddressInfo
{
	int32_t Address;
	SnesMemoryType Type;
};

struct MemoryOperationInfo
{
	uint32_t Address;
	int32_t Value;
	MemoryOperationType OperationType;
};

namespace CdlFlags
{
	enum CdlFlags : uint8_t
	{
		None = 0x00,
		Code = 0x01,
		Data = 0x02,
		JumpTarget = 0x04,
		SubEntryPoint = 0x08,

		IndexMode8 = 0x10,
		MemoryMode8 = 0x20,
	};
}

enum class CdlStripFlag
{
	StripNone = 0,
	StripUnused,
	StripUsed,
};

struct CdlRatios
{
	float CodeRatio;
	float DataRatio;
	float PrgRatio;
};

struct DisassemblyResult
{
	AddressInfo Address;
	int32_t CpuAddress;
	uint8_t Flags;

	DisassemblyResult(int32_t cpuAddress, uint8_t flags)
	{
		Flags = flags;
		CpuAddress = cpuAddress;
		Address.Address = -1;
	}

	DisassemblyResult(AddressInfo address, int32_t cpuAddress, uint8_t flags = 0)
	{
		Address = address;
		CpuAddress = cpuAddress;
		Flags = flags;
	}
};

namespace LineFlags
{
	enum LineFlags : uint8_t
	{
		None = 0,
		PrgRom = 1,
		WorkRam = 2,
		SaveRam = 4,
		VerifiedData = 8,
		VerifiedCode = 16,
		BlockStart = 32,
		BlockEnd = 64,
		SubStart = 128
	};
}

struct CodeLineData
{
	int32_t Address;
	int32_t AbsoluteAddress;
	uint8_t OpSize;
	uint8_t Flags;

	int32_t EffectiveAddress;
	uint16_t Value;
	uint8_t ValueSize;

	uint8_t ByteCode[4];
	char Text[1000];
	char Comment[1000];
};