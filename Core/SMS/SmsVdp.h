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

class SmsVdp final : public ISerializable
{
public:
	static constexpr int SmsVdpLeftBorder = 13;

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
		int16_t SpriteX = 0;
	};

	uint8_t _spriteCount = 0;
	SpriteShifter _spriteShifters[64];

	uint8_t _paletteRam[0x40] = {};
	uint16_t _scanlineCount = 262;
	ConsoleRegion _region = ConsoleRegion::Ntsc;

	uint8_t _writeBuffer = 0;
	SmsVdpWriteType _writePending = SmsVdpWriteType::None;

	bool _latchRequest = false;
	uint8_t _latchPos = 0;

	bool _needCramDot = false;
	uint16_t _cramDotColor = 0;

	//Used by SG-1000 modes
	uint16_t _bgTileIndex = 0;
	uint8_t _bgPatternData = 0;
	uint8_t _textModeStep = 0;

	void UpdateIrqState();

	void UpdateDisplayMode();

	uint8_t ReadVerticalCounter();

	uint8_t ReverseBitOrder(uint8_t val);
	
	__forceinline void Exec();
	__forceinline void ProcessVramWrite();
	__forceinline int GetVisiblePixelIndex();
	__forceinline void LoadBgTilesSms();
	void LoadBgTilesSg();
	void LoadBgTilesSgTextMode();
	__forceinline void PushBgPixel(uint8_t color, int index);
	__forceinline void DrawPixel();
	__forceinline void ProcessScanlineEvents();
	__forceinline uint16_t GetPixelColor();

	void WriteRegister(uint8_t reg, uint8_t value);
	void WriteSmsPalette(uint8_t addr, uint8_t value);
	void WriteGameGearPalette(uint8_t addr, uint16_t value);

	void LoadSpritesSms();
	void LoadSpritesSg();
	void ShiftSprite(uint8_t sprIndex);
	void ShiftSpriteSg(uint8_t sprIndex);

	bool IsZoomedSpriteAllowed(int spriteIndex);

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