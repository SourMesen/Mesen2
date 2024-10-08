#include "pch.h"
#include "SMS/Debugger/SmsVdpTools.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsVdp.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/SettingTypes.h"

SmsVdpTools::SmsVdpTools(Debugger* debugger, Emulator *emu, SmsConsole* console) : PpuTools(debugger, emu)
{
	_console = console;
}

FrameInfo SmsVdpTools::GetTilemapSize(GetTilemapOptions options, BaseState& baseState)
{
	SmsVdpState& state = (SmsVdpState&)baseState;
	bool isTextMode = !state.UseMode4 && state.M1_Use224LineMode;
	return { isTextMode ? 240u : 256u, state.NametableHeight };
}

DebugTilemapInfo SmsVdpTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	SmsVdpState& state = (SmsVdpState&)baseState;

	bool isGameGear = _console->GetModel() == SmsModel::GameGear;
	bool isTextMode = !state.UseMode4 && state.M1_Use224LineMode;

	DebugTilemapInfo result = {};
	result.Bpp = state.UseMode4 ? 4 : 1;
	result.Format = state.UseMode4 ? TileFormat::SmsBpp4 : TileFormat::SmsSgBpp1;
	result.TileWidth = isTextMode ? 6 : 8;
	result.TileHeight = 8;
	result.ColumnCount = isTextMode ? 40 : 32;
	result.RowCount = state.NametableHeight / 8;
	result.TilesetAddress = 0;
	result.ScrollWidth = isGameGear ? 160 : 256;
	result.ScrollHeight = isGameGear ? 144 : state.VisibleScanlineCount;
	
	uint8_t colorMask = 0xFF;
	if(options.DisplayMode == TilemapDisplayMode::Grayscale) {
		palette = (uint32_t*)_grayscaleColorsBpp4;
		colorMask = 0x0F;
	}

	if(state.UseMode4) {
		result.ScrollX = 256 - state.HorizontalScroll + (isGameGear ? 48 : 0);
		result.ScrollY = state.VerticalScroll + (isGameGear ? 24 : 0);
		result.TilemapAddress = state.EffectiveNametableAddress;

		for(uint8_t row = 0; row < result.RowCount; row++) {
			for(uint8_t column = 0; column < 32; column++) {
				uint16_t entryAddr = state.EffectiveNametableAddress + ((row * 32 + column) * 2);
				uint16_t ntData = vram[entryAddr] | (vram[entryAddr + 1] << 8);

				uint8_t paletteOffset = ntData & 0x800 ? 0x10 : 0;
				bool hMirror = ntData & 0x200;
				bool vMirror = ntData & 0x400;
				uint16_t tileIndex = ntData & 0x1FF;

				for(int y = 0; y < 8; y++) {
					uint8_t tileRow = vMirror ? 7 - (y & 0x07) : (y & 0x07);
					uint16_t tileAddr = tileIndex * 32 + tileRow * 4;
					for(int x = 0; x < 8; x++) {
						uint8_t tileColumn = hMirror ? 7 - (x & 0x07) : (x & 0x07);

						uint8_t color = GetTilePixelColor<TileFormat::SmsBpp4>(vram, 0x3FFF, tileAddr, tileColumn);
						uint16_t palAddr = color == 0 ? 0 : (paletteOffset + color);
						uint32_t outPos = (row * 8 + y) * 32 * 8 + column * 8 + x;
						outBuffer[outPos] = palette[palAddr & colorMask];
					}
				}
			}
		}
	} else if(state.M1_Use224LineMode) {
		//Text mode (mode 1)
		result.TilemapAddress = state.NametableAddress;
		for(uint8_t row = 0; row < result.RowCount; row++) {
			for(uint8_t column = 0; column < result.ColumnCount; column++) {
				uint16_t ntAddr = state.NametableAddress + (column + row * 40);

				uint16_t tileIndex = vram[ntAddr];
				uint16_t tileAddr = (state.BgPatternTableAddress & 0x3800) + (tileIndex * 8);

				for(int y = 0; y < 8; y++) {
					for(int x = 0; x < 6; x++) {
						uint32_t outPos = (row * 8 + y) * 40 * 6 + column * 6 + x;
						uint8_t colorBit = GetTilePixelColor<TileFormat::SmsSgBpp1>(vram, 0x3FFF, tileAddr + y, x);
						outBuffer[outPos] = palette[colorBit ? state.TextColorIndex : state.BackgroundColorIndex];
					}
				}
			}
		}
	} else {
		//Graphic 1 / Graphic 2 / Multicolor
		result.TilemapAddress = state.NametableAddress;

		for(uint8_t row = 0; row < result.RowCount; row++) {
			for(uint8_t column = 0; column < result.ColumnCount; column++) {
				uint16_t ntAddr = state.NametableAddress + (column + row * 32);

				uint16_t tileIndex = vram[ntAddr];
				uint16_t tileAddr;
				if(state.M3_Use240LineMode) {
					tileAddr = (state.BgPatternTableAddress & 0x3800) + (tileIndex * 8) + (row & 0x03) * 2;
				} else if(state.M2_AllowHeightChange) {
					//Move to the next 256 tiles after every 8 tile rows
					tileIndex += (row & 0x18) << 5;
					uint16_t mask = ((state.BgPatternTableAddress >> 3) | 0xFF) & 0x3FF;
					tileAddr = (state.BgPatternTableAddress & 0x2000) + ((tileIndex & mask) * 8);
				} else {
					tileAddr = (state.BgPatternTableAddress & 0x3800) + (tileIndex * 8);
				}

				uint16_t colorTableAddr;
				if(state.M2_AllowHeightChange) {
					uint16_t mask = ((state.ColorTableAddress >> 3) | 0x07) & 0x3FF;
					colorTableAddr = (state.ColorTableAddress & 0x2000) | ((tileIndex & mask) << 3);
				} else {
					colorTableAddr = (state.ColorTableAddress & 0x3FC0) | ((tileIndex >> 3) & 0x1F);
				}

				for(int y = 0; y < 8; y++) {
					uint8_t color;
					if(state.M3_Use240LineMode) {
						color = vram[tileAddr + (y >= 4 ? 1 : 0)];
					} else if(state.M2_AllowHeightChange) {
						color = vram[colorTableAddr + y];
					} else {
						color = vram[colorTableAddr];
					}

					for(int x = 0; x < 8; x++) {
						uint32_t outPos = (row * 8 + y) * 32 * 8 + column * 8 + x;
						uint8_t pixelColor;
						if(state.M3_Use240LineMode) {
							pixelColor = x < 4 ? (color >> 4) : (color & 0xF);
						} else {
							uint8_t colorBit = GetTilePixelColor<TileFormat::SmsSgBpp1>(vram, 0x3FFF, tileAddr + y, x);
							pixelColor = colorBit ? (color >> 4) : (color & 0xF);
						}
						outBuffer[outPos] = palette[pixelColor];
					}
				}
			}
		}
	}

	return result;
}

DebugTilemapTileInfo SmsVdpTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugTilemapTileInfo result = {};

	FrameInfo size = GetTilemapSize(options, baseState);
	if(x >= size.Width || y >= size.Height) {
		return result;
	}

	SmsVdpState& state = (SmsVdpState&)baseState;
	bool isTextMode = !state.UseMode4 && state.M1_Use224LineMode;

	result.Width = isTextMode ? 6 : 8;
	result.Height = 8;

	uint8_t row = y / 8;
	uint8_t column = x / result.Width;
	
	result.Row = row;
	result.Column = column;

	if(state.UseMode4) {
		uint16_t ntIndex = (row << 5) + column;
		uint16_t entryAddr = state.EffectiveNametableAddress + (ntIndex * 2);
		uint16_t ntData = vram[entryAddr] | (vram[entryAddr + 1] << 8);
	
		uint8_t paletteOffset = ntData & 0x800 ? 0x10 : 0;
		uint16_t tileIndex = ntData & 0x1FF;

		result.TileMapAddress = entryAddr;
		result.TileIndex = tileIndex;
		result.TileAddress = tileIndex * 32;
		result.PaletteIndex = paletteOffset >> 4;
		result.PaletteAddress = paletteOffset;
		result.HighPriority = (ntData & 0x1000) ? NullableBoolean::True : NullableBoolean::False;
		result.VerticalMirroring = (ntData & 0x400) ? NullableBoolean::True : NullableBoolean::False;
		result.HorizontalMirroring = (ntData & 0x200) ? NullableBoolean::True : NullableBoolean::False;
	} else if(state.M1_Use224LineMode) {
		//Text mode (mode 1)
		uint16_t ntAddr = state.NametableAddress + (column + row * 40);
		int32_t tileIndex = vram[ntAddr];
		uint16_t tileAddr = (state.BgPatternTableAddress & 0x3800) + (tileIndex * 8);
		result.TileMapAddress = ntAddr;
		result.TileIndex = tileIndex;
		result.TileAddress = tileAddr;
	} else {
		//Graphic 1 / Graphic 2 / Multicolor
		uint16_t ntAddr = state.NametableAddress + (column + row * 32);

		int32_t tileIndex = vram[ntAddr];
		uint16_t tileAddr;
		if(state.M3_Use240LineMode) {
			tileAddr = (state.BgPatternTableAddress & 0x3800) + (tileIndex * 8) + (row & 0x03) * 2;
			tileIndex = -1;
		} else if(state.M2_AllowHeightChange) {
			//Move to the next 256 tiles after every 8 tile rows
			tileIndex += (row & 0x18) << 5;
			uint16_t mask = ((state.BgPatternTableAddress >> 3) | 0xFF) & 0x3FF;
			tileAddr = (state.BgPatternTableAddress & 0x2000) + ((tileIndex & mask) * 8);
		} else {
			tileAddr = (state.BgPatternTableAddress & 0x3800) + (tileIndex * 8);
		}
		result.TileMapAddress = ntAddr;
		result.TileIndex = tileIndex;
		result.TileAddress = tileAddr;

		int32_t colorTableAddr;
		if(state.M3_Use240LineMode) {
			colorTableAddr = -1;
		} else if(state.M2_AllowHeightChange) {
			uint16_t mask = ((state.ColorTableAddress >> 3) | 0x07) & 0x3FF;
			colorTableAddr = (state.ColorTableAddress & 0x2000) | ((tileIndex & mask) << 3);
		} else {
			colorTableAddr = (state.ColorTableAddress & 0x3FC0) | ((tileIndex >> 3) & 0x1F);
		}

		result.PaletteAddress = colorTableAddr;
	}
	return result;
}

void SmsVdpTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer)
{
	SmsVdpState& state = (SmsVdpState&)baseState;
	
	uint32_t bgColor = GetSpriteBackgroundColor(options.Background, palette, false);

	std::fill(outBuffer, outBuffer + 256 * state.VisibleScanlineCount, bgColor);
	std::fill(outBuffer + 256 * state.VisibleScanlineCount, outBuffer + 256 * 256, GetSpriteBackgroundColor(options.Background, palette, true));

	if(_console->GetModel() == SmsModel::GameGear) {
		std::fill(outBuffer, outBuffer + 24 * 256, GetSpriteBackgroundColor(options.Background, palette, true));
		std::fill(outBuffer + 168 * 256, outBuffer + state.VisibleScanlineCount * 256, GetSpriteBackgroundColor(options.Background, palette, true));
		for(int i = 0; i < state.VisibleScanlineCount; i++) {
			std::fill(outBuffer + i * 256, outBuffer + i * 256 + 48, GetSpriteBackgroundColor(options.Background, palette, true));
			std::fill(outBuffer + i * 256 + 208, outBuffer + i * 256 + 256, GetSpriteBackgroundColor(options.Background, palette, true));
		}
	}

	int spriteCount = state.UseMode4 ? 64 : 32;
	for(int i = spriteCount - 1; i >= 0; i--) {
		DebugSpriteInfo& sprite = sprites[i];
		uint32_t* spritePreview = spritePreviews + i * _spritePreviewSize;

		int spritePosY = sprite.Y + 1;

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

					//TODOSMS zoomed sprites support
					outBuffer[((spritePosY + y) * 256) + sprite.X + x] = color;
				} else {
					spritePreview[y * sprite.Width + x] = bgColor;
				}
			}
		}
	}
}

DebugSpritePreviewInfo SmsVdpTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	SmsVdpState& state = (SmsVdpState&)baseState;
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 256;
	info.SpriteCount = state.UseMode4 ? 64 : 32;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 1;

	if(_console->GetModel() == SmsModel::GameGear) {
		info.VisibleX = 48;
		info.VisibleY = 24;
		info.VisibleWidth = 160;
		info.VisibleHeight = 144;
	} else {
		info.VisibleX = 0;
		info.VisibleY = 0;
		info.VisibleWidth = 256;
		info.VisibleHeight = state.VisibleScanlineCount;
	}
	
	info.WrapBottomToTop = true;

	return info;
}

void SmsVdpTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t i, GetSpritePreviewOptions& options, SmsVdpState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	uint8_t* oam = oamRam ? oamRam : (vram + (state.SpriteTableAddress & (state.UseMode4 ? 0x3F00 : 0x3FFF)));

	sprite.Bpp = state.UseMode4 ? 4 : 1;
	sprite.Format = state.UseMode4 ? TileFormat::SmsBpp4 : TileFormat::SmsSgBpp1;
	sprite.SpriteIndex = i;
	sprite.UseExtendedVram = false;
	sprite.RawY = state.UseMode4 ? oam[i] : oam[i*4];
	sprite.RawX = state.UseMode4 ? oam[0x80+i*2] : oam[i*4+1];
	sprite.Y = sprite.RawY;
	sprite.X = sprite.RawX;
	
	if(state.UseMode4) {
		if(state.ShiftSpritesLeft) {
			sprite.X -= 8;
		}
	} else {
		if(oam[i * 4 + 3] & 0x80) {
			sprite.X -= 32;
		}
	}

	sprite.TileIndex = state.UseMode4 ? oam[0x80 + i * 2 + 1] : oam[i*4+2];
	sprite.UseSecondTable = NullableBoolean::Undefined;

	sprite.Palette = state.UseMode4 ? 0 : (oam[i*4+3] & 0x0F);
	sprite.PaletteAddress = state.UseMode4 ? 0x10 : -1;
	sprite.Priority = DebugSpritePriority::Undefined;
	
	bool largeSprites = state.UseLargeSprites;
	sprite.Width = 8;
	sprite.Height = largeSprites ? 16 : 8;
	if(!state.UseMode4) {
		sprite.Width = sprite.Height;
	}

	if(!state.UseMode4 && sprite.Palette == 0) {
		sprite.Visibility = SpriteVisibility::Disabled;
	} else if(sprite.Y < state.VisibleScanlineCount || (sprite.Y > state.VisibleScanlineCount && (uint8_t)(sprite.Y + sprite.Height) >= 0)) {
		sprite.Visibility = SpriteVisibility::Visible;
	} else {
		sprite.Visibility = SpriteVisibility::Offscreen;
	}

	uint16_t tileIndex = sprite.TileIndex;
	if(largeSprites) {
		tileIndex &= state.UseMode4 ? ~0x01 : ~0x03;
	}
	uint16_t tileStart = (tileIndex * (state.UseMode4 ? 32 : 8)) | (state.SpritePatternSelector & (state.UseMode4 ? 0x2000 : ~0));
	sprite.TileAddresses[0] = tileStart;
	if(largeSprites) {
		if(state.UseMode4) {
			sprite.TileAddresses[1] = tileStart + 32;
			sprite.TileCount = 2;
		} else {
			sprite.TileAddresses[1] = tileStart + 8;
			sprite.TileAddresses[2] = tileStart + 16;
			sprite.TileAddresses[3] = tileStart + 24;
			sprite.TileCount = 4;
		}
	} else {
		sprite.TileCount = 1;
	}
	sprite.TileAddress = tileStart;

	for(int y = 0; y < sprite.Height; y++) {
		uint16_t pixelStart = tileStart + y * (state.UseMode4 ? 4 : 1);

		for(int x = 0; x < sprite.Width; x++) {
			uint8_t color;
			if(state.UseMode4) {
				color = GetTilePixelColor<TileFormat::SmsBpp4>(vram, 0x3FFF, pixelStart, x);
			} else {
				color = GetTilePixelColor<TileFormat::SmsSgBpp1>(vram, 0x3FFF, pixelStart + (x >= 8 ? 16 : 0), x & 0x07);
			}

			uint32_t outOffset = (y * sprite.Width) + x;
			if(color > 0) {
				spritePreview[outOffset] = state.UseMode4 ? palette[0x10 + color] : (sprite.Palette ? palette[sprite.Palette] : 0);
			} else {
				spritePreview[outOffset] = 0;
			}
		}
	}
}

void SmsVdpTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview)
{
	SmsVdpState& state = (SmsVdpState&)baseState;
	int spriteCount = state.UseMode4 ? 64 : 32;

	bool disableSprites = false;
	for(int i = 0; i < spriteCount; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], spritePreviews + (i * _spritePreviewSize), i, options, state, vram, oamRam, palette);

		disableSprites |= outBuffer[i].RawY == 0xD0;
		if(disableSprites) {
			outBuffer[i].Visibility = SpriteVisibility::Disabled;
		}
	}

	GetSpritePreview(options, baseState, outBuffer, spritePreviews, palette, screenPreview);
}

DebugPaletteInfo SmsVdpTools::GetPaletteInfo(GetPaletteInfoOptions options)
{
	DebugPaletteInfo info = {};

	SmsVdpState state;
	_debugger->GetPpuState(state, CpuType::Sms);

	info.ColorCount = 16;
	info.BgColorCount = 16;
	info.SpriteColorCount = 16;
	info.ColorsPerPalette = 16;
	
	if(state.UseMode4) {
		info.ColorCount = 32;
		info.SpritePaletteOffset = info.BgColorCount;
		info.HasMemType = true;
		info.PaletteMemType = MemoryType::SmsPaletteRam;
		info.RawFormat = _console->GetModel() == SmsModel::GameGear ? RawPaletteFormat::Rgb444 : RawPaletteFormat::Rgb222;

		uint8_t* paletteRam = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::SmsPaletteRam);

		if(info.RawFormat == RawPaletteFormat::Rgb222) {
			for(int i = 0; i < 32; i++) {
				info.RawPalette[i] = paletteRam[i];
				info.RgbPalette[i] = ColorUtilities::Rgb222ToArgb(paletteRam[i]);
			}
		} else {
			for(int i = 0; i < 32; i++) {
				info.RawPalette[i] = paletteRam[i * 2] | (paletteRam[i * 2 + 1] << 8);
				info.RgbPalette[i] = ColorUtilities::Rgb444ToArgb(info.RawPalette[i]);
			}
		}
	} else {
		info.ColorCount = 16;
		info.SpritePaletteOffset = 0;
		info.RawFormat = RawPaletteFormat::Indexed;
		for(int i = 0; i < 16; i++) {
			info.RawPalette[i] = _console->GetVdp()->GetSmsSgPalette()[i];
			info.RgbPalette[i] = ColorUtilities::Rgb555ToArgb(info.RawPalette[i]);
		}
	}

	return info;
}

void SmsVdpTools::SetPaletteColor(int32_t colorIndex, uint32_t color)
{
	if(_console->GetModel() == SmsModel::GameGear) {
		uint8_t r = (color >> 20) & 0x0F;
		uint8_t g = (color >> 12) & 0x0F;
		uint8_t b = (color >> 4) & 0x0F;

		uint16_t rgb444 = (b << 8) | (g << 4) | r;
		_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::SmsPaletteRam, colorIndex * 2, rgb444 & 0xFF);
		_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::SmsPaletteRam, colorIndex * 2 + 1, (rgb444 >> 8));
	} else {
		if(color < 0x3F) {
			_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::SmsPaletteRam, colorIndex, (uint8_t)color);
		}
	}
}
