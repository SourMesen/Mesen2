#include "pch.h"
#include "Debugger/PpuTools.h"
#include "Debugger/CdlManager.h"
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

void PpuTools::BlendColors(uint8_t output[4], uint8_t input[4])
{
	int alpha = input[3] + 1;
	uint8_t invertedAlpha = 256 - input[3];
	output[0] = (uint8_t)((alpha * input[0] + invertedAlpha * output[0]) >> 8);
	output[1] = (uint8_t)((alpha * input[1] + invertedAlpha * output[1]) >> 8);
	output[2] = (uint8_t)((alpha * input[2] + invertedAlpha * output[2]) >> 8);
	output[3] = 0xFF;
}

void PpuTools::GetTileView(GetTileViewOptions options, uint8_t* source, uint32_t srcSize, const uint32_t* colors, uint32_t* outBuffer)
{
	switch(options.Format) {
		case TileFormat::Bpp2: InternalGetTileView<TileFormat::Bpp2>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::Bpp4: InternalGetTileView<TileFormat::Bpp4>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::Bpp8: InternalGetTileView<TileFormat::Bpp8>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::DirectColor: InternalGetTileView<TileFormat::DirectColor>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::Mode7: InternalGetTileView<TileFormat::Mode7>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::Mode7DirectColor: InternalGetTileView<TileFormat::Mode7DirectColor>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::Mode7ExtBg: InternalGetTileView<TileFormat::Mode7ExtBg>(options, source, srcSize, colors, outBuffer); break;
		
		case TileFormat::NesBpp2: InternalGetTileView<TileFormat::NesBpp2>(options, source, srcSize, colors, outBuffer); break;
		
		case TileFormat::PceSpriteBpp4: InternalGetTileView<TileFormat::PceSpriteBpp4>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::PceBackgroundBpp2Cg0: InternalGetTileView<TileFormat::PceBackgroundBpp2Cg0>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::PceBackgroundBpp2Cg1: InternalGetTileView<TileFormat::PceBackgroundBpp2Cg1>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::PceSpriteBpp2Sp01: InternalGetTileView<TileFormat::PceSpriteBpp2Sp01>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::PceSpriteBpp2Sp23: InternalGetTileView<TileFormat::PceSpriteBpp2Sp23>(options, source, srcSize, colors, outBuffer); break;
	}
}

template<TileFormat format>
void PpuTools::InternalGetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, const uint32_t *colors, uint32_t *outBuffer)
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
		case TileFormat::PceSpriteBpp2Sp01: bpp = 4; rowOffset = 2; tileWidth = 16; tileHeight = 16; options.Width /= 2; options.Height /= 2; break;
		case TileFormat::PceSpriteBpp2Sp23: bpp = 4; rowOffset = 2; tileWidth = 16; tileHeight = 16; options.Width /= 2; options.Height /= 2; break;
		
		//2BPP, but use BPP=4 because tiles are arranged in the regular 4BPP layout
		case TileFormat::PceBackgroundBpp2Cg0: bpp = 4; break;
		case TileFormat::PceBackgroundBpp2Cg1: bpp = 4; break;

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
		case TileBackground::Transparent: bgColor = 0; break;
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

			if(IsTileHidden(options.MemType, addr, options)) {
				continue;
			}

			for(int y = 0; y < tileHeight; y++) {
				uint32_t pixelStart = addr + y * rowOffset;
				for(int x = 0; x < tileWidth; x++) {
					uint8_t color = GetTilePixelColor<format>(ram, ramMask, pixelStart, x);
					if(color != 0 || options.Background == TileBackground::PaletteColor) {
						uint32_t pos = baseOutputOffset + (y * options.Width * tileWidth) + x;
						if(pos < outputSize) {
							outBuffer[pos] = GetRgbPixelColor<format>(colors, color & colorMask, options.Palette);
						}
					}
				}
			}
		}
	}
}

bool PpuTools::IsTileHidden(MemoryType memType, uint32_t addr, GetTileViewOptions& options)
{
	if(options.Filter == TileFilter::None) {
		return false;
	}

	int16_t cdlFlags = _debugger->GetCdlManager()->GetCdlFlags(memType, addr);
	return (
		(cdlFlags == 0 && options.Filter == TileFilter::HideUnused) ||
		(cdlFlags > 0 && options.Filter == TileFilter::HideUsed)
	);
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

void PpuTools::SetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t color)
{
	ConsoleMemoryInfo memInfo = _emu->GetMemory(tileAddress.Type);
	if(!memInfo.Memory || memInfo.Size == 0) {
		return;
	}

	int rowOffset;
	switch(format) {
		default: rowOffset = 2; break;
		case TileFormat::Mode7:
		case TileFormat::Mode7DirectColor:
		case TileFormat::Mode7ExtBg:
			rowOffset = 16;
			break;

		case TileFormat::NesBpp2: rowOffset = 1; break;
		case TileFormat::PceSpriteBpp4: rowOffset = 2; break;
	}

	uint8_t* ram = (uint8_t*)memInfo.Memory;
	int rowStart = tileAddress.Address + (y * rowOffset);
	int ramMask = (memInfo.Size - 1);

	uint8_t shift = (7 - x);

	auto setBit = [&](uint32_t addr, uint8_t bitNumber, uint8_t bitValue) {
		ram[addr & ramMask] &= ~(1 << bitNumber);
		ram[addr & ramMask] |= (bitValue & 0x01) << bitNumber;
	};

	switch(format) {
		case TileFormat::Bpp2:
			setBit(rowStart, shift, color & 0x01);
			setBit(rowStart + 1, shift, (color & 0x02) >> 1);
			break;

		case TileFormat::NesBpp2:
			setBit(rowStart, shift, color & 0x01);
			setBit(rowStart + 8, shift, (color & 0x02) >> 1);
			break;

		case TileFormat::Bpp4:
			setBit(rowStart, shift, color & 0x01);
			setBit(rowStart + 1, shift, (color & 0x02) >> 1);
			setBit(rowStart + 16, shift, (color & 0x04) >> 2);
			setBit(rowStart + 17, shift, (color & 0x08) >> 3);
			break;

		case TileFormat::Bpp8:
		case TileFormat::DirectColor:
			setBit(rowStart, shift, color & 0x01);
			setBit(rowStart + 1, shift, (color & 0x02) >> 1);
			setBit(rowStart + 16, shift, (color & 0x04) >> 2);
			setBit(rowStart + 17, shift, (color & 0x08) >> 3);
			setBit(rowStart + 32, shift, (color & 0x10) >> 4);
			setBit(rowStart + 33, shift, (color & 0x20) >> 5);
			setBit(rowStart + 48, shift, (color & 0x40) >> 6);
			setBit(rowStart + 49, shift, (color & 0x80) >> 7);
			break;

		case TileFormat::Mode7:
		case TileFormat::Mode7DirectColor:
			ram[(rowStart + x * 2 + 1) & ramMask] = color;
			break;

		case TileFormat::Mode7ExtBg:
			ram[(rowStart + x * 2 + 1) & ramMask] = color;
			break;

		case TileFormat::PceSpriteBpp4:
		case TileFormat::PceSpriteBpp2Sp01:
		case TileFormat::PceSpriteBpp2Sp23:
		{
			shift = 15 - x;
			if(shift >= 8) {
				shift -= 8;
				rowStart++;
			}

			switch(format) {
				case TileFormat::PceSpriteBpp4:
					setBit(rowStart, shift, color & 0x01);
					setBit(rowStart + 32, shift, (color & 0x02) >> 1);
					setBit(rowStart + 64, shift, (color & 0x04) >> 2);
					setBit(rowStart + 96, shift, (color & 0x08) >> 3);
					break;

				case TileFormat::PceSpriteBpp2Sp01:
					setBit(rowStart, shift, color & 0x01);
					setBit(rowStart + 32, shift, (color & 0x02) >> 1);
					break;

				case TileFormat::PceSpriteBpp2Sp23:
					setBit(rowStart + 64, shift, (color & 0x04) >> 2);
					setBit(rowStart + 96, shift, (color & 0x08) >> 3);
					break;
			}
			break;
		}

		case TileFormat::PceBackgroundBpp2Cg0:
			setBit(rowStart, shift, color & 0x01);
			setBit(rowStart + 1, shift, (color & 0x02) >> 1);
			break;

		case TileFormat::PceBackgroundBpp2Cg1:
			setBit(rowStart + 16, shift, (color & 0x04) >> 2);
			setBit(rowStart + 17, shift, (color & 0x08) >> 3);
			break;

		default:
			throw std::runtime_error("unsupported format");
	}
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
