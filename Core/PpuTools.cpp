#include "stdafx.h"
#include "PpuTools.h"
#include "Ppu.h"
#include "DebugTypes.h"
#include "Console.h"
#include "NotificationManager.h"

PpuTools::PpuTools(Console *console, Ppu *ppu)
{
	_console = console;
	_ppu = ppu;
}

uint16_t PpuTools::GetTilePixelColor(const uint8_t bpp, const uint16_t pixelStart, const uint8_t shift)
{
	uint8_t *vram = _ppu->GetVideoRam();
	uint16_t color;
	if(bpp == 2) {
		color = (((vram[pixelStart + 0] >> shift) & 0x01) << 0);
		color |= (((vram[pixelStart + 1] >> shift) & 0x01) << 1);
	} else if(bpp == 4) {
		color = (((vram[pixelStart + 0] >> shift) & 0x01) << 0);
		color |= (((vram[pixelStart + 1] >> shift) & 0x01) << 1);
		color |= (((vram[pixelStart + 16] >> shift) & 0x01) << 2);
		color |= (((vram[pixelStart + 17] >> shift) & 0x01) << 3);
	} else if(bpp == 8) {
		color = (((vram[pixelStart + 0] >> shift) & 0x01) << 0);
		color |= (((vram[pixelStart + 1] >> shift) & 0x01) << 1);
		color |= (((vram[pixelStart + 16] >> shift) & 0x01) << 2);
		color |= (((vram[pixelStart + 17] >> shift) & 0x01) << 3);
		color |= (((vram[pixelStart + 32] >> shift) & 0x01) << 4);
		color |= (((vram[pixelStart + 33] >> shift) & 0x01) << 5);
		color |= (((vram[pixelStart + 48] >> shift) & 0x01) << 6);
		color |= (((vram[pixelStart + 49] >> shift) & 0x01) << 7);
	} else {
		throw std::runtime_error("unsupported bpp");
	}
	return color;
}

uint32_t PpuTools::ToArgb(uint16_t color)
{
	uint8_t b = (color >> 10) << 3;
	uint8_t g = ((color >> 5) & 0x1F) << 3;
	uint8_t r = (color & 0x1F) << 3;

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void PpuTools::BlendColors(uint8_t output[4], uint8_t input[4])
{
	uint8_t alpha = input[3] + 1;
	uint8_t invertedAlpha = 256 - input[3];
	output[0] = (uint8_t)((alpha * input[0] + invertedAlpha * output[0]) >> 8);
	output[1] = (uint8_t)((alpha * input[1] + invertedAlpha * output[1]) >> 8);
	output[2] = (uint8_t)((alpha * input[2] + invertedAlpha * output[2]) >> 8);
	output[3] = 0xFF;
}

void PpuTools::GetTilemap(GetTilemapOptions options, uint32_t* outBuffer)
{
	static constexpr uint8_t layerBpp[8][4] = {
		{ 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 }
	};

	PpuState state = _ppu->GetState();
	options.BgMode = state.BgMode;

	uint16_t basePaletteOffset = 0;
	if(options.BgMode == 0) {
		basePaletteOffset = options.Layer * 64;
	}

	uint8_t *vram = _ppu->GetVideoRam();
	uint8_t *cgram = _ppu->GetCgRam();
	LayerConfig layer = state.Layers[options.Layer];

	uint16_t bgColor = (cgram[1] << 8) | cgram[0];
	for(int i = 0; i < 512 * 512; i++) {
		outBuffer[i] = ToArgb(bgColor);
	}

	uint8_t bpp = layerBpp[options.BgMode][options.Layer];
	if(bpp == 0) {
		return;
	}

	bool largeTileWidth = layer.LargeTiles || options.BgMode == 5 || options.BgMode == 6;
	bool largeTileHeight = layer.LargeTiles;

	for(int row = 0; row < (layer.DoubleHeight ? 64 : 32); row++) {
		uint16_t addrVerticalScrollingOffset = layer.DoubleHeight ? ((row & 0x20) << (layer.DoubleWidth ? 6 : 5)) : 0;
		uint16_t baseOffset = (layer.TilemapAddress >> 1) + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

		for(int column = 0; column < (layer.DoubleWidth ? 64 : 32); column++) {
			uint16_t addr = (baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;

			bool vMirror = (vram[addr + 1] & 0x80) != 0;
			bool hMirror = (vram[addr + 1] & 0x40) != 0;

			uint16_t tileIndex = ((vram[addr + 1] & 0x03) << 8) | vram[addr];
			uint16_t tileStart = layer.ChrAddress + tileIndex * 8 * bpp;

			if(largeTileWidth || largeTileHeight) {
				tileIndex = (
					tileIndex +
					(largeTileHeight ? ((row & 0x01) ? (vMirror ? 0 : 16) : (vMirror ? 16 : 0)) : 0) +
					(largeTileWidth ? ((column & 0x01) ? (hMirror ? 0 : 1) : (hMirror ? 1 : 0)) : 0)
					) & 0x3FF;
			}

			for(int y = 0; y < 8; y++) {
				uint8_t yOffset = vMirror ? (7 - y) : y;
				uint16_t pixelStart = tileStart + yOffset * 2;

				for(int x = 0; x < 8; x++) {
					uint8_t shift = hMirror ? x : (7 - x);
					uint16_t color = GetTilePixelColor(bpp, pixelStart, shift);
					if(color != 0) {
						uint16_t paletteColor;
						/*if(bpp == 8 && directColorMode) {
							uint8_t palette = (vram[addr + 1] >> 2) & 0x07;
								paletteColor = (
								(((color & 0x07) | (palette & 0x01)) << 1) |
								(((color & 0x38) | ((palette & 0x02) << 1)) << 3) |
								(((color & 0xC0) | ((palette & 0x04) << 3)) << 7)
							);
						} else {*/
						uint8_t palette = bpp == 8 ? 0 : (vram[addr + 1] >> 2) & 0x07;
						uint16_t paletteRamOffset = basePaletteOffset + (palette * (1 << bpp) + color) * 2;
						paletteColor = cgram[paletteRamOffset] | (cgram[paletteRamOffset + 1] << 8);
						//}

						outBuffer[((row * 8) + y) * 512 + column * 8 + x] = ToArgb(paletteColor);
					}
				}
			}
		}
	}

	if(options.ShowTileGrid) {
		uint32_t gridColor = 0xA0AAAAFF;
		for(int i = 0; i < 512 * 512; i++) {
			if((i & 0x07) == 0x07 || (i & 0x0E00) == 0x0E00) {
				BlendColors((uint8_t*)&outBuffer[i], (uint8_t*)&gridColor);
			}
		}
	}

	if(options.ShowScrollOverlay) {
		uint32_t overlayColor = 0x40FFFFFF;
		for(int y = 0; y < 240; y++) {
			for(int x = 0; x < 256; x++) {
				int xPos = layer.HScroll + x;
				int yPos = layer.VScroll + y;
				
				xPos &= layer.DoubleWidth ? 0x1FF : 0xFF;
				yPos &= layer.DoubleHeight ? 0x1FF : 0xFF;

				if(x == 0 || y == 0 || x == 255 || y == 239) {
					outBuffer[(yPos << 9) | xPos] = 0xAFFFFFFF;
				} else {
					BlendColors((uint8_t*)&outBuffer[(yPos << 9) | xPos], (uint8_t*)&overlayColor);
				}
			}
		}
	}
}

void PpuTools::SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle)
{
	//TODO Thread safety
	_updateTimings[viewerId] = (scanline << 16) | cycle;
}

void PpuTools::RemoveViewer(uint32_t viewerId)
{
	//TODO Thread safety
	_updateTimings.erase(viewerId);
}

void PpuTools::UpdateViewers(uint16_t scanline, uint16_t cycle)
{
	uint32_t currentCycle = (scanline << 16) | cycle;
	for(auto updateTiming : _updateTimings) {
		if(updateTiming.second == currentCycle) {
			_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::ViewerRefresh, (void*)(uint64_t)updateTiming.first);
		}
	}
}