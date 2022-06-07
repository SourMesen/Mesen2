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

uint8_t PpuTools::GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, uint32_t rowStart, uint8_t pixelIndex, const TileFormat format)
{
	uint8_t shift = (7 - pixelIndex);
	uint8_t color;
	switch(format) {
		case TileFormat::PceSpriteBpp4: {
			shift = 15 - pixelIndex;
			if(shift >= 8) {
				shift -= 8;
				rowStart++;
			}
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 32) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 64) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 96) & ramMask] >> shift) & 0x01) << 3);
			return color;
		}

		case TileFormat::Bpp2:
			color = (((ram[rowStart & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			return color;
		
		case TileFormat::NesBpp2:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 8) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::Bpp4:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 16) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 17) & ramMask] >> shift) & 0x01) << 3);
			return color;

		case TileFormat::Bpp8:
		case TileFormat::DirectColor:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 16) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 17) & ramMask] >> shift) & 0x01) << 3);
			color |= (((ram[(rowStart + 32) & ramMask] >> shift) & 0x01) << 4);
			color |= (((ram[(rowStart + 33) & ramMask] >> shift) & 0x01) << 5);
			color |= (((ram[(rowStart + 48) & ramMask] >> shift) & 0x01) << 6);
			color |= (((ram[(rowStart + 49) & ramMask] >> shift) & 0x01) << 7);
			return color;

		case TileFormat::Mode7:
		case TileFormat::Mode7DirectColor:
			return ram[(rowStart + pixelIndex * 2 + 1) & ramMask];
		
		case TileFormat::Mode7ExtBg:
			return ram[(rowStart + pixelIndex * 2 + 1) & ramMask] & 0x7F;

		default:
			throw std::runtime_error("unsupported format");
	}
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

uint32_t PpuTools::GetRgbPixelColor(TileFormat format, const uint32_t* colors, uint8_t colorIndex, uint8_t palette)
{
	switch(format) {
		case TileFormat::DirectColor:
			return SnesDefaultVideoFilter::ToArgb(
				((((colorIndex & 0x07) << 1) | (palette & 0x01)) << 1) |
				(((colorIndex & 0x38) | ((palette & 0x02) << 1)) << 4) |
				(((colorIndex & 0xC0) | ((palette & 0x04) << 3)) << 7)
			);
		
		case TileFormat::NesBpp2:
		case TileFormat::Bpp2:
			return colors[palette * 4 + colorIndex];

		case TileFormat::Bpp4:
		case TileFormat::PceSpriteBpp4:
			return colors[palette * 16 + colorIndex];

		case TileFormat::Bpp8:
		case TileFormat::Mode7:
		case TileFormat::Mode7ExtBg:
			return colors[colorIndex];

		case TileFormat::Mode7DirectColor:
			return SnesDefaultVideoFilter::ToArgb(((colorIndex & 0x07) << 2) | ((colorIndex & 0x38) << 4) | ((colorIndex & 0xC0) << 7));

		default:
			throw std::runtime_error("unsupported format");
	}
}

void PpuTools::GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, const uint32_t *colors, uint32_t *outBuffer)
{
	constexpr uint32_t grayscaleColorsBpp2[4] = { 0xFF000000, 0xFF666666, 0xFFBBBBBB, 0xFFFFFFFF };
	constexpr uint32_t grayscaleColorsBpp4[16] = {
		0xFF000000, 0xFF111111, 0xFF222222, 0xFF333333, 0xFF444444, 0xFF555555, 0xFF666666, 0xFF777777,
		0xFF888888, 0xFF999999, 0xFFAAAAAA, 0xFFBBBBBB, 0xFFCCCCCC, 0xFFDDDDDD, 0xFFEEEEEE, 0xFFFFFFFF
	};

	uint32_t ramMask = srcSize - 1;
	uint8_t* ram = source;
	uint8_t bpp;

	int rowOffset = 2;
	
	int tileWidth = 8;
	int tileHeight = 8;
	switch(options.Format) {
		case TileFormat::Bpp2: bpp = 2; break;
		case TileFormat::Bpp4: bpp = 4; break;
		case TileFormat::DirectColor: bpp = 8; break;
		
		case TileFormat::Mode7:
		case TileFormat::Mode7DirectColor:
		case TileFormat::Mode7ExtBg:
			bpp = 16;
			rowOffset = 16;
			break;

		case TileFormat::NesBpp2: bpp = 2; rowOffset = 1; break;
		case TileFormat::PceSpriteBpp4: bpp = 4; rowOffset = 2; tileWidth = 16; tileHeight = 16; options.Width /= 2; options.Height /= 2; break;
		default: bpp = 8; break;
	}

	int bytesPerTile = tileHeight*tileWidth * bpp / 8;
	int tileCount = options.Width * options.Height;

	uint8_t colorMask = 0xFF;
	if(options.UseGrayscalePalette) {
		options.Palette = 0;
		colors = bpp == 2 ? grayscaleColorsBpp2 : grayscaleColorsBpp4;
		colorMask = bpp == 2 ? 0x03 : 0x0F;
	}

	uint32_t bgColor = 0;
	switch(options.Background) {
		case TileBackground::Default: bgColor = colors[0]; break;
		case TileBackground::PaletteColor: bgColor = colors[options.Palette * (1 << bpp)]; break;
		case TileBackground::Black: bgColor = 0xFF000000; break;
		case TileBackground::White: bgColor = 0xFFFFFFFF; break;
		case TileBackground::Magenta: bgColor = 0xFFFF00FF; break;
	}

	uint32_t outputSize = tileCount * tileWidth * tileHeight;
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
				baseOutputOffset = displayRow * options.Width * tileWidth * tileHeight + displayColumn * tileWidth;
			} else if(options.Layout == TileLayout::SingleLine16x16) {
				int displayColumn = (column / 2) + (column & 0x01) + ((row & 0x01) ? options.Width/2 : 0) + ((column & 0x02) ? -1 : 0);
				int displayRow = (row & ~0x01) + ((column & 0x02) ? 1 : 0);
				baseOutputOffset = displayRow * options.Width * tileWidth * tileHeight + displayColumn * tileWidth;
			} else {
				baseOutputOffset = row * options.Width * tileWidth * tileHeight + column * tileWidth;
			}

			for(int y = 0; y < tileHeight; y++) {
				uint32_t pixelStart = addr + y * rowOffset;
				for(int x = 0; x < tileWidth; x++) {
					uint8_t color = GetTilePixelColor(ram, ramMask, pixelStart, x, options.Format);
					if(color != 0 || options.Background == TileBackground::PaletteColor) {
						uint32_t pos = baseOutputOffset + (y * options.Width * tileWidth) + x;
						if(pos < outputSize) {
							outBuffer[pos] = GetRgbPixelColor(options.Format, colors, color & colorMask, options.Palette);
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

void PpuTools::UpdateViewers(uint16_t scanline, uint16_t cycle)
{
	for(auto updateTiming : _updateTimings) {
		ViewerRefreshConfig cfg = updateTiming.second;
		if(cfg.Cycle == cycle && cfg.Scanline == scanline) {
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ViewerRefresh, (void*)(uint64_t)updateTiming.first);
		}
	}
}
