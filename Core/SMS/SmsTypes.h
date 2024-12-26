#pragma once
#include "pch.h"
#include "Shared/BaseState.h"
#include "Shared/MemoryType.h"

enum class SmsModel
{
	Sms,
	GameGear,
	Sg,
	ColecoVision
};

struct SmsCpuState : public BaseState
{
	uint64_t CycleCount;
	uint16_t PC;
	uint16_t SP;

	uint8_t A;
	uint8_t Flags;

	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;

	uint8_t H;
	uint8_t L;

	uint8_t IXL;
	uint8_t IXH;

	uint8_t IYL;
	uint8_t IYH;

	uint8_t I;
	uint8_t R;

	uint8_t AltA;
	uint8_t AltFlags;
	uint8_t AltB;
	uint8_t AltC;
	uint8_t AltD;
	uint8_t AltE;
	uint8_t AltH;
	uint8_t AltL;

	uint8_t ActiveIrqs;
	bool NmiLevel;
	bool NmiPending;
	bool Halted;

	bool IFF1;
	bool IFF2;

	uint8_t IM;

	//Internal flags needed to properly emulate the behavior of the undocumented F3/F5 flags
	uint8_t FlagsChanged;
	uint16_t WZ;
};

namespace SmsCpuFlags
{
	enum SmsCpuFlags
	{
		Carry = 0x01,
		AddSub = 0x02,
		Parity = 0x04,
		F3 = 0x08,
		HalfCarry = 0x10,
		F5 = 0x20,
		Zero = 0x40,
		Sign = 0x80,
	};
}

struct SmsVdpState : public BaseState
{
	uint32_t FrameCount;
	uint16_t Cycle;
	uint16_t Scanline;
	uint16_t VCounter;

	uint16_t AddressReg;
	uint8_t CodeReg;
	bool ControlPortMsbToggle;

	uint8_t ReadBuffer;
	uint8_t PaletteLatch;
	uint8_t HCounterLatch;

	bool VerticalBlankIrqPending;
	bool ScanlineIrqPending;
	bool SpriteOverflow;
	bool SpriteCollision;
	uint8_t SpriteOverflowIndex;

	uint16_t ColorTableAddress;
	uint16_t BgPatternTableAddress;

	uint16_t SpriteTableAddress;
	uint16_t SpritePatternSelector;

	uint16_t NametableHeight;
	uint8_t VisibleScanlineCount;

	uint8_t TextColorIndex;
	uint8_t BackgroundColorIndex;
	uint8_t HorizontalScroll;
	uint8_t HorizontalScrollLatch;
	uint8_t VerticalScroll;
	uint8_t VerticalScrollLatch;
	uint8_t ScanlineCounter;
	uint8_t ScanlineCounterLatch;

	//Control register 1
	bool SyncDisabled;
	bool M2_AllowHeightChange;
	bool UseMode4;
	bool ShiftSpritesLeft;
	bool EnableScanlineIrq;
	bool MaskFirstColumn;
	bool HorizontalScrollLock;
	bool VerticalScrollLock;

	//Control register 2
	bool Sg16KVramMode;
	bool RenderingEnabled;
	bool EnableVerticalBlankIrq;
	bool M1_Use224LineMode;
	bool M3_Use240LineMode;
	bool UseLargeSprites;
	bool EnableDoubleSpriteSize;
	
	uint16_t NametableAddress;
	uint16_t EffectiveNametableAddress;
	uint16_t NametableAddressMask;
};

struct SmsToneChannelState
{
	uint16_t ReloadValue;
	uint16_t Timer;
	uint8_t Output;
	uint8_t Volume;
};

struct SmsNoiseChannelState
{
	uint16_t Timer;
	uint16_t Lfsr;
	uint8_t LfsrInputBit;
	uint8_t Control;
	uint8_t Output;
	uint8_t Volume;
};

struct SmsPsgState
{
	SmsToneChannelState Tone[3] = {};
	SmsNoiseChannelState Noise = {};
	uint8_t SelectedReg = 0;
	uint8_t GameGearPanningReg = 0xFF;
};

enum class SmsRegisterAccess
{
	None = 0,
	Read = 1,
	Write = 2,
	ReadWrite = 3
};

struct SmsMemoryManagerState
{
	bool IsReadRegister[0x100];
	bool IsWriteRegister[0x100];

	uint8_t OpenBus;
	uint8_t GgExtData;
	uint8_t GgExtConfig;
	uint8_t GgSendData;
	uint8_t GgSerialConfig;

	bool ExpEnabled;
	bool CartridgeEnabled;
	bool CardEnabled;
	bool WorkRamEnabled;
	bool BiosEnabled;
	bool IoEnabled;
};

struct SmsControlManagerState
{
	uint8_t ControlPort;
};

struct SmsState
{
	SmsCpuState Cpu;
	SmsVdpState Vdp;
	SmsPsgState Psg;
	SmsMemoryManagerState MemoryManager;
	SmsControlManagerState ControlManager;
};

enum class SmsIrqSource
{
	None = 0,
	Vdp = 1,
};

enum class SmsVdpWriteType : uint8_t
{
	None = 0,
	Vram = 1,
	Palette = 2
};

class SmsCpuParityTable
{
private:
	uint8_t _parityTable[0x100];

public:
	SmsCpuParityTable()
	{
		for(int i = 0; i < 256; i++) {
			int count = 0;
			for(int j = 0; j < 8; j++) {
				count += (i & (1 << j)) ? 1 : 0;
			}
			_parityTable[i] = count & 0x01 ? 0 : 1;
		}
	}

	__forceinline bool CheckParity(uint8_t val)
	{
		return _parityTable[val];
	}
};
