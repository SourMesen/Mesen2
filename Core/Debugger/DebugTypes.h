#pragma once
#include "pch.h"
#include "Shared/MemoryOperationType.h"
#include "Shared/CpuType.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/AddressInfo.h"

enum class MemoryType;
enum class CpuType : uint8_t;

struct MemoryOperationInfo
{
	uint32_t Address;
	int32_t Value;
	MemoryOperationType Type;
	MemoryType MemType;

	MemoryOperationInfo()
	{
		Address = 0;
		Value = 0;
		Type = (MemoryOperationType)0;
		MemType = (MemoryType)0;
	}

	MemoryOperationInfo(uint32_t addr, int32_t val, MemoryOperationType opType, MemoryType memType)
	{
		Address = addr;
		Value = val;
		Type = opType;
		MemType = memType;
	}
};

enum class BreakpointTypeFlags
{
	None = 0,
	Read = 1,
	Write = 2,
	Execute = 4,
	Forbid = 8,
};

enum class BreakpointType
{
	Execute = 0,
	Read = 1,
	Write = 2,
	Forbid = 3,
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
	};
}

enum class CdlStripOption
{
	StripNone = 0,
	StripUnused,
	StripUsed,
};

struct CdlStatistics
{
	uint32_t CodeBytes;
	uint32_t DataBytes;
	uint32_t TotalBytes;
	
	uint32_t JumpTargetCount;
	uint32_t FunctionCount;

	//CHR ROM (NES-specific)
	uint32_t DrawnChrBytes;
	uint32_t TotalChrBytes;
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
		Address.Type = {};
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
		UnmappedMemory = 0x1000,
		Empty = 0x2000
	};
}

struct CodeLineData
{
	int32_t Address;
	AddressInfo AbsoluteAddress;
	uint8_t OpSize;
	uint16_t Flags;

	EffectiveAddressInfo EffectiveAddress;
	uint32_t Value;
	CpuType LineCpuType;

	uint8_t ByteCode[8];
	char Text[1000];
	char Comment[1000];
};

enum class TilemapDisplayMode
{
	Default,
	Grayscale,
	AttributeView
};

struct AddressCounters;

enum class TilemapHighlightMode
{
	None,
	Changes,
	Writes
};

struct GetTilemapOptions
{
	uint8_t Layer;
	uint8_t* CompareVram;
	AddressCounters* AccessCounters;

	uint64_t MasterClock;
	TilemapHighlightMode TileHighlightMode;
	TilemapHighlightMode AttributeHighlightMode;

	TilemapDisplayMode DisplayMode;
};

enum class TileFormat
{
	Bpp2,
	Bpp4,
	Bpp8,
	DirectColor,
	Mode7,
	Mode7DirectColor,
	Mode7ExtBg,
	NesBpp2,
	PceSpriteBpp4,
	PceSpriteBpp2Sp01,
	PceSpriteBpp2Sp23,
	PceBackgroundBpp2Cg0,
	PceBackgroundBpp2Cg1,
	SmsBpp4,
	SmsSgBpp1,
	GbaBpp4,
	GbaBpp8,
	WsBpp4Packed
};

enum class TileLayout
{
	Normal,
	SingleLine8x16,
	SingleLine16x16
};

enum class TileBackground
{
	Default,
	Transparent,
	PaletteColor,
	Black,
	White,
	Magenta,
};

enum class TileFilter
{
	None,
	HideUnused,
	HideUsed
};

struct GetTileViewOptions
{
	MemoryType MemType;
	TileFormat Format;
	TileLayout Layout;
	TileFilter Filter;
	TileBackground Background;
	int32_t Width;
	int32_t Height;
	int32_t StartAddress;
	int32_t Palette;
	bool UseGrayscalePalette;
};

enum class SpriteBackground
{
	Gray,
	Background,
	Transparent,
	Black,
	White,
	Magenta,
};

struct GetSpritePreviewOptions
{
	SpriteBackground Background;
};

struct GetPaletteInfoOptions
{
	TileFormat Format;
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
	AddressInfo AbsSource;
	uint32_t Target;
	AddressInfo AbsTarget;
	uint32_t Return;
	uint32_t ReturnStackPointer;
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
	DmaRead
};

enum class BreakSource
{
	Unspecified = -1,
	Breakpoint = 0,
	Pause,
	CpuStep,
	PpuStep,

	Irq,
	Nmi,

	//Used by DebugBreakHelper, prevents debugger getting focus
	InternalOperation,

	//Everything after InternalOperation is treated as an "Exception"
	//Forbid breakpoints can block these, but not the other types above
	BreakOnBrk,
	BreakOnCop,
	BreakOnWdm,
	BreakOnStp,
	BreakOnUninitMemoryRead,
	
	GbInvalidOamAccess,
	GbInvalidVramAccess,
	GbDisableLcdOutsideVblank,
	GbInvalidOpCode,
	GbNopLoad,
	GbOamCorruption,

	NesBreakOnDecayedOamRead,
	NesBreakOnPpu2000ScrollGlitch,
	NesBreakOnPpu2006ScrollGlitch,
	BreakOnUnofficialOpCode,
	NesBusConflict,
	NesBreakOnCpuCrash,
	NesBreakOnExtOutputMode,

	PceBreakOnInvalidVramAddress,
	
	SmsNopLoad,

	GbaInvalidOpCode,
	GbaNopLoad,
	GbaUnalignedMemoryAccess,

	BreakOnUndefinedOpCode
};

struct BreakEvent
{
	BreakSource Source;
	CpuType SourceCpu;
	MemoryOperationInfo Operation;
	int32_t BreakpointId;
};

enum class StepType
{
	Step,
	StepOut,
	StepOver,
	CpuCycleStep,
	PpuStep,
	PpuScanline,
	PpuFrame,
	SpecificScanline,
	RunToNmi,
	RunToIrq,
	StepBack
};

enum class BreakType
{
	None = 0,
	User = 1,
	Exception = 2,
	Both = 3
};

struct StepRequest
{
	int64_t BreakAddress = -1;
	int64_t BreakStackPointer = -1;
	int32_t StepCount = -1;
	int32_t PpuStepCount = -1;
	int32_t CpuCycleStepCount = -1;
	int32_t BreakScanline = INT32_MIN;
	StepType Type = StepType::Step;
	
	bool HasRequest = false;

	BreakType BreakNeeded = BreakType::None;
	BreakSource Source = BreakSource::Unspecified;
	BreakSource ExSource = BreakSource::Unspecified;

	StepRequest()
	{
	}

	StepRequest(StepType type)
	{
		Type = type;
	}

	StepRequest(const StepRequest& obj)
	{
		Type = obj.Type;
		StepCount = obj.StepCount;
		PpuStepCount = obj.PpuStepCount;
		CpuCycleStepCount = obj.CpuCycleStepCount;
		BreakAddress = obj.BreakAddress;
		BreakStackPointer = obj.BreakStackPointer;
		BreakScanline = obj.BreakScanline;
		HasRequest = (StepCount != -1 || PpuStepCount != -1 || BreakAddress != -1 || BreakScanline != INT32_MIN || CpuCycleStepCount != -1);
	}

	void ClearException()
	{
		ExSource = BreakSource::Unspecified;
		ClearBreakType(BreakType::Exception);
	}

	__forceinline void SetBreakSource(BreakSource source, bool breakNeeded)
	{
		if(source > BreakSource::InternalOperation) {
			if(ExSource == BreakSource::Unspecified) {
				ExSource = source;
			}

			if(breakNeeded) {
				SetBreakType(BreakType::Exception);
			}
		} else {
			if(Source == BreakSource::Unspecified) {
				Source = source;
			}

			if(breakNeeded) {
				SetBreakType(BreakType::User);
			}
		}
	}

	BreakSource GetBreakSource()
	{
		if(ExSource != BreakSource::Unspecified) {
			return ExSource;
		}

		if(Source == BreakSource::Unspecified) {
			if(BreakScanline != INT32_MIN || PpuStepCount >= 0) {
				return BreakSource::PpuStep;
			}
		}

		return Source;
	}

	__forceinline void SetBreakType(BreakType type)
	{
		BreakNeeded = (BreakType)((int)BreakNeeded | (int)type);
	}

	__forceinline void ClearBreakType(BreakType type)
	{
		BreakNeeded = (BreakType)((int)BreakNeeded & ~(int)type);
	}

	__forceinline void Break(BreakSource src)
	{
		SetBreakSource(src, true);
	}

	__forceinline void ProcessCpuExec()
	{
		if(StepCount > 0) {
			StepCount--;
			if(StepCount == 0) {
				SetBreakSource(BreakSource::CpuStep, true);
			}
		}
	}

	__forceinline bool ProcessCpuCycle()
	{
		if(CpuCycleStepCount > 0) {
			CpuCycleStepCount--;
			if(CpuCycleStepCount == 0) {
				SetBreakSource(BreakSource::CpuStep, true);
				return true;
			}
		}
		return false;
	}

	__forceinline void ProcessNmiIrq(bool forNmi)
	{
		if(forNmi) {
			if(Type == StepType::RunToNmi) {
				SetBreakSource(BreakSource::Nmi, true);
			}
		} else {
			if(Type == StepType::RunToIrq) {
				SetBreakSource(BreakSource::Irq, true);
			}
		}
	}

	bool HasScanlineBreakRequest()
	{
		return BreakScanline != INT32_MIN;
	}
};

struct CpuInstructionProgress
{
	uint64_t StartCycle = 0;
	uint64_t CurrentCycle = 0;
	uint32_t LastOpCode = 0;
	MemoryOperationInfo LastMemOperation = {};
};

struct DebugControllerState
{
	bool A;
	bool B;
	bool X;
	bool Y;
	bool L;
	bool R;
	bool U;
	bool D;
	bool Up;
	bool Down;
	bool Left;
	bool Right;
	bool Select;
	bool Start;

	bool HasPressedButton()
	{
		return A || B || X || Y || L || R || U || D || Up || Down || Left || Right || Select || Start;
	}
};
