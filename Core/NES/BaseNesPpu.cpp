#include "stdafx.h"
#include "BaseNesPpu.h"
#include "NES/NesTypes.h"
#include "Shared/SettingTypes.h"

void BaseNesPpu::GetState(NesPpuState& state)
{
	//TODO
	//state.ControlFlags = _flags;
	state.StatusFlags = _statusFlags;
	//state.State = _state;
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
	//TODO
	//_flags = state.ControlFlags;
	_statusFlags = state.StatusFlags;
	//_state = state.State;
	_cycle = state.Cycle;
	_scanline = state.Scanline;
	_frameCount = state.FrameCount;

	//TODO
	//UpdateMinimumDrawCycles();

	_paletteRamMask = _grayscale ? 0x30 : 0x3F;
	if(_region == ConsoleRegion::Ntsc) {
		_intensifyColorBits = (_intensifyGreen ? 0x40 : 0x00) | (_intensifyRed ? 0x80 : 0x00) | (_intensifyBlue ? 0x100 : 0x00);
	} else if(_region == ConsoleRegion::Pal || _region == ConsoleRegion::Dendy) {
		//"Note that on the Dendy and PAL NES, the green and red bits swap meaning."
		_intensifyColorBits = (_intensifyRed ? 0x40 : 0x00) | (_intensifyGreen ? 0x80 : 0x00) | (_intensifyBlue ? 0x100 : 0x00);
	}
}

bool BaseNesPpu::IsRenderingEnabled()
{
	return _renderingEnabled;
}

uint16_t BaseNesPpu::GetCurrentBgColor()
{
	uint16_t color;
	if(IsRenderingEnabled() || (_videoRamAddr & 0x3F00) != 0x3F00) {
		color = _paletteRAM[0];
	} else {
		color = _paletteRAM[_videoRamAddr & 0x1F];
	}
	return (color & _paletteRamMask) | _intensifyColorBits;
}