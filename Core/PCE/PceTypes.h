#pragma once
#include "pch.h"
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
};

enum class PceAddrMode
{
	None, Acc, Imp, Imm, Rel,
	Zero, Abs, ZeroX, ZeroY,
	Ind, IndX, IndY,
	AbsX, AbsY,
	ZInd, ZeroRel,
	Block,
	ImZero, ImZeroX, ImAbs, ImAbsX,
	AbsXInd,
};

struct PceVdcHvLatches
{
	//R07 - BXR
	uint16_t BgScrollX;

	//R08 - BYR
	uint16_t BgScrollY;

	//R09 - MWR - Memory Width
	uint8_t ColumnCount;
	uint8_t RowCount;
	uint8_t SpriteAccessMode;
	uint8_t VramAccessMode;
	bool CgMode;

	//R0A - HSR
	uint8_t HorizDisplayStart;
	uint8_t HorizSyncWidth;

	//R0B - HDR
	uint8_t HorizDisplayWidth;
	uint8_t HorizDisplayEnd;

	//R0C - VPR
	uint8_t VertDisplayStart;
	uint8_t VertSyncWidth;

	//R0D - VDW
	uint16_t VertDisplayWidth;

	//R0E - VCR
	uint8_t VertEndPosVcr;
};

struct PceVdcState
{
	uint32_t FrameCount;
	
	uint16_t HClock;
	uint16_t Scanline;
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
	bool OutputVerticalSync;
	bool OutputHorizontalSync;
	bool SpritesEnabled;
	bool BackgroundEnabled;
	uint8_t VramAddrIncrement;

	//R06 - RCR
	uint16_t RasterCompareRegister;

	bool BgScrollYUpdatePending;

	PceVdcHvLatches HvLatch;
	PceVdcHvLatches HvReg;

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
};

struct PceVceState
{
	uint16_t ScanlineCount;
	uint16_t PalAddr;
	uint8_t ClockDivider;
	bool Grayscale;
};

struct PceMemoryManagerState
{
	uint64_t CycleCount;
	uint8_t Mpr[8];
	uint8_t ActiveIrqs;
	uint8_t DisabledIrqs;
	bool FastCpuSpeed;
	uint8_t MprReadBuffer;
	uint8_t IoBuffer;
};

struct PceControlManagerState
{

};

struct PceTimerState
{
	uint8_t ReloadValue;
	uint8_t Counter;
	uint16_t Scaler;
	bool Enabled;
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
	uint32_t Timer;
	int8_t CurrentOutput;

	//Channel 5 & 6 only
	uint32_t NoiseLfsr;
	uint32_t NoiseTimer;
	bool NoiseEnabled;
	int8_t NoiseOutput;
	uint8_t NoiseFrequency;
};

enum class PceVpcPriorityMode
{
	Default = 0,
	Vdc2SpritesAboveVdc1Bg = 1,
	Vdc1SpritesBelowVdc2Bg = 2,
};

enum class PceVpcPixelWindow
{
	NoWindow,
	Window1,
	Window2,
	Both
};

struct PceVpcPriorityConfig
{
	PceVpcPriorityMode PriorityMode;
	bool Vdc1Enabled;
	bool Vdc2Enabled;
};

struct PceVpcState
{
	PceVpcPriorityConfig WindowCfg[(int)PceVpcPixelWindow::Both + 1];
	uint8_t Priority1;
	uint8_t Priority2;
	uint16_t Window1;
	uint16_t Window2;
	bool StToVdc2Mode;
	bool HasIrqVdc1;
	bool HasIrqVdc2;
};

struct PceVideoState : BaseState
{
	PceVdcState Vdc;
	PceVceState Vce;
	PceVpcState Vpc;
	PceVdcState Vdc2;
};

enum class PceArcadePortOffsetTrigger
{
	None = 0,
	AddOnLowWrite = 1,
	AddOnHighWrite = 2,
	AddOnReg0AWrite = 3,
};

struct PceArcadeCardPortConfig
{
	uint32_t BaseAddress;
	uint16_t Offset;
	uint16_t IncValue;

	uint8_t Control;
	bool AutoIncrement;
	bool AddOffset;
	bool SignedIncrement; //unused?
	bool SignedOffset;
	bool AddIncrementToBase;
	PceArcadePortOffsetTrigger AddOffsetTrigger;
};

struct PceArcadeCardState
{
	PceArcadeCardPortConfig Port[4];
	uint32_t ValueReg;
	uint8_t ShiftReg;
	uint8_t RotateReg;
};

enum class PceCdRomIrqSource
{
	Adpcm = 0x04,
	Stop = 0x08,
	DataTransferDone = 0x20,
	DataTransferReady = 0x40
};

struct PceCdRomState
{
	uint16_t AudioSampleLatch = 0;
	uint8_t ActiveIrqs = 0;
	uint8_t EnabledIrqs = 0;
	bool ReadRightChannel = false;
	bool BramLocked = false;
	uint8_t ResetRegValue = 0;
};

struct PceAdpcmState
{
	bool Nibble;
	uint16_t ReadAddress;
	uint16_t WriteAddress;

	uint16_t AddressPort;

	uint8_t DmaControl;
	uint8_t Control;
	uint8_t PlaybackRate;

	uint32_t AdpcmLength;
	bool EndReached;
	bool HalfReached;

	bool Playing;

	uint8_t ReadBuffer;
	uint8_t ReadClockCounter;

	uint8_t WriteBuffer;
	uint8_t WriteClockCounter;
};

enum class CdPlayEndBehavior
{
	Stop,
	Loop,
	Irq
};

enum class CdAudioStatus : uint8_t
{
	Playing = 0,
	Inactive = 1,
	Paused = 2,
	Stopped = 3
};

struct PceCdAudioPlayerState
{
	CdAudioStatus Status;

	uint32_t StartSector;
	uint32_t EndSector;
	CdPlayEndBehavior EndBehavior;

	uint32_t CurrentSector;
	uint32_t CurrentSample;

	int16_t LeftSample;
	int16_t RightSample;
};

enum class ScsiPhase
{
	BusFree,
	Command,
	DataIn,
	DataOut, //unused
	MessageIn,
	MessageOut, //unused
	Status
};

struct PceScsiBusState
{
	bool Signals[9];
	ScsiPhase Phase;

	bool MessageDone;
	uint8_t DataPort;
	uint8_t ReadDataPort;

	uint32_t Sector;
	uint8_t SectorsToRead;
};

enum class PceAudioFaderTarget
{
	Adpcm,
	CdAudio,
};

struct PceAudioFaderState
{
	uint64_t StartClock;
	PceAudioFaderTarget Target;
	bool FastFade;
	bool Enabled;
	uint8_t RegValue;
};

struct PceState
{
	PceCpuState Cpu;
	PceVideoState Video;
	PceMemoryManagerState MemoryManager;
	PceTimerState Timer;
	PcePsgState Psg;
	PcePsgChannelState PsgChannels[6];

	PceCdRomState CdRom;
	PceCdAudioPlayerState CdPlayer;
	PceAdpcmState Adpcm;
	PceAudioFaderState AudioFader;
	PceScsiBusState ScsiDrive;
	PceArcadeCardState ArcadeCard;

	bool IsSuperGrafx;
	bool HasArcadeCard;
	bool HasCdRom;
};
