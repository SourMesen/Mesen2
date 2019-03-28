#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"

struct DebugState
{
	uint64_t MasterClock;
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
	SpcRam,
	Register,
};

struct AddressInfo
{
	int32_t Address;
	SnesMemoryType Type;

	AddressInfo() { }

	AddressInfo(int32_t address, SnesMemoryType type)
	{
		Address = address;
		Type = type;
	}
};

struct MemoryOperationInfo
{
	uint32_t Address;
	int32_t Value;
	MemoryOperationType Type;
	
	MemoryOperationInfo() { }

	MemoryOperationInfo(uint32_t address, int32_t value, MemoryOperationType type)
	{
		Address = address;
		Value = value;
		Type = type;
	}
};

enum class BreakpointTypeFlags
{
	None = 0,
	Execute = 1,
	Read = 2,
	Write = 4,
};

enum class BreakpointType
{
	Execute = 0,
	Read = 1,
	Write = 2,
};

enum class BreakpointCategory
{
	Cpu = 0,
	VideoRam = 1,
	Oam = 2,
	CgRam = 3,
	Spc = 4
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

struct GetTilemapOptions
{
	uint8_t BgMode;
	uint8_t Layer;

	int8_t Bpp;
	int32_t TilemapAddr;
	int32_t ChrAddr;

	bool ShowTileGrid;
	bool ShowScrollOverlay;
};

enum TileFormat
{
	Bpp2,
	Bpp4,
	Bpp8,
	DirectColor,
	Mode7,
	Mode7DirectColor,
};

struct GetTileViewOptions
{
	TileFormat Format;
	int32_t Width;
	int32_t Palette;
	SnesMemoryType MemoryType;
	int32_t AddressOffset;

	bool ShowTileGrid;
};

enum class StackFrameFlags
{
	None = 0,
	Nmi = 1,
	Irq = 2
};

struct StackFrameInfo
{
	uint32_t Source;
	uint32_t Target;
	uint32_t Return;
	StackFrameFlags Flags;
};

enum class StepType
{
	CpuStep,
	CpuStepOut,
	CpuStepOver,
	PpuStep,
};
