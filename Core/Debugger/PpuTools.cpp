#include "pch.h"
#include "Debugger/PpuTools.h"
#include "Debugger/CdlManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugBreakHelper.h"
#include "Shared/SettingTypes.h"

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
		
		case TileFormat::SmsBpp4: InternalGetTileView<TileFormat::SmsBpp4>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::SmsSgBpp1: InternalGetTileView<TileFormat::SmsSgBpp1>(options, source, srcSize, colors, outBuffer); break;
		
		case TileFormat::GbaBpp4: InternalGetTileView<TileFormat::GbaBpp4>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::GbaBpp8: InternalGetTileView<TileFormat::GbaBpp8>(options, source, srcSize, colors, outBuffer); break;
		case TileFormat::WsBpp4Packed: InternalGetTileView<TileFormat::WsBpp4Packed>(options, source, srcSize, colors, outBuffer); break;
	}
}

uint32_t PpuTools::GetBackgroundColor(TileBackground bgColor, const uint32_t* colors, uint8_t paletteIndex, uint8_t bpp)
{
	switch(bgColor) {
		default:
		case TileBackground::Default: return colors[0];
		case TileBackground::PaletteColor: return colors[paletteIndex * (1 << bpp)];
		case TileBackground::Black: return 0xFF000000;
		case TileBackground::White: return 0xFFFFFFFF;
		case TileBackground::Magenta: return 0xFFFF00FF;
		case TileBackground::Transparent: return 0;
	}
}

uint32_t PpuTools::GetSpriteBackgroundColor(SpriteBackground bgColor, const uint32_t* colors, bool useDarkerColor)
{
	switch(bgColor) {
		default:
		case SpriteBackground::Gray: return useDarkerColor ? 0xFF333333 : 0xFF666666;
		case SpriteBackground::Background: return useDarkerColor ? (((colors[0] >> 1) & 0x7F7F7F) | 0xFF000000) : colors[0];
		case SpriteBackground::Black: return useDarkerColor ? 0xFF000000 : 0xFF202020;
		case SpriteBackground::White: return useDarkerColor ? 0xFFEEEEEE : 0xFFFFFFFF;
		case SpriteBackground::Magenta: return useDarkerColor ? 0xFFCC00CC : 0xFFFF00FF;
		case SpriteBackground::Transparent: return 0;
	}
}

template<TileFormat format>
void PpuTools::InternalGetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, const uint32_t *colors, uint32_t *outBuffer)
{
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
		
		case TileFormat::SmsBpp4: bpp = 4; rowOffset = 4; break;
		case TileFormat::SmsSgBpp1: bpp = 1; rowOffset = 1; break;

		case TileFormat::GbaBpp4: bpp = 4; rowOffset = 4; break;
		case TileFormat::GbaBpp8: bpp = 8; rowOffset = 8; break;
		
		case TileFormat::WsBpp4Packed: bpp = 4; rowOffset = 4; break;

		default: bpp = 8; break;
	}

	int bytesPerTile = tileHeight*tileWidth * bpp / 8;
	int tileCount = options.Width * options.Height;

	uint8_t colorMask = 0xFF;
	if(options.UseGrayscalePalette) {
		options.Palette = 0;
		switch(bpp) {
			case 1: colors = _grayscaleColorsBpp1; colorMask = 0x01; break;
			case 2: colors = _grayscaleColorsBpp2; colorMask = 0x03; break;
			default: colors = _grayscaleColorsBpp4; colorMask = 0x0F; break;
		}
	}

	uint32_t bgColor = GetBackgroundColor(options.Background, colors, options.Palette, bpp);

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
			uint32_t addr = baseOffset + bytesPerTile * column;

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
					uint8_t color = GetTilePixelColor<format>(ram, 0xFFFFFFFF, pixelStart, x);
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

int32_t PpuTools::GetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y)
{
	int32_t color = 0;
	GetSetTilePixel(tileAddress, format, x, y, color, true);
	return color;
}

void PpuTools::SetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t color)
{
	GetSetTilePixel(tileAddress, format, x, y, color, false);
}

void PpuTools::GetSetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t& color, bool forGet)
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
		case TileFormat::SmsBpp4: rowOffset = 4; break;
		case TileFormat::SmsSgBpp1: rowOffset = 1; break;
		case TileFormat::GbaBpp4: rowOffset = 4; break;
		case TileFormat::GbaBpp8: rowOffset = 8; break;
		case TileFormat::WsBpp4Packed: rowOffset = 4; break;
	}

	uint8_t* ram = (uint8_t*)memInfo.Memory;
	int rowStart = tileAddress.Address + (y * rowOffset);

	uint8_t shift = (7 - x);

	auto setBit = [&](uint32_t addr, uint8_t pixelNumber, uint8_t bitNumber) {
		if(addr >= memInfo.Size) {
			return;
		}

		if(forGet) {
			uint8_t bitValue = ((ram[addr] >> pixelNumber) & 0x01);
			color |= bitValue << bitNumber;
		} else {
			uint8_t bitValue = (color >> bitNumber) & 0x01;
			ram[addr] &= ~(1 << pixelNumber);
			ram[addr] |= (bitValue & 0x01) << pixelNumber;
		}
	};

	switch(format) {
		case TileFormat::Bpp2:
			setBit(rowStart, shift, 0);
			setBit(rowStart + 1, shift, 1);
			break;

		case TileFormat::NesBpp2:
			setBit(rowStart, shift, 0);
			setBit(rowStart + 8, shift, 1);
			break;

		case TileFormat::Bpp4:
			setBit(rowStart, shift, 0);
			setBit(rowStart + 1, shift, 1);
			setBit(rowStart + 16, shift, 2);
			setBit(rowStart + 17, shift, 3);
			break;

		case TileFormat::Bpp8:
		case TileFormat::DirectColor:
			setBit(rowStart, shift, 0);
			setBit(rowStart + 1, shift, 1);
			setBit(rowStart + 16, shift, 2);
			setBit(rowStart + 17, shift, 3);
			setBit(rowStart + 32, shift, 4);
			setBit(rowStart + 33, shift, 5);
			setBit(rowStart + 48, shift, 6);
			setBit(rowStart + 49, shift, 7);
			break;

		case TileFormat::Mode7:
		case TileFormat::Mode7DirectColor:
		case TileFormat::Mode7ExtBg: {
			uint32_t addr = (rowStart + x * 2 + 1);
			if(addr < memInfo.Size) {
				if(forGet) {
					color = ram[addr];
				} else {
					ram[addr] = color;
				}
			}
			break;
		}

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
					setBit(rowStart, shift, 0);
					setBit(rowStart + 32, shift, 1);
					setBit(rowStart + 64, shift, 2);
					setBit(rowStart + 96, shift, 3);
					break;

				case TileFormat::PceSpriteBpp2Sp01:
					setBit(rowStart, shift, 0);
					setBit(rowStart + 32, shift, 1);
					break;

				case TileFormat::PceSpriteBpp2Sp23:
					setBit(rowStart + 64, shift, 2);
					setBit(rowStart + 96, shift, 3);
					break;
			}
			break;
		}

		case TileFormat::PceBackgroundBpp2Cg0:
			setBit(rowStart, shift, 0);
			setBit(rowStart + 1, shift, 1);
			break;

		case TileFormat::PceBackgroundBpp2Cg1:
			setBit(rowStart + 16, shift, 2);
			setBit(rowStart + 17, shift, 3);
			break;

		case TileFormat::SmsBpp4:
			setBit(rowStart, shift, 0);
			setBit(rowStart + 1, shift, 1);
			setBit(rowStart + 2, shift, 2);
			setBit(rowStart + 3, shift, 3);
			break;

		case TileFormat::SmsSgBpp1:
			setBit(rowStart, shift, 0);
			break;

		case TileFormat::GbaBpp4: {
			uint8_t pixelOffset = (7 - shift);
			int32_t addr = (rowStart + (pixelOffset >> 1));
			int offset = pixelOffset & 0x01 ? 4 : 0;
			for(int i = 0; i < 4; i++) {
				setBit(addr, i+offset, i);
			}
			break;
		}

		case TileFormat::GbaBpp8: {
			uint8_t pixelOffset = (7 - shift);
			int32_t addr = rowStart + pixelOffset;
			for(int i = 0; i < 8; i++) {
				setBit(addr, i, i);
			}
			break;
		}

		case TileFormat::WsBpp4Packed: {
			uint8_t pixelOffset = (7 - shift);
			int32_t addr = (rowStart + (pixelOffset >> 1));
			int offset = pixelOffset & 0x01 ? 0 : 4;
			for(int i = 0; i < 4; i++) {
				setBit(addr, i+offset, i);
			}
			break;
		}

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
