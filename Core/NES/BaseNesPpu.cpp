#include "pch.h"
#include "NES/BaseNesPpu.h"
#include "NES/NesTypes.h"
#include "NES/NesConstants.h"
#include "NES/NesConsole.h"
#include "Shared/Emulator.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/SettingTypes.h"

void BaseNesPpu::GetState(NesPpuState& state)
{
	state.Control = _control;
	state.Mask = _mask;
	state.StatusFlags = _statusFlags;

	state.VideoRamAddr = _videoRamAddr;
	state.TmpVideoRamAddr = _tmpVideoRamAddr;
	state.SpriteRamAddr = _spriteRamAddr;
	state.ScrollX = _xScroll;
	state.WriteToggle = _writeToggle;

	state.Cycle = _cycle;
	state.Scanline = _scanline;
	state.FrameCount = _frameCount;
	state.NmiScanline = _nmiScanline;
	state.ScanlineCount = _vblankEnd + 2;
	state.SafeOamScanline = _region == ConsoleRegion::Ntsc ? _nmiScanline + 19 : _palSpriteEvalScanline;
	state.BusAddress = _ppuBusAddress;
	state.MemoryReadBuffer = _memoryReadBuffer;
}

void BaseNesPpu::SetState(NesPpuState& state)
{
	_control = state.Control;
	_mask = state.Mask;
	_statusFlags = state.StatusFlags;

	_videoRamAddr = state.VideoRamAddr;
	_tmpVideoRamAddr = state.TmpVideoRamAddr;
	_xScroll = state.ScrollX;
	_writeToggle = state.WriteToggle;
	_spriteRamAddr = state.SpriteRamAddr;

	_cycle = state.Cycle;
	_scanline = state.Scanline;
	_frameCount = state.FrameCount;
	_ppuBusAddress = state.BusAddress;
	_memoryReadBuffer = state.MemoryReadBuffer;

	//Update internal flags based on new state
	if(_renderingEnabled != (_mask.BackgroundEnabled | _mask.SpritesEnabled)) {
		_needStateUpdate = true;
	}
	UpdateMinimumDrawCycles();
	UpdateGrayscaleAndIntensifyBits();
}

bool BaseNesPpu::IsRenderingEnabled()
{
	return _renderingEnabled;
}

uint16_t BaseNesPpu::GetCurrentBgColor()
{
	uint16_t color;
	if((IsRenderingEnabled() && _scanline < 240) || (_ppuBusAddress & 0x3F00) != 0x3F00) {
		color = _paletteRam[0];
	} else {
		color = _paletteRam[_ppuBusAddress & 0x1F];
	}
	return (color & _paletteRamMask) | _intensifyColorBits;
}

uint8_t BaseNesPpu::ReadPaletteRam(uint16_t addr)
{
	addr &= 0x1F;
	if(addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C) {
		addr &= ~0x10;
	}
	return _paletteRam[addr];
}

void BaseNesPpu::WritePaletteRam(uint16_t addr, uint8_t value)
{
	addr &= 0x1F;
	value &= 0x3F;
	if(addr == 0x00 || addr == 0x10) {
		_paletteRam[0x00] = value;
		_paletteRam[0x10] = value;
		_emu->AddDebugEvent<CpuType::Nes>(DebugEventType::BgColorChange);
	} else if(addr == 0x04 || addr == 0x14) {
		_paletteRam[0x04] = value;
		_paletteRam[0x14] = value;
	} else if(addr == 0x08 || addr == 0x18) {
		_paletteRam[0x08] = value;
		_paletteRam[0x18] = value;
	} else if(addr == 0x0C || addr == 0x1C) {
		_paletteRam[0x0C] = value;
		_paletteRam[0x1C] = value;
	} else {
		_paletteRam[addr] = value;
	}
}

void BaseNesPpu::DebugSendFrame()
{
	int offset = std::max(0, (int)(_cycle + _scanline * NesConstants::ScreenWidth));
	int pixelsToClear = NesConstants::ScreenPixelCount - offset;
	if(pixelsToClear > 0) {
		memset(_currentOutputBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
	}

	RenderedFrame frame(_currentOutputBuffer, NesConstants::ScreenWidth, NesConstants::ScreenHeight, 1.0, _frameCount);
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

/* Applies the effect of grayscale/intensify bits to the output buffer (batched) */
void BaseNesPpu::UpdateGrayscaleAndIntensifyBits()
{
	if(_scanline < 0 || _scanline > _nmiScanline) {
		UpdateColorBitMasks();
		return;
	}

	int pixelNumber;
	if(_scanline >= 240) {
		pixelNumber = 61439;
	} else if(_cycle < 3) {
		pixelNumber = (_scanline << 8) - 1;
	} else if(_cycle <= 258) {
		pixelNumber = (_scanline << 8) + _cycle - 3;
	} else {
		pixelNumber = (_scanline << 8) + 255;
	}

	if(_paletteRamMask == 0x3F && _intensifyColorBits == 0) {
		//Nothing to do (most common case)
		UpdateColorBitMasks();
		_lastUpdatedPixel = pixelNumber;
		return;
	}

	if(_lastUpdatedPixel < pixelNumber) {
		uint16_t* out = _currentOutputBuffer + _lastUpdatedPixel + 1;
		while(_lastUpdatedPixel < pixelNumber) {
			*out = (*out & _paletteRamMask) | _intensifyColorBits;
			out++;
			_lastUpdatedPixel++;
		}
	}

	UpdateColorBitMasks();
}

void BaseNesPpu::UpdateColorBitMasks()
{
	//"Bit 0 controls a greyscale mode, which causes the palette to use only the colors from the grey column: $00, $10, $20, $30. This is implemented as a bitwise AND with $30 on any value read from PPU $3F00-$3FFF"
	_paletteRamMask = _mask.Grayscale ? 0x30 : 0x3F;
	_intensifyColorBits = (_mask.IntensifyRed ? 0x40 : 0x00) | (_mask.IntensifyGreen ? 0x80 : 0x00) | (_mask.IntensifyBlue ? 0x100 : 0x00);
}

void BaseNesPpu::UpdateMinimumDrawCycles()
{
	_minimumDrawBgCycle = _mask.BackgroundEnabled ? ((_mask.BackgroundMask || _console->GetNesConfig().ForceBackgroundFirstColumn) ? 0 : 8) : 300;
	_minimumDrawSpriteCycle = _mask.SpritesEnabled ? ((_mask.SpriteMask || _console->GetNesConfig().ForceSpritesFirstColumn) ? 0 : 8) : 300;
	_minimumDrawSpriteStandardCycle = _mask.SpritesEnabled ? (_mask.SpriteMask ? 0 : 8) : 300;

	_emulatorBgEnabled = _console->GetNesConfig().BackgroundEnabled;
	_emulatorSpritesEnabled = _console->GetNesConfig().SpritesEnabled;
}
