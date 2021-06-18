#pragma once
#include "stdafx.h"
#include "Core/MemoryOperationType.h"

enum class SnesMemoryType;

struct AddressInfo
{
	int32_t Address;
	SnesMemoryType Type;
};

struct MemoryOperationInfo
{
	uint32_t Address;
	int32_t Value;
	MemoryOperationType Type;
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

		Gsu = 0x40,
		Cx4 = 0x80
	};
}

enum class CdlStripOption
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
	uint16_t Flags;
	int16_t CommentLine;

	DisassemblyResult(int32_t cpuAddress, uint16_t flags, int16_t commentLine = -1)
	{
		Flags = flags;
		CpuAddress = cpuAddress;
		Address.Address = -1;
		CommentLine = commentLine;
	}

	DisassemblyResult(AddressInfo address, int32_t cpuAddress, uint16_t flags = 0, int16_t commentLine = -1)
	{
		Address = address;
		CpuAddress = cpuAddress;
		Flags = flags;
		CommentLine = commentLine;
	}

	void SetByteCount(uint8_t byteCount)
	{
		CommentLine = byteCount;
	}

	uint8_t GetByteCount()
	{
		return (uint8_t)CommentLine;
	}
};

namespace LineFlags
{
	enum LineFlags : uint16_t
	{
		None = 0,
		PrgRom = 0x01,
		WorkRam = 0x02,
		SaveRam = 0x04,
		VerifiedData = 0x08,
		VerifiedCode = 0x10,
		BlockStart = 0x20,
		BlockEnd = 0x40,
		SubStart = 0x80,
		Label = 0x100,
		Comment = 0x200,
		ShowAsData = 0x400,
		UnexecutedCode = 0x800,
		UnmappedMemory = 0x1000
	};
}

struct CodeLineData
{
	int32_t Address;
	int32_t AbsoluteAddress;
	uint8_t OpSize;
	uint16_t Flags;

	int32_t EffectiveAddress;
	uint16_t Value;
	uint8_t ValueSize;

	uint8_t ByteCode[4];
	char Text[1000];
	char Comment[1000];
};

struct GetTilemapOptions
{
	uint8_t Layer;
};

enum class TileFormat
{
	Bpp2,
	Bpp4,
	Bpp8,
	DirectColor,
	Mode7,
	Mode7DirectColor,
};

enum class TileLayout
{
	Normal,
	SingleLine8x16,
	SingleLine16x16
};

enum class TileBackground
{
	Default = 0,
	PaletteColor = 1,
	Black = 2,
	White = 3,
	Magenta = 4
};

struct GetTileViewOptions
{
	TileFormat Format;
	TileLayout Layout;
	TileBackground Background;
	int32_t Width;
	int32_t Height;
	int32_t StartAddress;
	int32_t Palette;
};

struct GetSpritePreviewOptions
{
	int32_t SelectedSprite;
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
	AddressInfo AbsReturn;
	StackFrameFlags Flags;
};

enum class DebugEventType
{
	Register,
	Nmi,
	Irq,
	Breakpoint,
	BgColorChange,
	SpriteZeroHit,
	DmcDmaRead,
};

enum class BreakSource
{
	Unspecified = -1,
	Breakpoint = 0,
	CpuStep = 1,
	PpuStep = 2,
	BreakOnBrk = 3,
	BreakOnCop = 4,
	BreakOnWdm = 5,
	BreakOnStp = 6,
	BreakOnUninitMemoryRead = 7,
	
	GbInvalidOamAccess = 8,
	GbInvalidVramAccess = 9,
	GbDisableLcdOutsideVblank = 10,
	GbInvalidOpCode = 11,
	GbNopLoad = 12,
	GbOamCorruption = 13,

	BreakOnCpuCrash = 14,

	NesBreakOnDecayedOamRead = 100,
	NesBreakOnPpu2006ScrollGlitch = 101,
};

struct BreakEvent
{
	BreakSource Source;
	MemoryOperationInfo Operation;
	int32_t BreakpointId;
};

enum class StepType
{
	Step,
	StepOut,
	StepOver,
	PpuStep,
	SpecificScanline,
};

struct StepRequest
{
	int32_t StepCount = -1;
	int32_t PpuStepCount = -1;
	int32_t BreakAddress = -1;
	int32_t BreakScanline = INT32_MIN;
};

enum class CpuType : uint8_t
{
	Cpu,
	Spc,
	NecDsp,
	Sa1,
	Gsu,
	Cx4,
	Gameboy,
	Nes
};
