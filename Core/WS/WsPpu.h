#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Shared/Emulator.h"
#include "Shared/SettingTypes.h"
#include "Utilities/ISerializable.h"

class Emulator;
class WsTimer;
class WsConsole;

class WsPpu final : public ISerializable
{
private:
	WsPpuState _state = {};
	Emulator* _emu = nullptr;
	WsConsole* _console = nullptr;
	WsTimer* _timer = nullptr;
	uint16_t* _outputBuffers[2] = {};
	uint16_t* _currentBuffer = nullptr;
	uint8_t* _vram = nullptr;
	
	uint8_t _spriteRam[512] = {};

	struct PixelData
	{
		uint8_t Palette;
		uint8_t Color;
		uint8_t Priority;
	};

	PixelData _rowData[2][224];

	uint16_t _screenHeight = 0;
	uint16_t _screenWidth = 0;
	bool _showIcons = false;

	void ProcessEndOfScanline();
	void ProcessSpriteCopy();

	void DrawIcons();
	void DrawIcon(bool visible, const uint16_t icon[11], uint8_t position);
	uint8_t GetLcdStatus();

	void SendFrame();

	template<WsVideoMode mode> void DrawScanline();
	template<WsVideoMode mode> void DrawSprites();
	template<WsVideoMode mode, int layerIndex> void DrawBackground();

	template<WsVideoMode mode> __forceinline uint16_t GetPixelColor(uint16_t tileAddr, uint8_t column);

	__forceinline uint16_t GetBgColor()
	{
		if(_state.Mode == WsVideoMode::Monochrome) {
			uint8_t bgBrightness = _state.BwShades[_state.BgColor & 0x07] ^ 0x0F;
			return bgBrightness | (bgBrightness << 4) | (bgBrightness << 8);
		} else {
			return _vram[0xFE00 | (_state.BgColor << 1)] | ((_vram[0xFE00 | (_state.BgColor << 1) | 1] & 0x0F) << 8);
		}
	}

	__forceinline uint16_t GetPixelRgbColor(WsVideoMode mode, uint8_t color, uint8_t palette)
	{
		switch(mode) {
			case WsVideoMode::Monochrome:
			{
				uint8_t shadeValue = _state.BwPalettes[(palette << 2) | color];
				uint8_t brightness = _state.BwShades[shadeValue] ^ 0x0F;
				return brightness | (brightness << 4) | (brightness << 8);
			}

			case WsVideoMode::Color2bpp:
			case WsVideoMode::Color4bpp:
			case WsVideoMode::Color4bppPacked:
			{
				uint16_t paletteAddr = 0xFE00 | (palette << 5) | (color << 1);
				return _vram[paletteAddr] | ((_vram[paletteAddr + 1] & 0x0F) << 8);
			}
		}

		return 0;
	}
	
	void ProcessHblank();

public:
	WsPpu(Emulator* emu, WsConsole* console, WsTimer* timer, uint8_t* vram);
	~WsPpu();

	__forceinline void Exec()
	{
		if(_state.Scanline == WsConstants::ScreenHeight) {
			ProcessSpriteCopy();
		}

		if(_state.Cycle < 224) {
			if(_state.Scanline < WsConstants::ScreenHeight + 1 && _state.Scanline > 0) {
				//Palette lookup + output pixel on the first 224 cycles
				uint8_t rowIndex = (_state.Scanline & 0x01) ^ 1;
				PixelData& data = _rowData[rowIndex][_state.Cycle];
				uint16_t screenWidth = _showIcons ? _screenWidth : WsConstants::ScreenWidth;
				uint16_t offset = (_state.Scanline - 1) * screenWidth + _state.Cycle;
				_currentBuffer[offset] = data.Priority == 0 ? GetBgColor() : GetPixelRgbColor(_state.Mode, data.Color, data.Palette);
			}
		} else {
			if(_state.Cycle == 224) {
				ProcessHblank();
			} else if(_state.Cycle == 255) {
				ProcessEndOfScanline();
				_state.Cycle = -1;
			}
		}

		_state.Cycle++;
		_emu->ProcessPpuCycle<CpuType::Ws>();
	}

	void SetVideoMode(WsVideoMode mode);

	uint8_t ReadPort(uint16_t port);
	void WritePort(uint16_t port, uint8_t value);

	uint8_t ReadLcdConfigPort(uint16_t port);
	void WriteLcdConfigPort(uint16_t port, uint8_t value);

	uint16_t GetCycle() { return _state.Cycle; }
	uint16_t GetScanline() { return _state.Scanline; }
	uint16_t GetScanlineCount() { return _state.LastScanline + 1; }
	uint32_t GetFrameCount() { return _state.FrameCount; }
	uint16_t GetScreenWidth() { return _showIcons ? _screenWidth : WsConstants::ScreenWidth; }
	uint16_t GetScreenHeight() { return _showIcons ? _screenHeight : WsConstants::ScreenHeight; }
	WsPpuState& GetState() { return _state; }

	uint16_t GetVisibleScanlineCount();
	uint16_t* GetScreenBuffer(bool prevFrame);

	void DebugSendFrame();
	void SetOutputToBgColor();

	void ShowVolumeIcon();

	void Serialize(Serializer& s) override;
};
