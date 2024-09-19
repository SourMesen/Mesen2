#include "pch.h"
#include "WS/WsConsole.h"
#include "WS/WsConsole.h"
#include "WS/WsTimer.h"
#include "WS/WsControlManager.h"
#include "WS/WsMemoryManager.h"
#include "WS/APU/WsApu.h"
#include "Shared/EmuSettings.h"
#include "Shared/NotificationManager.h"
#include "Shared/RewindManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/RenderedFrame.h"
#include "Shared/EventType.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

WsPpu::WsPpu(Emulator* emu, WsConsole* console, WsTimer* timer, uint8_t* vram)
{
	_emu = emu;
	_console = console;
	_timer = timer;
	_vram = vram;

	_state.LastScanline = 158;
	_state.BackPorchScanline = 155;
	_state.ShowVolumeIconFrame = UINT32_MAX;

	_screenWidth = WsConstants::ScreenWidth;
	_screenHeight = WsConstants::ScreenHeight;
	if(console->GetModel() == WsModel::Monochrome) {
		_screenHeight += 13;
	} else {
		_screenWidth += 13;
	}

	_outputBuffers[0] = new uint16_t[WsConstants::MaxPixelCount];
	_outputBuffers[1] = new uint16_t[WsConstants::MaxPixelCount];
	memset(_outputBuffers[0], 0, WsConstants::MaxPixelCount * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, WsConstants::MaxPixelCount * sizeof(uint16_t));
	_currentBuffer = _outputBuffers[0];

	_showIcons = _emu->GetSettings()->GetWsConfig().LcdShowIcons;
}

WsPpu::~WsPpu()
{
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
}

void WsPpu::SetVideoMode(WsVideoMode mode)
{
	_state.NextMode = mode;
}

void WsPpu::ProcessHblank()
{
	_timer->TickHorizontalTimer();
	if(_state.Scanline < WsConstants::ScreenHeight) {
		switch(_state.Mode) {
		case WsVideoMode::Monochrome: DrawScanline<WsVideoMode::Monochrome>(); break;
		case WsVideoMode::Color2bpp: DrawScanline<WsVideoMode::Color2bpp>(); break;
		case WsVideoMode::Color4bpp: DrawScanline<WsVideoMode::Color4bpp>(); break;
		case WsVideoMode::Color4bppPacked: DrawScanline<WsVideoMode::Color4bppPacked>(); break;
		}
	}
}

void WsPpu::ProcessEndOfScanline()
{
	_state.Cycle = 0;
	_state.Scanline++;

	_state.BgLayers[0].Latch();
	_state.BgLayers[1].Latch();
	_state.BgWindow.Latch();
	_state.SpriteWindow.Latch();
	_state.SpritesEnabledLatch = _state.SpritesEnabled;
	_state.DrawOutsideBgWindowLatch = _state.DrawOutsideBgWindow;

	if(_state.Scanline > _state.LastScanline) {
		if(_state.Scanline <= 145) {
			//Support sending frame to LCD even when number of scanlines is less than the 144px resolution
			SendFrame();
		}
		_state.Mode = _state.NextMode;
		_state.Scanline = 0;
		_emu->ProcessEvent(EventType::StartFrame, CpuType::Ws);
		_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
		_showIcons = _emu->GetSettings()->GetWsConfig().LcdShowIcons;
	} else if(_state.Scanline == 145) {
		SendFrame();
	} else if(_state.Scanline == 144) {
		_timer->TickVerticalTimer();
		_console->GetMemoryManager()->SetIrqSource(WsIrqSource::VerticalBlank);
	}

	if(_state.Scanline == _state.IrqScanline) {
		_console->GetMemoryManager()->SetIrqSource(WsIrqSource::Scanline);
	}
}

template<WsVideoMode mode>
void WsPpu::DrawScanline()
{
	uint8_t rowIndex = _state.Scanline & 0x01;
	std::fill(_rowData[rowIndex], _rowData[rowIndex] + WsConstants::ScreenWidth, PixelData{});

	DrawSprites<mode>();
	DrawBackground<mode, 0>();
	DrawBackground<mode, 1>();
}

template<WsVideoMode mode>
void WsPpu::DrawSprites()
{
	WsConfig& cfg = _emu->GetSettings()->GetWsConfig();
	if(!_state.SpritesEnabled || cfg.DisableSprites) {
		return;
	}

	uint16_t scanline = _state.Scanline;
	uint8_t rowIndex = _state.Scanline & 0x01;

	constexpr int tileSize = mode >= WsVideoMode::Color4bpp ? 32 : 16;
	constexpr int tileBytesPerRow = mode >= WsVideoMode::Color4bpp ? 4 : 2;
	constexpr int bank0Addr = mode >= WsVideoMode::Color4bpp ? 0x4000 : 0x2000;

	int spriteCount = 0;
	for(int i = 0; i < _state.SpriteCountLatch; i++) {
		uint16_t addr = i * 4;
		uint8_t attributes = _spriteRam[addr + 1];
		bool highPriority = attributes & 0x20;

		uint8_t y = _spriteRam[addr + 2];
		uint8_t tileRow = scanline - y;

		if(tileRow < 8 && scanline < y + 8) {
			spriteCount++;

			bool showOutsideWindow = attributes & 0x10;
			bool vMirror = attributes & 0x80;
			bool hMirror = attributes & 0x40;

			uint16_t tileIndex = _spriteRam[addr] | ((attributes & 0x01) << 8);
			uint8_t palette = ((attributes >> 1) & 0x07) + 8;
			uint8_t sprX = _spriteRam[addr + 3];

			if(vMirror) {
				tileRow = 7 - tileRow;
			}

			uint16_t tileDataAddr = (bank0Addr + tileIndex * tileSize + tileRow * tileBytesPerRow);
			for(int j = 0; j < 8; j++) {
				uint8_t x = sprX + j;

				if(x >= 224) {
					continue;
				}
				else if(_rowData[rowIndex][x].Priority > 0) {
					continue;
				}
				else if(_state.SpriteWindow.EnabledLatch && showOutsideWindow == _state.SpriteWindow.IsInsideWindow(x, scanline)) {
					//Don't draw this pixel, it's outside/inside the window and should only be drawn on the other side
					continue;
				}

				int tileColumn = j;
				if(hMirror) {
					tileColumn = 7 - tileColumn;
				}

				uint8_t color = GetPixelColor<mode>(tileDataAddr, tileColumn);
				if(color != 0 || (!(palette & 0x04) && mode <= WsVideoMode::Color2bpp)) {
					_rowData[rowIndex][x] = { palette, color, highPriority ? (uint8_t)2 : (uint8_t)1 };
				}
			}
		}

		if(spriteCount >= 32) {
			break;
		}
	}
}

template<WsVideoMode mode, int layerIndex>
void WsPpu::DrawBackground()
{
	constexpr int tileSize = mode >= WsVideoMode::Color4bpp ? 32 : 16;
	constexpr int tileBytesPerRow = mode >= WsVideoMode::Color4bpp ? 4 : 2;
	constexpr int bank0Addr = mode >= WsVideoMode::Color4bpp ? 0x4000 : 0x2000;
	constexpr int bank1Addr = mode >= WsVideoMode::Color4bpp ? 0x8000 : 0x4000;

	WsConfig& cfg = _emu->GetSettings()->GetWsConfig();
	uint8_t rowIndex = _state.Scanline & 0x01;
	WsBgLayer& layer = _state.BgLayers[layerIndex];
	if(cfg.HideBgLayers[layerIndex] || !layer.EnabledLatch) {
		return;
	}

	uint16_t scanline = _state.Scanline;
	uint16_t layerAddr = (uint16_t)(layer.MapAddressLatch & (_console->GetModel() == WsModel::Monochrome ? 0x3FFF : 0x7FFF));

	for(int cycle = 0; cycle < WsConstants::ScreenWidth; cycle++) {
		int y = (scanline + layer.ScrollYLatch) & 0xFF;
		int x = (cycle + layer.ScrollXLatch) & 0xFF;
		int row = y / 8;
		int column = x / 8;
		uint16_t tilemapAddr = layerAddr + row * 32 * 2 + column * 2;
		uint16_t tilemapData = _vram[tilemapAddr] | (_vram[tilemapAddr + 1] << 8);

		int tileRow = y & 0x07;
		int tileColumn = x & 0x07;
		int counter = 8 - tileColumn;

		uint16_t tileIndex = tilemapData & 0x1FF;
		uint8_t palette = (tilemapData >> 9) & 0x0F;
		bool vMirror = tilemapData & 0x8000;
		bool hMirror = tilemapData & 0x4000;
		if(hMirror) {
			tileColumn = 7 - tileColumn;
		}
		if(vMirror) {
			tileRow = 7 - tileRow;
		}

		uint16_t tilesetAddr = mode >= WsVideoMode::Color2bpp && (tilemapData & 0x2000) ? bank1Addr : bank0Addr;
		uint16_t tileDataAddr = (tilesetAddr + tileIndex * tileSize + tileRow * tileBytesPerRow);

		for(int i = cycle, end = std::min<int>(cycle + counter, WsConstants::ScreenWidth); i < end; i++) {
			uint8_t color = GetPixelColor<mode>(tileDataAddr, tileColumn);
			tileColumn += (hMirror ? -1 : 1);

			if(_rowData[rowIndex][i].Priority >= layerIndex + 1) {
				continue;
			}

			if constexpr(layerIndex == 1) {
				if(_state.BgWindow.EnabledLatch && _state.DrawOutsideBgWindowLatch == _state.BgWindow.IsInsideWindow(i, scanline)) {
					//Pixel is hidden by window
					continue;
				}
			}

			//"In two bit per pixel modes: Palettes 0-3 and 8-11 are opaque. For these, index zero is treated as opaque."
			if(color != 0 || (!(palette & 0x04) && mode <= WsVideoMode::Color2bpp)) {
				_rowData[rowIndex][i] = { palette, color, (uint8_t)(layerIndex + 1) };
			}
		}

		cycle += counter - 1;
	}
}

template<WsVideoMode mode>
uint16_t WsPpu::GetPixelColor(uint16_t tileAddr, uint8_t column)
{
	switch(mode) {
	case WsVideoMode::Monochrome: {
		uint8_t tileData = _vram[tileAddr];
		uint8_t tileData2 = _vram[tileAddr + 1];
		return (
			((tileData << column) & 0x80) >> 7 |
			((tileData2 << column) & 0x80) >> 6
			);
	}

	case WsVideoMode::Color2bpp: {
		uint8_t tileData = _vram[tileAddr];
		uint8_t tileData2 = _vram[tileAddr + 1];
		return (
			((tileData << column) & 0x80) >> 7 |
			((tileData2 << column) & 0x80) >> 6
			);
	}

	case WsVideoMode::Color4bpp: {
		uint8_t tileData = _vram[tileAddr];
		uint8_t tileData2 = _vram[tileAddr + 1];
		uint8_t tileData3 = _vram[tileAddr + 2];
		uint8_t tileData4 = _vram[tileAddr + 3];
		return (
			((tileData << column) & 0x80) >> 7 |
			((tileData2 << column) & 0x80) >> 6 |
			((tileData3 << column) & 0x80) >> 5 |
			((tileData4 << column) & 0x80) >> 4
			);
	}

	case WsVideoMode::Color4bppPacked: {
		return (_vram[tileAddr + column / 2] >> (column & 0x01 ? 0 : 4)) & 0x0F;
	}
	}

	return 0;
}

void WsPpu::ProcessSpriteCopy()
{
	if(_state.Cycle == 0) {
		_state.SpriteCountLatch = _state.SpriteCount;
	}

	uint16_t baseAddr = _state.SpriteTableAddress & (_console->GetModel() == WsModel::Monochrome ? 0x3FFF : 0x7FFF);

	int i = _state.Cycle << 1;
	_spriteRam[i] = _vram[baseAddr + (((_state.FirstSpriteIndex * 4) + i) & 0x1FF)];
	_spriteRam[i+1] = _vram[baseAddr + (((_state.FirstSpriteIndex * 4) + i + 1) & 0x1FF)];
}

void WsPpu::DrawIcons()
{
	//11x11 1bpp icons (same shapes as ares)
	static constexpr uint16_t power[11] = { 0x70, 0xD8, 0x18C, 0x18C, 0x3DE, 0x3FE, 0x3FE, 0x1FC, 0x1FC, 0xF8, 0x70 };
	//static constexpr uint16_t initialized[11] = { 0, 0x3FE, 0x7FF, 0x603, 0x603, 0x603, 0x7FF, 0x7FF, 0x7FF, 0x3FE, 0 };
	static constexpr uint16_t sleep[11] = { 0x80, 0x250, 0x11B, 0x1F, 0x3E, 0xFC, 0x3C, 0x1E, 0x3E, 0x33, 0 };
	//static constexpr uint16_t lowBattery[11] = { 0x70, 0xD8, 0x88, 0x88, 0x88, 0x88, 0x88, 0x98, 0xB8, 0x88, 0xF8 };
	static constexpr uint16_t volumeWsOff[11] = { 0, 0x10, 0x18, 0x1C, 0x1E, 0x1E, 0x1E, 0x1C, 0x18, 0x10, 0 };
	static constexpr uint16_t volumeWsLow[11] = { 0, 0x10, 0x18, 0x5C, 0x9E, 0x9E, 0x9E, 0x5C, 0x18, 0x10, 0 };
	static constexpr uint16_t volumeWsHigh[11] = { 0, 0x90, 0x118, 0x25C, 0x29E, 0x29E, 0x29E, 0x25C, 0x118, 0x90, 0 };
	static constexpr uint16_t volumeWscOff[11] = { 0, 0, 0, 0, 0, 0, 0, 0x3FE, 0x1FC, 0xF8, 0x70 };
	static constexpr uint16_t volumeWscLow[11] = { 0, 0, 0, 0xF8, 0x104, 0, 0, 0x3FE, 0x1FC, 0xF8, 0x70 };
	static constexpr uint16_t volumeWscMed[11] = { 0x1FC, 0x202, 0x401, 0, 0x70, 0x88, 0, 0x3FE, 0x1FC, 0xF8, 0x70 };
	static constexpr uint16_t volumeWscHigh[11] = { 0x1FC, 0x202, 0x4F9, 0x104, 0x272, 0x88, 0, 0x3FE, 0x1FC, 0xF8, 0x70 };
	static constexpr uint16_t headphones[11] = { 0x70, 0x18C, 0x306, 0x20B, 0x41F, 0x40E, 0x447, 0x2E2, 0x370, 0x1F8, 0xD0 };
	static constexpr uint16_t verticalIcon[11] = { 0x6, 0x1E, 0x38, 0x3BF, 0x7FF, 0x7FF, 0x7FF, 0x3BF, 0x38, 0x1E, 0x6 };
	static constexpr uint16_t horizontalIcon[11] = { 0x70, 0xF8, 0xF8, 0xF8, 0x70, 0x1FC, 0x3FE, 0x3FE, 0x6FB, 0x6FB, 0xF8 };
	static constexpr uint16_t aux3[11] = { 0, 0x70, 0x1FC, 0x1FC, 0x3FE, 0x3FE, 0x3FE, 0x1FC, 0x1FC, 0x70, 0 };
	static constexpr uint16_t aux2[11] = { 0, 0, 0x70, 0xF8, 0x1FC, 0x1FC, 0x1FC, 0xF8, 0x70, 0, 0 };
	static constexpr uint16_t aux1[11] = { 0, 0, 0, 0, 0x70, 0x70, 0x70, 0, 0, 0, 0 };

	if(_console->GetModel() == WsModel::Monochrome) {
		uint16_t* start = _currentBuffer + (WsConstants::ScreenWidth * WsConstants::ScreenHeight);
		std::fill(start, start + WsConstants::ScreenWidth * 13, 0xFFF);
	} else {
		for(int i = 0; i < WsConstants::ScreenHeight; i++) {
			uint16_t* start = _currentBuffer + (i * _screenWidth) + WsConstants::ScreenWidth;
			std::fill(start, start + 13, 0);
		}
	}

	DrawIcon(true, power, 0);
	//DrawIcon(true, initialized, 13);
	DrawIcon(_state.Icons.Sleep, sleep, 26);
	//DrawIcon(true, lowBattery, 39, 144);
	if(_state.ShowVolumeIconFrame <= _state.FrameCount && _state.FrameCount - _state.ShowVolumeIconFrame < 128) {
		//Show speaker/headphone icons if sound button was pressed within the last 128 frames
		if(_emu->GetSettings()->GetWsConfig().AudioMode == WsAudioMode::Headphones) {
			DrawIcon(true, headphones, 65);
		} else {
			if(_console->GetModel() == WsModel::Monochrome) {
				switch(_console->GetApu()->GetMasterVolume()) {
					default: case 0: DrawIcon(true, volumeWsOff, 52); break;
					case 1: DrawIcon(true, volumeWsLow, 52); break;
					case 2: DrawIcon(true, volumeWsHigh, 52); break;
				}
			} else {
				switch(_console->GetApu()->GetMasterVolume()) {
					default: case 0: DrawIcon(true, volumeWscOff, 52); break;
					case 1: DrawIcon(true, volumeWscLow, 52); break;
					case 2: DrawIcon(true, volumeWscMed, 52); break;
					case 3: DrawIcon(true, volumeWscHigh, 52); break;
				}
			}
		}
	}
		
	DrawIcon(_state.Icons.Vertical, verticalIcon, 78);
	DrawIcon(_state.Icons.Horizontal, horizontalIcon, 91);
	DrawIcon(_state.Icons.Aux3, aux3, 104);
	DrawIcon(_state.Icons.Aux2, aux2, 117);
	DrawIcon(_state.Icons.Aux1, aux1, 130);
}

void WsPpu::DrawIcon(bool visible, const uint16_t icon[11], uint8_t position)
{
	if(!visible) {
		return;
	}

	uint8_t xPos;
	uint8_t yPos;
	uint16_t color;
	if(_console->GetModel() == WsModel::Monochrome) {
		xPos = position + 1;
		yPos = 145;
		color = 0;
	} else {
		xPos = 225;
		yPos = WsConstants::ScreenHeight - 13 - position;
		color = 0xFFF;
	}

	for(int y = 0; y < 11; y++) {
		uint16_t basePos = (yPos + y) * _screenWidth + xPos;
		for(int x = 0; x < 11; x++) {
			if(icon[y] & (1 << x)) {
				_currentBuffer[basePos + x] = color;
			}
		}
	}
}

uint16_t WsPpu::GetVisibleScanlineCount()
{
	uint16_t scanlineCount = GetScanlineCount();
	return scanlineCount <= WsConstants::ScreenHeight ? scanlineCount - 1 : WsConstants::ScreenHeight;
}

uint16_t* WsPpu::GetScreenBuffer(bool prevFrame)
{
	return prevFrame ? ((_currentBuffer == _outputBuffers[0]) ? _outputBuffers[1] : _outputBuffers[0]) : _currentBuffer;
}

void WsPpu::DebugSendFrame()
{
	int offset = std::max(0, (int)(_state.Cycle + (_state.Scanline - 1) * _screenWidth));
	int pixelsToClear = WsConstants::MaxPixelCount - offset;
	if(pixelsToClear > 0) {
		memset(_currentBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
	}

	uint16_t width = _showIcons ? _screenWidth : WsConstants::ScreenWidth;
	uint16_t height = _showIcons ? _screenHeight : WsConstants::ScreenHeight;
	RenderedFrame frame(_currentBuffer, width, height, 1.0, _state.FrameCount);
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

void WsPpu::SetOutputToBgColor()
{
	std::fill(_currentBuffer, _currentBuffer + WsConstants::MaxPixelCount, GetBgColor());
}

void WsPpu::ShowVolumeIcon()
{
	_state.ShowVolumeIconFrame = _state.FrameCount;
}

uint8_t WsPpu::GetLcdStatus()
{
	uint8_t masterVolume = _console->GetApu()->GetMasterVolume();
	uint8_t volumeLevel;

	if(_console->GetModel() == WsModel::Monochrome) {
		switch(masterVolume) {
			default: case 0: volumeLevel = 0; break;
			case 1: volumeLevel = 2; break;
			case 2: volumeLevel = 3; break;
		}
	} else {
		switch(masterVolume) {
			default: case 0: volumeLevel = 0; break;
			case 1: volumeLevel = 2; break;
			case 2: volumeLevel = 1; break;
			case 3: volumeLevel = 3; break;
		}
	}

	bool headphone = false;
	bool speaker = false;
	if(_state.ShowVolumeIconFrame <= _state.FrameCount && _state.FrameCount - _state.ShowVolumeIconFrame < 128) {
		//Show speaker/headphone icons if sound button was pressed within the last 128 frames
		if(_emu->GetSettings()->GetWsConfig().AudioMode == WsAudioMode::Headphones) {
			headphone = true;
		} else {
			speaker = true;
		}
	}

	return (
		(_state.SleepEnabled ? 0x01 : 0) |
		(headphone ? 0x02 : 0) |
		(volumeLevel << 2) |
		(speaker ? 0x10 : 0)
	);
}

void WsPpu::SendFrame()
{
	if(_state.SleepEnabled || !_state.LcdEnabled || _state.LastScanline == 255) {
		//Screen should be white when in sleep mode, or if the last scanline is set to 255
		std::fill(_currentBuffer, _currentBuffer + WsConstants::MaxPixelCount, 0xFFF);
	} else if(_state.LastScanline < 144) {
		//Clear everything after the last scanline (results in less than 144 visible scanlines)
		std::fill(_currentBuffer + _state.LastScanline * _screenWidth, _currentBuffer + WsConstants::MaxPixelCount, 0xFFF);
	}

	if(_showIcons) {
		DrawIcons();
	}

	_emu->ProcessEvent(EventType::EndFrame, CpuType::Ws);
	_state.FrameCount++;

	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	uint16_t width = _showIcons ? _screenWidth : WsConstants::ScreenWidth;
	uint16_t height = _showIcons ? _screenHeight : WsConstants::ScreenHeight;
	RenderedFrame frame(_currentBuffer, width, height, 1.0, _state.FrameCount, _console->GetControlManager()->GetPortStates());
	bool rewinding = _emu->GetRewindManager()->IsRewinding();
	_emu->GetVideoDecoder()->UpdateFrame(frame, rewinding, rewinding);

	_emu->ProcessEndOfFrame();
	_console->ProcessEndOfFrame();
}

uint8_t WsPpu::ReadPort(uint16_t port)
{
	switch(port) {
	case 0x00: return _state.Control;
	case 0x01: return _state.BgColor & (_console->GetModel() == WsModel::Monochrome ? 0x07 : 0xFF);
	case 0x02: return _state.Scanline;
	case 0x03: return _state.IrqScanline;
	case 0x04: return (_state.SpriteTableAddress >> 9) & (_console->GetModel() == WsModel::Monochrome ? 0x1F : 0x3F);
	case 0x05: return _state.FirstSpriteIndex;
	case 0x06: return _state.SpriteCount;
	case 0x07: return _state.ScreenAddress & (_console->GetModel() == WsModel::Monochrome ? 0x77 : 0xFF);
	case 0x08: return _state.BgWindow.Left;
	case 0x09: return _state.BgWindow.Top;
	case 0x0A: return _state.BgWindow.Right;
	case 0x0B: return _state.BgWindow.Bottom;

	case 0x0C: return _state.SpriteWindow.Left;
	case 0x0D: return _state.SpriteWindow.Top;
	case 0x0E: return _state.SpriteWindow.Right;
	case 0x0F: return _state.SpriteWindow.Bottom;

	case 0x10: return _state.BgLayers[0].ScrollX;
	case 0x11: return _state.BgLayers[0].ScrollY;
	case 0x12: return _state.BgLayers[1].ScrollX;
	case 0x13: return _state.BgLayers[1].ScrollY;

	case 0x14:
		return (
			(_state.LcdEnabled ? 0x01 : 0) |
			(_state.HighContrast ? 0x02 : 0)
			);

	case 0x15: return _state.Icons.Value;
	case 0x16: return _state.LastScanline;
	case 0x17: return _state.BackPorchScanline;
	case 0x1A: return GetLcdStatus();

	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
		return (
			_state.BwShades[(port - 0x1C) * 2] |
			(_state.BwShades[(port - 0x1C) * 2 + 1] << 4)
			);

	default:
		if(port >= 0x20 && port <= 0x3F) {
			return (
				_state.BwPalettes[(port - 0x20) * 2] |
				(_state.BwPalettes[(port - 0x20) * 2 + 1] << 4)
				);
		}
		else {
			LogDebug("[Debug] PPU Read - missing handler: $" + HexUtilities::ToHex(port));
			return 0;
		}
	}
}

void WsPpu::WritePort(uint16_t port, uint8_t value)
{
	switch(port) {
	case 0x00:
		_state.Control = value & 0x3F;
		_state.BgLayers[0].Enabled = value & 0x01;
		_state.BgLayers[1].Enabled = value & 0x02;
		_state.SpritesEnabled = value & 0x04;
		_state.SpriteWindow.Enabled = value & 0x08;
		_state.DrawOutsideBgWindow = value & 0x10;
		_state.BgWindow.Enabled = value & 0x20;
		break;

	case 0x01: _state.BgColor = value & (_console->GetModel() == WsModel::Monochrome ? 0x07 : 0xFF); break;
	case 0x03: _state.IrqScanline = value; break;
	case 0x04: _state.SpriteTableAddress = (value & (_console->GetModel() == WsModel::Monochrome ? 0x1F : 0x3F)) << 9; break;

	case 0x05: _state.FirstSpriteIndex = value & 0x7F; break;
	case 0x06: _state.SpriteCount = value; break;

	case 0x07:
		_state.ScreenAddress = value & (_console->GetModel() == WsModel::Monochrome ? 0x77 : 0xFF);
		_state.BgLayers[0].MapAddress = (_state.ScreenAddress & 0x0F) << 11;
		_state.BgLayers[1].MapAddress = (_state.ScreenAddress & 0xF0) << 7;
		break;

	case 0x08: _state.BgWindow.Left = value; break;
	case 0x09: _state.BgWindow.Top = value; break;
	case 0x0A: _state.BgWindow.Right = value; break;
	case 0x0B: _state.BgWindow.Bottom = value; break;

	case 0x0C: _state.SpriteWindow.Left = value; break;
	case 0x0D: _state.SpriteWindow.Top = value; break;
	case 0x0E: _state.SpriteWindow.Right = value; break;
	case 0x0F: _state.SpriteWindow.Bottom = value; break;

	case 0x10: _state.BgLayers[0].ScrollX = value; break;
	case 0x11: _state.BgLayers[0].ScrollY = value; break;
	case 0x12: _state.BgLayers[1].ScrollX = value; break;
	case 0x13: _state.BgLayers[1].ScrollY = value; break;

	case 0x14:
		_state.LcdEnabled = value & 0x01;
		if(_console->GetModel() == WsModel::Color) {
			_state.HighContrast = value & 0x02;
		}
		break;

	case 0x15:
		_state.Icons.Sleep = value & 0x01;
		_state.Icons.Vertical = value & 0x02;
		_state.Icons.Horizontal = value & 0x04;
		_state.Icons.Aux1 = value & 0x08;
		_state.Icons.Aux2 = value & 0x10;
		_state.Icons.Aux3 = value & 0x20;
		_state.Icons.Value = value & 0x3F;
		break;

	case 0x16: _state.LastScanline = value; break;

	case 0x17:
		if(_console->GetModel() == WsModel::Color) {
			_state.BackPorchScanline = value; break;
		}
		break;

	case 0x1A:
		_state.SleepEnabled = value & 0x01;
		break;

	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
		_state.BwShades[(port - 0x1C) * 2] = value & 0x0F;
		_state.BwShades[(port - 0x1C) * 2 + 1] = (value >> 4) & 0x0F;
		break;

	default:
		if(port >= 0x20 && port <= 0x3F) {
			_state.BwPalettes[(port - 0x20) * 2] = value & 0x07;
			_state.BwPalettes[(port - 0x20) * 2 + 1] = (value >> 4) & 0x07;
		}
		else {
			LogDebug("[Debug] PPU Write - missing handler: $" + HexUtilities::ToHex(port) + "  = " + HexUtilities::ToHex(value));
		}
		break;

	}
}

uint8_t WsPpu::ReadLcdConfigPort(uint16_t port)
{
	return _state.LcdTftConfig[port - 0x70];
}

void WsPpu::WriteLcdConfigPort(uint16_t port, uint8_t value)
{
	_state.LcdTftConfig[port - 0x70] = value;
}

void WsPpu::Serialize(Serializer& s)
{
	SV(_state.FrameCount);
	SV(_state.Cycle);
	SV(_state.Scanline);
	SV(_state.LastScanline);
	
	for(int i = 0; i < 2; i++) {
		SVI(_state.BgLayers[i].Enabled);
		SVI(_state.BgLayers[i].EnabledLatch);
		SVI(_state.BgLayers[i].MapAddress);
		SVI(_state.BgLayers[i].MapAddressLatch);
		SVI(_state.BgLayers[i].ScrollX);
		SVI(_state.BgLayers[i].ScrollXLatch);
		SVI(_state.BgLayers[i].ScrollY);
		SVI(_state.BgLayers[i].ScrollYLatch);
	}

	SV(_state.BgWindow.Enabled);
	SV(_state.BgWindow.EnabledLatch);
	SV(_state.BgWindow.Bottom);
	SV(_state.BgWindow.BottomLatch);
	SV(_state.BgWindow.Top);
	SV(_state.BgWindow.TopLatch);
	SV(_state.BgWindow.Left);
	SV(_state.BgWindow.LeftLatch);
	SV(_state.BgWindow.Right);
	SV(_state.BgWindow.RightLatch);

	SV(_state.SpriteWindow.Enabled);
	SV(_state.SpriteWindow.EnabledLatch);
	SV(_state.SpriteWindow.Bottom);
	SV(_state.SpriteWindow.BottomLatch);
	SV(_state.SpriteWindow.Top);
	SV(_state.SpriteWindow.TopLatch);
	SV(_state.SpriteWindow.Left);
	SV(_state.SpriteWindow.LeftLatch);
	SV(_state.SpriteWindow.Right);
	SV(_state.SpriteWindow.RightLatch);

	SV(_state.DrawOutsideBgWindow);
	SV(_state.DrawOutsideBgWindowLatch);

	SVArray(_state.BwPalettes, 0x40);
	SVArray(_state.BwShades, 8);
	SVArray(_spriteRam, 512);
	SVArray(_state.LcdTftConfig, 8);

	SV(_state.SpriteTableAddress);
	SV(_state.FirstSpriteIndex);
	SV(_state.SpriteCount);
	SV(_state.SpriteCountLatch);
	SV(_state.SpritesEnabled);
	SV(_state.SpritesEnabledLatch);
	SV(_state.Mode);
	SV(_state.NextMode);
	
	SV(_state.BgColor);
	SV(_state.IrqScanline);

	SV(_state.Icons.Sleep);
	SV(_state.Icons.Horizontal);
	SV(_state.Icons.Vertical);
	SV(_state.Icons.Aux1);
	SV(_state.Icons.Aux2);
	SV(_state.Icons.Aux3);
	SV(_state.Icons.Value);

	SV(_state.Control);
	SV(_state.ScreenAddress);

	SV(_state.LcdEnabled);
	SV(_state.HighContrast);
	SV(_state.SleepEnabled);

	SV(_state.ShowVolumeIconFrame);
	SV(_state.BackPorchScanline);
}
