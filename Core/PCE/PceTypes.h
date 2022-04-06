#pragma once
#include "stdafx.h"
#include "Shared/BaseState.h"

enum class PceIrqSource
{
	Irq2 = 1,
	Irq1 = 2,
	TimerIrq = 4,
};

namespace PceCpuFlags
{
	enum PceCpuFlags : uint8_t
	{
		Carry = 0x01,
		Zero = 0x02,
		Interrupt = 0x04,
		Decimal = 0x08,
		Break = 0x10,
		Reserved = 0x20,
		Overflow = 0x40,
		Negative = 0x80
	};
}

struct PceCpuState : public BaseState
{
	uint64_t CycleCount = 0;
	uint16_t PC = 0;
	uint8_t SP = 0;
	uint8_t A = 0;
	uint8_t X = 0;
	uint8_t Y = 0;
	uint8_t PS = 0;

	/* Source High */
	uint8_t SH = 0;

	/* Dest High */
	uint8_t DH = 0;

	/* Length High */
	uint8_t LH = 0;
};

enum class PceAddrMode
{
	None, Acc, Imp, Imm, Rel,
	Zero, Abs, ZeroX, ZeroY,
	Ind, IndX, IndY, IndYW,
	AbsX, AbsXW, AbsY, AbsYW,
	ZInd, ZeroRel,
	Block,
	ImZero, ImZeroX, ImAbs, ImAbsX
};

struct PcePpuState
{
	uint32_t FrameCount;

	uint8_t CurrentReg;

	uint16_t MemAddrWrite;  //R00 - MAWR
	uint16_t MemAddrRead;   //R01 - MARR
	uint16_t VramData;      //R02 - VWR

	//R05 - CR - Control
	bool EnableCollisionIrq;
	bool EnableOverflowIrq;
	bool EnableScanlineIrq;
	bool EnableVerticalBlankIrq;
	uint8_t ExternalSync;
	bool SpritesEnabled;
	bool BackgroundEnabled;
	uint8_t VramAddrIncrement;

	uint16_t ScanlineIrqValue;           //R06 - RCR

	uint16_t BgScrollX;     //R07 - BXR
	uint16_t BgScrollY;     //R08 - BYR

	//R09 - MWR - Memory Width
	uint8_t ColumnCount;
	uint8_t RowCount;
	uint8_t SpriteAccessMode;
	uint8_t VramAccessMode;
	bool CgMode;

	uint16_t HorizSync;     //R0A - HSR
	uint16_t HorizDisplay;  //R0B - HDR

	uint16_t VertSync;      //R0C - VPR
	uint16_t VertDisplay;   //R0D - VDW
	uint8_t VertEndPos;     //R0E - VCR

	//uint8_t BlockTransCtrl; //R0F - DCR
	bool VramSatbIrqEnabled;
	bool VramVramIrqEnabled;
	bool DecrementSrc;
	bool DecrementDst;
	bool RepeatSatbTransfer;

	uint16_t BlockSrc;      //R10 - SOUR
	uint16_t BlockDst;      //R11 - DESR
	uint16_t BlockLen;      //R12 - LENR

	uint16_t SatbBlockSrc;  //R13 - DVSSR
	bool SatbTransferPending;

	uint16_t PalAddr;
	uint16_t PalData;

	//Status flags
	bool VerticalBlank;
	bool VramTransferDone;
	bool SatbTransferDone;
	bool ScanlineDetected;
	bool SpriteOverflow;
	bool Sprite0Hit;
};

struct PceMemoryManagerState
{
	uint8_t ActiveIrqs;
	uint8_t DisabledIrqs;
	uint64_t CycleCount;
	uint8_t CpuClockSpeed;
};

struct PceControlManagerState
{

};