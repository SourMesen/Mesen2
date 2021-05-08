#pragma once
#include "stdafx.h"
#include "NES/NesPpu.h"

class DefaultNesPpu final : public NesPpu<DefaultNesPpu>
{
public:
	DefaultNesPpu(NesConsole* console) : NesPpu(console)
	{
	}

	__forceinline void StoreSpriteAbsoluteAddress()
	{
	}

	__forceinline void StoreTileAbsoluteAddress()
	{
	}

	__forceinline void ProcessScanline()
	{
		ProcessScanlineImpl();
	}

	__forceinline bool IsEmulatorBgEnabled()
	{
		return true;
	}

	__forceinline bool IsEmulatorSpritesEnabled()
	{
		return true;
	}

	__forceinline void DrawPixel()
	{
		//This is called 3.7 million times per second - needs to be as fast as possible.
		if(IsRenderingEnabled() || ((_state.VideoRamAddr & 0x3F00) != 0x3F00)) {
			uint32_t color = GetPixelColor();
			_currentOutputBuffer[(_scanline << 8) + _cycle - 1] = _paletteRAM[color & 0x03 ? color : 0];
		} else {
			//"If the current VRAM address points in the range $3F00-$3FFF during forced blanking, the color indicated by this palette location will be shown on screen instead of the backdrop color."
			_currentOutputBuffer[(_scanline << 8) + _cycle - 1] = _paletteRAM[_state.VideoRamAddr & 0x1F];
		}
	}
};
