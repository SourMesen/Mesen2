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
		Memory = 0x20,
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
	Ind, IndX, IndY,
	AbsX, AbsY,
	ZInd, ZeroRel,
	Block,
	ImZero, ImZeroX, ImAbs, ImAbsX
};

struct PcePpuState : public BaseState
{
	uint32_t FrameCount;
	
	uint16_t HClock;
	uint16_t Scanline;
	uint16_t DisplayCounter;
	uint16_t VceScanlineCount;
	uint16_t DisplayStart;
	uint16_t VerticalBlankScanline;
	uint16_t RcrCounter;

	uint8_t CurrentReg;

	//R00 - MAWR
	uint16_t MemAddrWrite;

	//R01 - MARR
	uint16_t MemAddrRead;
	uint16_t ReadBuffer;

	//R02 - VWR
	uint16_t VramData;

	//R05 - CR - Control
	bool EnableCollisionIrq;
	bool EnableOverflowIrq;
	bool EnableScanlineIrq;
	bool EnableVerticalBlankIrq;
	uint8_t ExternalSync;
	bool SpritesEnabled;
	bool BackgroundEnabled;
	uint8_t VramAddrIncrement;

	//R06 - RCR
	uint16_t RasterCompareRegister;

	//R07 - BXR
	uint16_t BgScrollX;
	uint16_t BgScrollXLatch;
	
	//R08 - BYR
	uint16_t BgScrollY;
	uint16_t BgScrollYLatch;
	bool BgScrollYUpdatePending;
	
	//R09 - MWR - Memory Width
	uint8_t ColumnCount;
	uint8_t RowCount;
	uint8_t SpriteAccessMode;
	uint8_t VramAccessMode;
	bool CgMode;

	//R0A - HSR
	uint8_t HorizDisplayStart;
	uint8_t HorizSyncWidth; //no effect

	//R0B - HDR
	uint8_t HorizDisplayWidth;
	uint8_t HorizDisplayEnd; //no effect

	//R0C - VPR
	uint8_t VertDisplayStart;
	uint8_t VertSyncWidth;

	//R0D - VDW
	uint16_t VertDisplayWidth;
	
	//R0E - VCR
	uint8_t VertEndPosVcr;

	//R0F - DCR
	bool VramSatbIrqEnabled;
	bool VramVramIrqEnabled;
	bool DecrementSrc;
	bool DecrementDst;
	bool RepeatSatbTransfer;

	//R10 - SOUR
	uint16_t BlockSrc;
	
	//R11 - DESR
	uint16_t BlockDst;
	
	//R12 - LENR
	uint16_t BlockLen;

	//R13 - DVSSR
	uint16_t SatbBlockSrc;
	bool SatbTransferPending;
	bool SatbTransferRunning;
	
	uint16_t SatbTransferNextWordCounter;
	uint8_t SatbTransferOffset;

	//Status flags
	bool VerticalBlank;
	bool VramTransferDone;
	bool SatbTransferDone;
	bool ScanlineDetected;
	bool SpriteOverflow;
	bool Sprite0Hit;
	
	bool BurstModeEnabled;
	bool NextSpritesEnabled;
	bool NextBackgroundEnabled;
	
	//VCE
	uint8_t VceClockDivider;
	uint16_t PalAddr;
};

struct PceMemoryManagerState
{
	uint64_t CycleCount;
	uint8_t Mpr[8];
	uint8_t ActiveIrqs;
	uint8_t DisabledIrqs;
	uint8_t CpuClockSpeed;
	uint8_t MprReadBuffer;
	uint8_t IoBuffer;
};

struct PceControlManagerState
{

};

struct PcePsgState
{
	uint8_t ChannelSelect;
	uint8_t LeftVolume;
	uint8_t RightVolume;
	uint8_t LfoFrequency;
	uint8_t LfoControl;
};

struct PcePsgChannelState
{
	uint16_t Frequency;
	uint8_t Amplitude;
	bool Enabled;
	uint8_t LeftVolume;
	uint8_t RightVolume;
	uint8_t WaveData[0x20];

	bool DdaEnabled;
	uint8_t DdaOutputValue;

	uint8_t WriteAddr;
	uint8_t ReadAddr;
	uint16_t Timer;
	int16_t CurrentOutput;

	//Channel 5 & 6 only
	bool NoiseEnabled;
	uint8_t NoiseFrequency;
};

struct PceState : public BaseState
{
	PceCpuState Cpu;
	PcePpuState Ppu;
	PceMemoryManagerState MemoryManager;
	PcePsgState Psg;
	PcePsgChannelState PsgChannels[6];
};