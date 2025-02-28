#pragma once
#include "pch.h"
#include "SMS/SmsTypes.h"
#include "Shared/SettingTypes.h"
#include "Shared/ColorUtilities.h"
#include "Utilities/ISerializable.h"

class Emulator;
class SmsConsole;
class SmsCpu;
class SmsControlManager;
class SmsMemoryManager;

enum class SmsVdpMemAccess : uint8_t
{
	None = 0,
	BgLoadTable = 1,
	BgLoadTile = 2,
	SpriteEval = 3,
	SpriteLoadTable = 4,
	SpriteLoadTile = 5,
	CpuSlot = 6
};

class SmsVdp final : public ISerializable
{
public:
	static constexpr int SmsVdpLeftBorder = 8;

private:
	Emulator* _emu = nullptr;
	SmsConsole* _console = nullptr;
	SmsCpu* _cpu = nullptr;
	SmsControlManager* _controlManager = nullptr;
	SmsMemoryManager* _memoryManager = nullptr;

	uint8_t* _videoRam = nullptr;
	uint16_t _internalPaletteRam[0x20] = {};

	uint16_t _smsSgPalette[0x10] = {
		ColorUtilities::Rgb222To555(0x00), ColorUtilities::Rgb222To555(0x00), ColorUtilities::Rgb222To555(0x08), ColorUtilities::Rgb222To555(0x0C),
		ColorUtilities::Rgb222To555(0x10), ColorUtilities::Rgb222To555(0x30), ColorUtilities::Rgb222To555(0x01), ColorUtilities::Rgb222To555(0x3C),
		ColorUtilities::Rgb222To555(0x02), ColorUtilities::Rgb222To555(0x03), ColorUtilities::Rgb222To555(0x05), ColorUtilities::Rgb222To555(0x0F),
		ColorUtilities::Rgb222To555(0x04), ColorUtilities::Rgb222To555(0x33), ColorUtilities::Rgb222To555(0x15), ColorUtilities::Rgb222To555(0x3F),
	};

	static constexpr uint16_t _originalSgPalette[0x10] = { 
		0x0000, 0x0000, 0x2324, 0x3f6b, 0x754a, 0x7dcf, 0x255a, 0x7ba8,
		0x295f, 0x3dff, 0x2b1a, 0x433c, 0x1ec4, 0x5d79, 0x6739, 0x7fff
	};

	const uint16_t* _activeSgPalette = nullptr;
	bool _disableBackground = false;
	bool _disableSprites = false;
	bool _removeSpriteLimit = false;
	SmsModel _model = {};
	SmsRevision _revision = {};

	uint16_t* _outputBuffers[2] = {};
	uint16_t* _currentOutputBuffer = nullptr;

	SmsVdpState _state = {};
	uint64_t _lastMasterClock = 0;

	uint32_t _bgShifters[4] = {};
	uint32_t _bgPriority = 0;
	uint32_t _bgPalette = 0;
	uint16_t _bgTileAddr = 0;
	uint16_t _bgOffsetY = 0;
	uint16_t _minDrawCycle = 0;
	uint8_t _pixelsAvailable = 0;
	bool _bgHorizontalMirror = false;

	struct SpriteShifter
	{
		uint8_t TileData[4] = {};
		uint16_t TileAddr = 0;
		int16_t SpriteX = 0;
		uint8_t SpriteRow = 0;
		bool HardwareSprite = false;
	};

	uint8_t _evalCounter = 0;
	uint8_t _inRangeSpriteCount = 0;
	bool _spriteOverflowPending = false;
	
	uint8_t _spriteIndex = 0;
	uint8_t _inRangeSpriteIndex = 0;
	uint8_t _spriteCount = 0;
	uint8_t _inRangeSprites[64] = {};
	SpriteShifter _spriteShifters[64];

	uint8_t _paletteRam[0x40] = {};
	uint16_t _scanlineCount = 262;
	ConsoleRegion _region = ConsoleRegion::Ntsc;

	SmsVdpWriteType _writePending = SmsVdpWriteType::None;
	bool _readPending = false;

	bool _latchRequest = false;
	uint8_t _latchPos = 0;

	bool _needCramDot = false;
	uint16_t _cramDotColor = 0;

	//Used by SG-1000 modes
	uint16_t _bgTileIndex = 0;
	uint8_t _bgPatternData = 0;
	uint8_t _textModeStep = 0;

	SmsVdpMemAccess _memAccess[342] = {};

	void UpdateIrqState();

	void UpdateDisplayMode();

	uint8_t ReadVerticalCounter();

	__forceinline uint8_t ReadVram(uint16_t addr, SmsVdpMemAccess type);
	__forceinline void WriteVram(uint16_t addr, uint8_t value, SmsVdpMemAccess type);

	void DebugProcessMemoryAccessView();
	__forceinline void ProcessVramAccess();
	void ProcessVramWrite();

	uint8_t ReverseBitOrder(uint8_t val);
	
	__forceinline void Exec();
	__forceinline void ExecForcedBlank();
	__forceinline void ProcessForcedBlankVblank();

	int GetVisiblePixelIndex();
	__forceinline void LoadBgTilesSms();
	void LoadBgTilesSg();
	void LoadBgTilesSgTextMode();
	void PushBgPixel(uint8_t color, int index);
	
	__forceinline void DrawPixel();

	void ProcessScanlineEvents();
	void ProcessEndOfScanline();

	__forceinline void ProcessSpriteEvaluation();

	uint16_t GetSmsSpriteTileAddr(uint8_t sprTileIndex, uint8_t spriteRow, uint8_t i);
	void LoadSpriteTilesSms();
	void LoadExtraSpritesSms();
	__forceinline uint16_t GetPixelColor();

	void LoadSpriteTilesSg();
	void LoadExtraSpritesSg();
	void ShiftSprite(uint8_t sprIndex);
	void ShiftSpriteSg(uint8_t sprIndex);

	__forceinline bool IsZoomedSpriteAllowed(int spriteIndex);

	void WriteRegister(uint8_t reg, uint8_t value);
	void WriteSmsPalette(uint8_t addr, uint8_t value);
	void WriteGameGearPalette(uint8_t addr, uint16_t value);

	void InitSmsPostBiosState();
	void InitGgPowerOnState();
	
	void UpdateConfig();

public:
	void Init(Emulator* emu, SmsConsole* console, SmsCpu* cpu, SmsControlManager* controlManager, SmsMemoryManager* memoryManager);
	~SmsVdp();

	void Run(uint64_t runTo);

	void WritePort(uint8_t port, uint8_t value);
	uint8_t ReadPort(uint8_t port);
	uint8_t PeekPort(uint8_t port);

	void SetLocationLatchRequest(uint8_t x);
	void InternalLatchHorizontalCounter(uint16_t cycle);
	void LatchHorizontalCounter();
	void SetRegion(ConsoleRegion region);

	void DebugSendFrame();
	uint16_t GetScanline() { return _state.Scanline; }
	uint16_t GetScanlineCount() { return _scanlineCount; }
	uint16_t GetCycle() { return _state.Cycle; }
	uint16_t GetFrameCount() { return _state.FrameCount; }
	uint32_t GetPixelBrightness(uint8_t x, uint8_t y);
	int GetViewportYOffset();
	const uint16_t* GetSmsSgPalette() { return _activeSgPalette; }
	SmsVdpState& GetState() { return _state; }

	void DebugWritePalette(uint8_t addr, uint8_t value);

	uint16_t* GetScreenBuffer(bool previousBuffer)
	{
		return previousBuffer ? ((_currentOutputBuffer == _outputBuffers[0]) ? _outputBuffers[1] : _outputBuffers[0]) : _currentOutputBuffer;
	}

	void Serialize(Serializer& s) override;
};