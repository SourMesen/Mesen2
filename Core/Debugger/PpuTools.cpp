#include "stdafx.h"
#include "Debugger/PpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugBreakHelper.h"
#include "Shared/SettingTypes.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "SNES/SnesPpu.h"
#include "Gameboy/GbTypes.h"

PpuTools::PpuTools(Debugger* debugger, Emulator *emu)
{
	_emu = emu;
	_debugger = debugger;
}

uint8_t PpuTools::GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, const uint8_t bpp, const uint32_t pixelStart, const uint8_t shift, const int secondByteOffset)
{
	uint8_t color;
	if(bpp == 2) {
		color = (((ram[(pixelStart + 0) & ramMask] >> shift) & 0x01) << 0);
		color |= (((ram[(pixelStart + secondByteOffset) & ramMask] >> shift) & 0x01) << 1);
	} else if(bpp == 4) {
		color = (((ram[(pixelStart + 0) & ramMask] >> shift) & 0x01) << 0);
		color |= (((ram[(pixelStart + 1) & ramMask] >> shift) & 0x01) << 1);
		color |= (((ram[(pixelStart + 16) & ramMask] >> shift) & 0x01) << 2);
		color |= (((ram[(pixelStart + 17) & ramMask] >> shift) & 0x01) << 3);
	} else if(bpp == 8) {
		color = (((ram[(pixelStart + 0) & ramMask] >> shift) & 0x01) << 0);
		color |= (((ram[(pixelStart + 1) & ramMask] >> shift) & 0x01) << 1);
		color |= (((ram[(pixelStart + 16) & ramMask] >> shift) & 0x01) << 2);
		color |= (((ram[(pixelStart + 17) & ramMask] >> shift) & 0x01) << 3);
		color |= (((ram[(pixelStart + 32) & ramMask] >> shift) & 0x01) << 4);
		color |= (((ram[(pixelStart + 33) & ramMask] >> shift) & 0x01) << 5);
		color |= (((ram[(pixelStart + 48) & ramMask] >> shift) & 0x01) << 6);
		color |= (((ram[(pixelStart + 49) & ramMask] >> shift) & 0x01) << 7);
	} else {
		throw std::runtime_error("unsupported bpp");
	}
	return color;
}

void PpuTools::BlendColors(uint8_t output[4], uint8_t input[4])
{
	int alpha = input[3] + 1;
	uint8_t invertedAlpha = 256 - input[3];
	output[0] = (uint8_t)((alpha * input[0] + invertedAlpha * output[0]) >> 8);
	output[1] = (uint8_t)((alpha * input[1] + invertedAlpha * output[1]) >> 8);
	output[2] = (uint8_t)((alpha * input[2] + invertedAlpha * output[2]) >> 8);
	output[3] = 0xFF;
}

uint32_t PpuTools::GetRgbPixelColor(uint32_t* colors, uint8_t colorIndex, uint8_t palette, uint8_t bpp, bool directColorMode, uint16_t basePaletteOffset)
{
	if(bpp == 8 && directColorMode) {
		uint32_t paletteColor = (
			((((colorIndex & 0x07) << 1) | (palette & 0x01)) << 1) |
			(((colorIndex & 0x38) | ((palette & 0x02) << 1)) << 4) |
			(((colorIndex & 0xC0) | ((palette & 0x04) << 3)) << 7)
		);
		return SnesDefaultVideoFilter::ToArgb(paletteColor);
	} else {
		return colors[basePaletteOffset + (palette * (1 << bpp) + colorIndex)];
	}
}

void PpuTools::GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint32_t *colors, uint32_t *outBuffer)
{
	uint32_t ramMask = srcSize - 1;

	uint8_t* ram = source;
	uint8_t bpp;

	int rowOffset = 2;
	int secondByteOffset = 1;
	bool directColor = false;
	switch(options.Format) {
		case TileFormat::Bpp2: bpp = 2; break;
		case TileFormat::Bpp4: bpp = 4; break;
		case TileFormat::DirectColor: directColor = true; bpp = 8; break;
		case TileFormat::Mode7: bpp = 16; break;
		case TileFormat::Mode7DirectColor: directColor = true; bpp = 16; break;
		case TileFormat::NesBpp2: bpp = 2; rowOffset = 1; secondByteOffset = 8; break;
		default: bpp = 8; break;
	}

	int bytesPerTile = 64 * bpp / 8;
	int tileCount = options.Width * options.Height;

	uint32_t bgColor = 0;
	switch(options.Background) {
		case TileBackground::Default: bgColor = colors[0]; break;
		case TileBackground::PaletteColor: bgColor = SnesDefaultVideoFilter::ToArgb(0); break;
		case TileBackground::Black: bgColor = 0xFF000000; break;
		case TileBackground::White: bgColor = 0xFFFFFFFF; break;
		case TileBackground::Magenta: bgColor = 0xFFFF00FF; break;
	}

	uint32_t outputSize = tileCount * 8*8;
	for(uint32_t i = 0; i < outputSize; i++) {
		outBuffer[i] = bgColor;
	}

	int rowCount = (int)std::ceil((double)tileCount / options.Width);

	for(int row = 0; row < rowCount; row++) {
		uint32_t baseOffset = row * bytesPerTile * options.Width;
		if(baseOffset >= srcSize) {
			break;
		}

		for(int column = 0; column < options.Width; column++) {
			uint32_t addr = baseOffset + options.StartAddress + bytesPerTile * column;

			int baseOutputOffset;
			if(options.Layout == TileLayout::SingleLine8x16) {
				int displayColumn = column / 2 + ((row & 0x01) ? options.Width/2 : 0);
				int displayRow = (row & ~0x01) + ((column & 0x01) ? 1 : 0);
				baseOutputOffset = displayRow * options.Width * 64 + displayColumn * 8;
			} else if(options.Layout == TileLayout::SingleLine16x16) {
				int displayColumn = (column / 2) + (column & 0x01) + ((row & 0x01) ? options.Width/2 : 0) + ((column & 0x02) ? -1 : 0);
				int displayRow = (row & ~0x01) + ((column & 0x02) ? 1 : 0);
				baseOutputOffset = displayRow * options.Width * 64 + displayColumn * 8;
			} else {
				baseOutputOffset = row * options.Width * 64 + column * 8;
			}

			if(options.Format == TileFormat::Mode7 || options.Format == TileFormat::Mode7DirectColor) {
				for(int y = 0; y < 8; y++) {
					uint32_t pixelStart = addr + y * 16;

					for(int x = 0; x < 8; x++) {
						uint8_t color = ram[(pixelStart + x * 2 + 1) & ramMask];

						if(color != 0 || options.Background == TileBackground::PaletteColor) {
							int pos = baseOutputOffset + (y * options.Width * 8) + x;
							if(pos < outputSize) {
								uint32_t rgbColor;
								if(directColor) {
									rgbColor = SnesDefaultVideoFilter::ToArgb(((color & 0x07) << 2) | ((color & 0x38) << 4) | ((color & 0xC0) << 7));
								} else {
									rgbColor = GetRgbPixelColor(colors, color, 0, 8, false, 0);
								}
								outBuffer[pos] = rgbColor;
							}
						}
					}
				}
			} else {
				for(int y = 0; y < 8; y++) {
					uint32_t pixelStart = addr + y * rowOffset;
					for(int x = 0; x < 8; x++) {
						uint8_t color = GetTilePixelColor(ram, ramMask, bpp, pixelStart, 7 - x, secondByteOffset);
						if(color != 0 || options.Background == TileBackground::PaletteColor) {
							int pos = baseOutputOffset + (y * options.Width * 8) + x;
							if(pos < outputSize) {
								outBuffer[pos] = GetRgbPixelColor(colors, color, options.Palette, bpp, directColor, 0);
							}
						}
					}
				}
			}
		}
	}
}

void PpuTools::SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle)
{
	DebugBreakHelper helper(_debugger);

	ViewerRefreshConfig cfg;
	cfg.Scanline = scanline;
	cfg.Cycle = cycle;
	_updateTimings[viewerId] = cfg;
}

void PpuTools::RemoveViewer(uint32_t viewerId)
{
	DebugBreakHelper helper(_debugger);
	_updateTimings.erase(viewerId);
}
