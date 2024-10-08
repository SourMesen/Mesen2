#include "pch.h"
#include "WS/Debugger/WsPpuTools.h"
#include "WS/WsTypes.h"
#include "WS/WsConsole.h"
#include "WS/WsPpu.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/SettingTypes.h"

WsPpuTools::WsPpuTools(Debugger* debugger, Emulator *emu, WsConsole* console) : PpuTools(debugger, emu)
{
	_console = console;
}

FrameInfo WsPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& baseState)
{
	return { 256, 256 };
}

DebugTilemapInfo WsPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	WsPpuState& state = (WsPpuState&)baseState;

	uint16_t ramMask = state.Mode == WsVideoMode::Monochrome ? 0x3FFF : 0xFFFF;
	uint16_t baseAddr = state.BgLayers[options.Layer].MapAddress;

	std::fill(outBuffer, outBuffer + 256 * 256, 0xFFFFFFFF);

	int tileSize = state.Mode >= WsVideoMode::Color4bpp ? 32 : 16;
	int bpp = state.Mode >= WsVideoMode::Color4bpp ? 4 : 2;
	int paletteSize = state.Mode == WsVideoMode::Monochrome ? 4 : 16;
	int bank0Addr = state.Mode >= WsVideoMode::Color4bpp ? 0x4000 : 0x2000;
	int bank1Addr = state.Mode >= WsVideoMode::Color4bpp ? 0x8000 : 0x4000;

	uint8_t colorMask = 0xFF;
	if(options.DisplayMode == TilemapDisplayMode::Grayscale) {
		palette = (uint32_t*)_grayscaleColorsBpp2;
		colorMask = 0x03;
	}

	for(int row = 0; row < 32; row++) {
		uint16_t baseOffset = baseAddr + row * 64;

		for(int column = 0; column < 32; column++) {
			uint16_t tilemapAddr = (baseOffset + column * 2);

			uint16_t tilemapData = vram[tilemapAddr] | (vram[tilemapAddr + 1] << 8);
			uint16_t tileIndex = tilemapData & 0x1FF;
			uint8_t tilePalette = (tilemapData >> 9) & 0x0F;
			bool vMirror = tilemapData & 0x8000;
			bool hMirror = tilemapData & 0x4000;

			uint16_t tilesetAddr = state.Mode >= WsVideoMode::Color2bpp && (tilemapData & 0x2000) ? bank1Addr : bank0Addr;
			uint16_t tileStart = tilesetAddr + tileIndex * tileSize;

			for(int y = 0; y < 8; y++) {
				uint8_t tileRow = vMirror ? 7 - (y & 0x07) : (y & 0x07);
				uint16_t tileAddr = tileStart + tileRow * bpp;
				for(int x = 0; x < 8; x++) {
					uint8_t tileColumn = hMirror ? 7 - (x & 0x07) : (x & 0x07);
					uint8_t color;
					switch(state.Mode) {
						default:
						case WsVideoMode::Monochrome:
						case WsVideoMode::Color2bpp:
							color = GetTilePixelColor<TileFormat::Bpp2>(vram, ramMask, tileAddr, tileColumn);
							break;

						case WsVideoMode::Color4bpp:
							color = GetTilePixelColor<TileFormat::SmsBpp4>(vram, ramMask, tileAddr, tileColumn);
							break;

						case WsVideoMode::Color4bppPacked:
							color = GetTilePixelColor<TileFormat::WsBpp4Packed>(vram, ramMask, tileAddr, tileColumn);
							break;
					}

					if(color != 0 || (!(tilePalette & 0x04) && state.Mode <= WsVideoMode::Color2bpp)) {
						uint32_t outPos = (row * 8 + y) * 256 + column * 8 + x;
						outBuffer[outPos] = palette[((tilePalette * paletteSize) + color) & colorMask];
					}
				}
			}
		}
	}

	DebugTilemapInfo result = {};
	result.Bpp = bpp;

	switch(state.Mode) {
		case WsVideoMode::Monochrome:
		case WsVideoMode::Color2bpp:
			result.Format = TileFormat::Bpp2;
			break;

		case WsVideoMode::Color4bpp:
			result.Format = TileFormat::SmsBpp4;
			break;

		case WsVideoMode::Color4bppPacked:
			result.Format = TileFormat::WsBpp4Packed;
			break;
	}

	result.TileWidth = 8;
	result.TileHeight = 8;
	result.ColumnCount = 32;
	result.RowCount = 32;
	result.TilemapAddress = baseAddr;
	result.TilesetAddress = bank0Addr;
	result.ScrollX = state.BgLayers[options.Layer].ScrollX;
	result.ScrollY = state.BgLayers[options.Layer].ScrollY;
	result.ScrollWidth = WsConstants::ScreenWidth;
	result.ScrollHeight = WsConstants::ScreenHeight;
	return result;
}

DebugTilemapTileInfo WsPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugTilemapTileInfo result = {};

	FrameInfo size = GetTilemapSize(options, baseState);
	if(x >= size.Width || y >= size.Height) {
		return result;
	}

	WsPpuState& state = (WsPpuState&)baseState;

	int tileSize = state.Mode >= WsVideoMode::Color4bpp ? 32 : 16;
	int bank0Addr = state.Mode >= WsVideoMode::Color4bpp ? 0x4000 : 0x2000;
	int bank1Addr = state.Mode >= WsVideoMode::Color4bpp ? 0x8000 : 0x4000;

	int row = y / 8;
	int column = x / 8;

	uint16_t baseAddr = state.BgLayers[options.Layer].MapAddress;
	
	uint16_t tilemapAddr = (baseAddr + row * 64 + column * 2);

	uint16_t tilemapData = vram[tilemapAddr] | (vram[tilemapAddr + 1] << 8);
	uint16_t tileIndex = tilemapData & 0x1FF;
	uint8_t tilePalette = (tilemapData >> 9) & 0x0F;
	bool vMirror = tilemapData & 0x8000;
	bool hMirror = tilemapData & 0x4000;

	uint16_t tilesetAddr = state.Mode >= WsVideoMode::Color2bpp && (tilemapData & 0x2000) ? bank1Addr : bank0Addr;

	result.Column = column;
	result.Row = row;
	result.Height = 8;
	result.Width = 8;
	result.TileMapAddress = tilemapAddr;
	result.TileIndex = tileIndex;
	result.TileAddress = tilesetAddr + tileIndex * tileSize;

	result.PaletteIndex = tilePalette;
	result.PaletteAddress = state.Mode >= WsVideoMode::Color2bpp ? (0xFE00 | (result.PaletteIndex << 4)) : (result.PaletteIndex << 2);
	result.HorizontalMirroring = (NullableBoolean)hMirror;
	result.VerticalMirroring = (NullableBoolean)vMirror;

	return result;
}

void WsPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer)
{
	uint32_t bgColor = GetSpriteBackgroundColor(options.Background, palette, false);

	uint32_t darkBg = GetSpriteBackgroundColor(options.Background, palette, true);
	std::fill(outBuffer, outBuffer + 256 * WsConstants::ScreenHeight, bgColor);
	std::fill(outBuffer + 256 * WsConstants::ScreenHeight, outBuffer + 256 * 256, darkBg);
	for(int i = 0; i < WsConstants::ScreenHeight; i++) {
		std::fill(outBuffer + WsConstants::ScreenWidth + i * 256, outBuffer + 256 +  i * 256 , darkBg);
	}
	
	int spriteCount = 128;
	for(int i = spriteCount - 1; i >= 0; i--) {
		DebugSpriteInfo& sprite = sprites[i];
		uint32_t* spritePreview = spritePreviews + i * _spritePreviewSize;

		int spritePosY = sprite.Y;

		for(int y = 0; y < sprite.Height; y++) {
			for(int x = 0; x < sprite.Width; x++) {
				if(spritePosY + y >= 256) {
					spritePosY -= 256;
				}

				uint32_t color = spritePreview[y * sprite.Width + x];
				if(color != 0) {
					if(sprite.X + x >= 256 || sprite.Visibility == SpriteVisibility::Disabled) {
						continue;
					}

					outBuffer[((spritePosY + y) * 256) + sprite.X + x] = color;
				} else {
					spritePreview[y * sprite.Width + x] = bgColor;
				}
			}
		}
	}
}

DebugSpritePreviewInfo WsPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 256;
	info.SpriteCount = 128;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 0;
	info.WrapBottomToTop = true;
	info.WrapRightToLeft = true;

	info.VisibleX = 0;
	info.VisibleY = 0;
	info.VisibleWidth = WsConstants::ScreenWidth;
	info.VisibleHeight = WsConstants::ScreenHeight;

	return info;
}

void WsPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t i, GetSpritePreviewOptions& options, WsPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	uint16_t ramMask = state.Mode == WsVideoMode::Monochrome ? 0x3FFF : 0xFFFF;

	uint8_t* oam = oamRam ? oamRam : (vram + state.SpriteTableAddress);

	int tileSize = state.Mode >= WsVideoMode::Color4bpp ? 32 : 16;
	int bpp = state.Mode >= WsVideoMode::Color4bpp ? 4 : 2;
	int bank0Addr = state.Mode >= WsVideoMode::Color4bpp ? 0x4000 : 0x2000;

	sprite.Bpp = bpp;

	switch(state.Mode) {
		case WsVideoMode::Monochrome:
		case WsVideoMode::Color2bpp:
			sprite.Format = TileFormat::Bpp2;
			break;

		case WsVideoMode::Color4bpp:
			sprite.Format = TileFormat::SmsBpp4;
			break;

		case WsVideoMode::Color4bppPacked:
			sprite.Format = TileFormat::WsBpp4Packed;
			break;
	}

	sprite.SpriteIndex = i;
	sprite.UseExtendedVram = false;
	sprite.RawY = oam[i*4 + 2];
	sprite.RawX = oam[i*4 + 3];
	sprite.Y = sprite.RawY;
	sprite.X = sprite.RawX;
	sprite.UseSecondTable = NullableBoolean::Undefined;
	
	uint8_t attributes = oam[i*4 + 1];
	bool highPriority = attributes & 0x20;

	bool vMirror = attributes & 0x80;
	bool hMirror = attributes & 0x40;

	uint16_t tileIndex = oam[i*4] | ((attributes & 0x01) << 8);
	sprite.TileIndex = tileIndex;
	uint8_t sprPalette = ((attributes >> 1) & 0x07);
	int paletteSize = state.Mode == WsVideoMode::Monochrome ? 4 : 16;

	sprite.Palette = sprPalette;
	sprite.PaletteAddress = (8 + sprPalette) * paletteSize;
	sprite.Priority = highPriority ? DebugSpritePriority::Foreground : DebugSpritePriority::Background;
	
	sprite.Width = 8;
	sprite.Height = 8;

	if(i < state.FirstSpriteIndex || i >= state.FirstSpriteIndex + state.SpriteCount) {
		sprite.Visibility = SpriteVisibility::Disabled;
	} else if(sprite.Y < WsConstants::ScreenHeight && sprite.X < WsConstants::ScreenWidth) {
		sprite.Visibility = SpriteVisibility::Visible;
	} else {
		sprite.Visibility = SpriteVisibility::Offscreen;
	}

	uint16_t tilesetAddr = bank0Addr;
	uint16_t tileStart = tilesetAddr + (sprite.TileIndex * tileSize);
	sprite.TileAddresses[0] = tileStart;
	sprite.TileCount = 1;
	sprite.TileAddress = tileStart;

	for(int y = 0; y < 8; y++) {
		uint8_t yPos = vMirror ? 7 - y : y;
		uint16_t pixelStart = tileStart + yPos * bpp;
		for(int x = 0; x < 8; x++) {
			uint8_t color;
			uint8_t xPos = hMirror ? 7 - x : x;
			switch(state.Mode) {
				default:
				case WsVideoMode::Monochrome:
				case WsVideoMode::Color2bpp:
					color = GetTilePixelColor<TileFormat::Bpp2>(vram, ramMask, pixelStart, xPos);
					break;

				case WsVideoMode::Color4bpp:
					color = GetTilePixelColor<TileFormat::SmsBpp4>(vram, ramMask, pixelStart, xPos);
					break;

				case WsVideoMode::Color4bppPacked:
					color = GetTilePixelColor<TileFormat::WsBpp4Packed>(vram, ramMask, pixelStart, xPos);
					break;
			}

			uint32_t outOffset = (y * 8) + x;
			if(color > 0 || (!(sprPalette & 0x04) && state.Mode <= WsVideoMode::Color2bpp)) {
				spritePreview[outOffset] = palette[sprite.PaletteAddress + color];
			} else {
				spritePreview[outOffset] = 0;
			}
		}
	}
}

void WsPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview)
{
	WsPpuState& state = (WsPpuState&)baseState;
	int spriteCount = 128;

	for(int i = 0; i < spriteCount; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], spritePreviews + (i * _spritePreviewSize), i, options, state, vram, oamRam, palette);
	}

	GetSpritePreview(options, baseState, outBuffer, spritePreviews, palette, screenPreview);
}

DebugPaletteInfo WsPpuTools::GetPaletteInfo(GetPaletteInfoOptions options)
{
	DebugPaletteInfo info = {};

	WsPpuState state;
	_debugger->GetPpuState(state, CpuType::Ws);

	if(state.Mode == WsVideoMode::Monochrome) {
		info.ColorCount = 64;
		info.BgColorCount = 32;
		info.SpriteColorCount = 32;
		info.ColorsPerPalette = 4;
		info.SpritePaletteOffset = info.BgColorCount;

		info.RawFormat = RawPaletteFormat::Indexed;
		for(int i = 0; i < 64; i++) {
			info.RawPalette[i] = state.BwPalettes[i];
			uint8_t brightness = state.BwShades[state.BwPalettes[i]] ^ 0x0F;
			info.RgbPalette[i] = ColorUtilities::Rgb444ToArgb(brightness | (brightness << 4) | (brightness << 8));
		}
	} else {
		info.ColorCount = 256;
		info.BgColorCount = 128;
		info.SpriteColorCount = 128;
		info.ColorsPerPalette = 16;
		info.SpritePaletteOffset = info.BgColorCount;

		info.HasMemType = true;
		info.PaletteMemType = MemoryType::WsWorkRam;
		info.PaletteMemOffset = 0xFE00;

		uint8_t* palette = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::WsWorkRam);

		info.RawFormat = RawPaletteFormat::Rgb444;
		for(int i = 0; i < 256; i++) {
			int addr = 0xFE00 + i * 2;
			info.RawPalette[i] = palette[addr] | ((palette[addr + 1] & 0x0F) << 8);
			info.RgbPalette[i] = ColorUtilities::Bgr444ToArgb(info.RawPalette[i]);
		}
	}
	return info;
}

void WsPpuTools::SetPaletteColor(int32_t colorIndex, uint32_t color)
{
	WsPpuState state;
	_debugger->GetPpuState(state, CpuType::Ws);

	if(state.Mode == WsVideoMode::Monochrome) {
		_console->GetPpu()->GetState().BwPalettes[colorIndex] = color;
	} else {
		uint8_t r = (color >> 20) & 0x0F;
		uint8_t g = (color >> 12) & 0x0F;
		uint8_t b = (color >> 4) & 0x0F;

		uint16_t rgb444 = (r << 8) | (g << 4) | b;
		_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::WsWorkRam, 0xFE00 + colorIndex * 2, rgb444 & 0xFF);
		_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::WsWorkRam, 0xFE00 + colorIndex * 2 + 1, (rgb444 >> 8));
	}
}
