#pragma once
#include "pch.h"
#include "Shared/BaseState.h"
#include "Shared/SettingTypes.h"

class WsConstants
{
public:
	static constexpr uint32_t ScreenWidth = 224;
	static constexpr uint32_t ScreenHeight = 144;
	static constexpr uint32_t ClocksPerScanline = 256;
	static constexpr uint32_t ScanlineCount = 159;
	static constexpr uint32_t PixelCount = WsConstants::ScreenWidth * WsConstants::ScreenHeight;
	static constexpr uint32_t MaxPixelCount = WsConstants::ScreenWidth * (WsConstants::ScreenHeight + 13);
};

struct WsCpuFlags
{
	bool Carry; //0x01
	bool Parity; //0x04
	bool AuxCarry; //0x10
	bool Zero; //0x40
	bool Sign; //0x80
	bool Trap; //0x100
	bool Irq; //0x200
	bool Direction; //0x400
	bool Overflow; //0x800
	bool Mode; //0x8000

	uint16_t Get()
	{
		return (
			(uint8_t)Carry |
			((uint8_t)Parity << 2) |
			((uint8_t)AuxCarry << 4) |
			((uint8_t)Zero << 6) |
			((uint8_t)Sign << 7) |
			((uint8_t)Trap << 8) |
			((uint8_t)Irq << 9) |
			((uint8_t)Direction << 10) |
			((uint8_t)Overflow << 11) |
			((uint8_t)Mode << 15) |
			0x7002
		);
	}

	void Set(uint16_t f)
	{
		Carry = f & 0x01;
		Parity = f & 0x04;
		AuxCarry = f & 0x10;
		Zero = f & 0x40;
		Sign = f & 0x80;
		Trap = f & 0x100;
		Irq = f & 0x200;
		Direction = f & 0x400;
		Overflow = f & 0x800;
		Mode = f & 0x8000;
	}
};

struct WsCpuState : BaseState
{
	uint64_t CycleCount;

	uint16_t CS;
	uint16_t IP;

	uint16_t SS;
	uint16_t SP;
	uint16_t BP;

	uint16_t DS;
	uint16_t ES;

	uint16_t SI;
	uint16_t DI;

	uint16_t AX;
	uint16_t BX;
	uint16_t CX;
	uint16_t DX;

	WsCpuFlags Flags;
	bool Halted;
};

struct WsBgLayer
{
	uint16_t MapAddress;
	uint16_t MapAddressLatch;

	uint8_t ScrollX;
	uint8_t ScrollXLatch;

	uint8_t ScrollY;
	uint8_t ScrollYLatch;

	bool Enabled;
	bool EnabledLatch;

	void Latch()
	{
		EnabledLatch = Enabled;
		ScrollXLatch = ScrollX;
		ScrollYLatch = ScrollY;
		MapAddressLatch = MapAddress;
	}
};

struct WsWindow
{
	bool Enabled;
	bool EnabledLatch;

	uint8_t Left;
	uint8_t LeftLatch;
	uint8_t Right;
	uint8_t RightLatch;
	uint8_t Top;
	uint8_t TopLatch;
	uint8_t Bottom;
	uint8_t BottomLatch;

	bool IsInsideWindow(uint8_t x, uint8_t y) { return x >= LeftLatch && x <= RightLatch && y >= TopLatch && y <= BottomLatch; }

	void Latch()
	{
		EnabledLatch = Enabled;
		LeftLatch = Left;
		RightLatch = Right;
		TopLatch = Top;
		BottomLatch = Bottom;
	}
};

struct WsLcdIcons
{
	bool Sleep;
	bool Vertical;
	bool Horizontal;
	bool Aux1;
	bool Aux2;
	bool Aux3;

	uint8_t Value;
};

enum class WsVideoMode : uint8_t
{
	Monochrome,
	Color2bpp,
	Color4bpp,
	Color4bppPacked
};

struct WsPpuState : BaseState
{
	uint32_t FrameCount;
	uint16_t Cycle;
	uint16_t Scanline;

	WsBgLayer BgLayers[2];
	WsWindow BgWindow;
	WsWindow SpriteWindow;
	bool DrawOutsideBgWindow;
	bool DrawOutsideBgWindowLatch;

	uint8_t BwPalettes[0x20 * 2];
	uint8_t BwShades[8];

	uint16_t SpriteTableAddress;
	uint8_t FirstSpriteIndex;
	uint8_t SpriteCount;
	uint8_t SpriteCountLatch;
	bool SpritesEnabled;
	bool SpritesEnabledLatch;

	WsVideoMode Mode;
	WsVideoMode NextMode;

	uint8_t BgColor;
	uint8_t IrqScanline;
	
	bool LcdEnabled;
	bool HighContrast;
	bool SleepEnabled;

	WsLcdIcons Icons;

	uint8_t LastScanline;
	uint8_t BackPorchScanline;

	uint32_t ShowVolumeIconFrame;
	uint8_t LcdTftConfig[8];

	uint8_t Control; //$00
	uint8_t ScreenAddress; //$07
};

enum class WsSegment : uint8_t
{
	Default,
	ES,
	SS,
	CS,
	DS
};

enum class WsIrqSource : uint8_t
{
	UartSendReady = 0x01,
	KeyPressed = 0x02,
	Cart = 0x04,
	UartRecvReady = 0x08,
	Scanline = 0x10,
	VerticalBlankTimer = 0x20,
	VerticalBlank = 0x40,
	HorizontalBlankTimer = 0x80
};

struct WsMemoryManagerState
{
	uint8_t ActiveIrqs;
	uint8_t EnabledIrqs;
	uint8_t IrqVectorOffset;

	uint8_t SystemControl2;
	bool ColorEnabled;
	bool Enable4bpp;
	bool Enable4bppPacked;
	
	bool BootRomDisabled;
	bool CartWordBus;
	bool SlowRom;

	bool SlowSram;
	bool SlowPort;

	bool EnableLowBatteryNmi;
};

struct WsControlManagerState
{
	uint8_t InputSelect;
};

struct WsDmaControllerState
{
	uint32_t GdmaSrc;
	uint32_t SdmaSrc;
	uint32_t SdmaLength;
	uint32_t SdmaSrcReloadValue;
	uint32_t SdmaLengthReloadValue;

	uint16_t GdmaDest;
	uint16_t GdmaLength;
	uint8_t GdmaControl;
	uint8_t SdmaControl;

	bool SdmaEnabled;
	bool SdmaDecrement;
	bool SdmaHyperVoice;
	bool SdmaRepeat;
	bool SdmaHold;
	uint8_t SdmaFrequency;
	uint8_t SdmaTimer;
};

struct WsTimerState
{
	uint16_t HTimer;
	uint16_t VTimer;

	uint16_t HReloadValue;
	uint16_t VReloadValue;

	uint8_t Control;
	bool HBlankEnabled;
	bool HBlankAutoReload;
	bool VBlankEnabled;
	bool VBlankAutoReload;
};

struct BaseWsApuState
{
	uint16_t Frequency;
	uint16_t Timer;

	bool Enabled;
	uint8_t LeftVolume;
	uint8_t RightVolume;

	uint8_t SamplePosition;
	uint8_t LeftOutput;
	uint8_t RightOutput;

	void SetVolume(uint8_t value)
	{
		RightVolume = value & 0x0F;
		LeftVolume = (value >> 4);
	}

	uint8_t GetVolume()
	{
		return RightVolume | (LeftVolume << 4);
	}
};

struct WsApuCh1State : public BaseWsApuState {};

struct WsApuCh2State : public BaseWsApuState
{
	bool PcmEnabled;
	bool MaxPcmVolumeRight;
	bool HalfPcmVolumeRight;
	bool MaxPcmVolumeLeft;
	bool HalfPcmVolumeLeft;
};

struct WsApuCh3State : public BaseWsApuState
{
	uint16_t SweepScaler;
	bool SweepEnabled;
	int8_t SweepValue;
	uint8_t SweepPeriod;
	uint8_t SweepTimer;
	bool UseSweepCpuClock;
};

struct WsApuCh4State : public BaseWsApuState
{
	bool NoiseEnabled;
	bool LfsrEnabled;
	uint8_t TapMode;
	uint8_t TapShift;
	uint16_t Lfsr;
	uint8_t HoldLfsr;
};

enum class WsHyperVoiceScalingMode : uint8_t
{
	Unsigned,
	UnsignedNegated,
	Signed,
	None
};

enum class WsHyperVoiceChannelMode : uint8_t
{
	Stereo,
	MonoLeft,
	MonoRight,
	MonoBoth
};

struct WsApuHyperVoiceState
{
	int16_t LeftOutput;
	int16_t RightOutput;

	bool Enabled;

	uint8_t LeftSample;
	uint8_t RightSample;
	bool UpdateRightValue;

	uint8_t Divisor;
	uint8_t Timer;
	uint8_t Input;
	uint8_t Shift;
	WsHyperVoiceChannelMode ChannelMode;
	WsHyperVoiceScalingMode ScalingMode;

	uint8_t ControlLow;
	uint8_t ControlHigh;
};

struct WsApuState
{
	WsApuCh1State Ch1;
	WsApuCh2State Ch2;
	WsApuCh3State Ch3;
	WsApuCh4State Ch4;
	WsApuHyperVoiceState Voice;

	uint16_t WaveTableAddress;
	bool SpeakerEnabled;
	uint8_t SpeakerVolume;
	uint8_t InternalMasterVolume;
	uint8_t MasterVolume;
	bool HeadphoneEnabled;

	bool HoldChannels;
	bool ForceOutput2;
	bool ForceOutput4;
	bool ForceOutputCh2Voice;
};

struct WsSerialState
{
	uint64_t SendClock;

	bool Enabled;
	bool HighSpeed;
	bool ReceiveOverflow;

	bool HasReceiveData;
	uint8_t ReceiveBuffer;

	bool HasSendData;
	uint8_t SendBuffer;
};

enum class WsEepromSize : uint16_t
{
	Size0 = 0,
	Size128 = 0x80,
	Size1kb = 0x400,
	Size2kb = 0x800
};

struct WsEepromState
{
	WsEepromSize Size;
	uint16_t ReadBuffer;
	uint16_t WriteBuffer;
	uint16_t Command;
	uint16_t Control;
	bool WriteDisabled;
	bool ReadDone;
	bool Idle;

	bool InternalEepromWriteProtected;
};

struct WsCartState
{
	uint8_t SelectedBanks[4];
};

struct WsState
{
	WsCpuState Cpu;
	WsPpuState Ppu;
	WsApuState Apu;
	WsMemoryManagerState MemoryManager;
	WsControlManagerState ControlManager;
	WsDmaControllerState DmaController;
	WsTimerState Timer;
	WsSerialState Serial;
	WsEepromState InternalEeprom;
	WsCartState Cart;
	WsEepromState CartEeprom;
	WsModel Model;
};

class WsCpuParityTable
{
private:
	uint8_t _parityTable[0x100];

public:
	WsCpuParityTable()
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
