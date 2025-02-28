#include "pch.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsCpu.h"
#include "SMS/SmsControlManager.h"
#include "SMS/SmsMemoryManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BaseControlManager.h"
#include "Shared/RewindManager.h"
#include "Shared/EventType.h"
#include "Shared/NotificationManager.h"
#include "Shared/ColorUtilities.h"
#include "Utilities/Serializer.h"
#include "Utilities/RandomHelper.h"
#include "Debugger/SmsVdpTools.h"

void SmsVdp::Init(Emulator* emu, SmsConsole* console, SmsCpu* cpu, SmsControlManager* controlManager, SmsMemoryManager* memoryManager)
{
	_emu = emu;
	_console = console;
	_cpu = cpu;
	_controlManager = controlManager;
	_memoryManager = memoryManager;

	_outputBuffers[0] = new uint16_t[256 * 240];
	_outputBuffers[1] = new uint16_t[256 * 240];
	_currentOutputBuffer = _outputBuffers[0];
	memset(_outputBuffers[0], 0, 256 * 240 * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, 256 * 240 * sizeof(uint16_t));

	//TODOSMS - this allows Impossible Mission to have a random stage on power on
	//Without this, the bios runs in the exact same amount of time each time, causing
	//the game to use the same value for R, which generates the same stage each time
	//Unsure what the proper fix for this is (R is supposed to be 0 at reset based on the Z80 datasheet)
	_state.Scanline = RandomHelper::GetValue(0, 200);
	_state.VCounter = _state.Scanline;

	//Offset VDP from CPU clock - passes VDPTest "HCounter correct"
	_state.Cycle = 1;

	_videoRam = new uint8_t[0x4000];
	console->InitializeRam(_videoRam, 0x4000);
	_emu->RegisterMemory(MemoryType::SmsVideoRam, _videoRam, 0x4000);

	console->InitializeRam(_paletteRam, 0x40);

	_model = _console->GetModel();
	if(_model == SmsModel::Sms) {
		if(!_console->HasBios()) {
			InitSmsPostBiosState();
		}

		for(int i = 0; i < 0x20; i++) {
			WriteSmsPalette(i, _paletteRam[i]);
		}
	} else if(_model == SmsModel::ColecoVision) {
		for(int i = 0; i < 0x20; i++) {
			WriteSmsPalette(i, _paletteRam[i]);
		}
	} else {
		InitGgPowerOnState();

		for(int i = 0; i < 0x40; i += 2) {
			WriteGameGearPalette(i, _paletteRam[i] | (_paletteRam[i + 1] << 8));
		}
	}
	_emu->RegisterMemory(MemoryType::SmsPaletteRam, _paletteRam, _model == SmsModel::GameGear ? 0x40 : 0x20);

	UpdateConfig();
	UpdateDisplayMode();
}

SmsVdp::~SmsVdp()
{
	delete[] _videoRam;
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
}

void SmsVdp::Run(uint64_t runTo)
{
	do {
		//Always need to run at least once, check condition at the end of the loop (slightly faster)
		Exec();
		_lastMasterClock += 2;
	} while(_lastMasterClock < runTo - 1);
}

void SmsVdp::UpdateConfig()
{
	bool useSgPalette = _model == SmsModel::ColecoVision || (_model == SmsModel::Sg && _emu->GetSettings()->GetSmsConfig().UseSgPalette);
	_activeSgPalette = useSgPalette ? _originalSgPalette : _smsSgPalette;

	_disableBackground = _model == SmsModel::ColecoVision ? _emu->GetSettings()->GetCvConfig().DisableBackground : _emu->GetSettings()->GetSmsConfig().DisableBackground;
	_disableSprites = _model == SmsModel::ColecoVision ? _emu->GetSettings()->GetCvConfig().DisableSprites : _emu->GetSettings()->GetSmsConfig().DisableSprites;
	_removeSpriteLimit  = _model == SmsModel::ColecoVision ? _emu->GetSettings()->GetCvConfig().RemoveSpriteLimit : _emu->GetSettings()->GetSmsConfig().RemoveSpriteLimit;
	_revision = _console->GetRevision();
}

void SmsVdp::UpdateIrqState()
{
	if(_state.VerticalBlankIrqPending && _state.EnableVerticalBlankIrq) {
		if(_model == SmsModel::ColecoVision) {
			_cpu->SetNmiLevel(true);
		} else {
			_cpu->SetIrqSource(SmsIrqSource::Vdp);
		}
	} else if(_state.ScanlineIrqPending && _state.EnableScanlineIrq) {
		_cpu->SetIrqSource(SmsIrqSource::Vdp);
	} else {
		if(_model == SmsModel::ColecoVision) {
			_cpu->SetNmiLevel(false);
		} else {
			_cpu->ClearIrqSource(SmsIrqSource::Vdp);
		}
	}
}

void SmsVdp::UpdateDisplayMode()
{
	if(_state.UseMode4) {
		if(_console->GetRegion() == ConsoleRegion::Pal || _revision == SmsRevision::Sms2) {
			//Disable the mask for 224+ line mode, micromachines doesn't except this to have an effect
			//Does it not affect the PAL SMS?
			_state.NametableAddressMask = 0x3FFF;
		} else {
			//On SMS1, not setting bit 0 forces bit 10 to 0 when fetching the nametable data
			//Ys (JP) needs this to display properly
			_state.NametableAddressMask = (_state.NametableAddress & 0x400) ? ~0 : ~0x400;
		}

		if(_revision != SmsRevision::Sms1 && _state.M2_AllowHeightChange && (_state.M3_Use240LineMode || _state.M1_Use224LineMode)) {
			_state.VisibleScanlineCount = _state.M3_Use240LineMode ? 240 : 224;
			_state.NametableHeight = 256;
			_state.EffectiveNametableAddress = (_state.NametableAddress & 0x3000) | 0x700;
		} else {
			_state.VisibleScanlineCount = 192;
			_state.NametableHeight = 224;
			_state.EffectiveNametableAddress = _state.NametableAddress & ~0x400;
		}
	} else {
		_state.VisibleScanlineCount = 192;
		_state.NametableHeight = 192;
	}
}

uint8_t SmsVdp::ReadVerticalCounter()
{
	uint16_t rollOverlimit;
	bool mode224 = _state.M1_Use224LineMode && _state.M2_AllowHeightChange;
	bool mode240 = _state.M3_Use240LineMode && _state.M2_AllowHeightChange;
	if(_region == ConsoleRegion::Pal) {
		rollOverlimit = mode240 ? 266 : (mode224 ? 258 : 242);
	} else {
		rollOverlimit = mode240 ? 0xFFFF : (mode224 ? 234 : 218);
	}

	if(_state.VCounter <= rollOverlimit) {
		return (uint8_t)_state.VCounter;
	} else {
		return (uint8_t)(_state.VCounter - _scanlineCount);
	}
}

void SmsVdp::Exec()
{
	uint16_t cyc = _state.Cycle;
	if(!_state.RenderingEnabled) {
		ExecForcedBlank();
	} else if(cyc < 256 + SmsVdp::SmsVdpLeftBorder) {
		//0 to 263 - left border + draw screen
		bool visibleScanline = _state.Scanline < _state.VisibleScanlineCount;
		if(visibleScanline || _state.VCounter == 0 || _state.VCounter == _scanlineCount - 1) {
			if(cyc < 256) {
				//0 to 255 - load BG tiles (+ sprite eval for remaining sprites)
				if(visibleScanline) {
					if(_state.UseMode4) {
						LoadBgTilesSms();
					} else {
						LoadBgTilesSg();
					}
				} else {
					if((cyc & 0x07) == 2 && (cyc & 0x18)) {
						ProcessSpriteEvaluation();
						if(_state.UseMode4) {
							ProcessSpriteEvaluation();
						}
					}
				}
			}

			if(visibleScanline && cyc >= SmsVdp::SmsVdpLeftBorder) {
				//8 to 263 - draw screen
				DrawPixel();
			}

			if(!(cyc & 0x01) && _memAccess[cyc] == SmsVdpMemAccess::None) {
				//Unused slots are available for the CPU to read/write data
				ProcessVramAccess();
			}
		} else {
			ProcessForcedBlankVblank();
		}
	} else if(cyc < 326) {
		//Cycles 264 - 325 - load sprite tiles (hblank)
		if(_state.Scanline < _state.VisibleScanlineCount || _state.VCounter == 0 || _state.Scanline == _scanlineCount - 1) {
			if(_state.UseMode4) {
				LoadSpriteTilesSms();
			} else {
				LoadSpriteTilesSg();
			}
		}

		if(!(cyc & 0x01) && _memAccess[cyc] == SmsVdpMemAccess::None) {
			//Unused slots are available for the CPU to read/write data
			ProcessVramAccess();
		}

		if(cyc >= 313) {
			ProcessScanlineEvents();
		}
	} else {
		//326-341 - sprite evaluation (first 8/16 sprites) (hblank)
		if(!(cyc & 0x01)) {
			if(_state.Scanline < _state.VisibleScanlineCount || _state.VCounter == 0 || _state.VCounter == _scanlineCount - 1) {
				if(cyc == 326) {
					_evalCounter = 0;
					_inRangeSpriteCount = 0;
					memset(_inRangeSprites, 0, sizeof(_inRangeSprites));
				}

				ProcessSpriteEvaluation();
				if(_state.UseMode4) {
					ProcessSpriteEvaluation();
				}
			}
		
			if(_memAccess[cyc] == SmsVdpMemAccess::None) {
				//Unused slots are available for the CPU to read/write data
				ProcessVramAccess();
			}
		} else if(cyc == 341) {
			ProcessEndOfScanline();
		}
	}

	_state.Cycle++;
	_needCramDot = false;
	_emu->ProcessPpuCycle<CpuType::Sms>();
}

void SmsVdp::ExecForcedBlank()
{
	uint16_t cyc = _state.Cycle;
	ProcessForcedBlankVblank();

	if(cyc >= SmsVdp::SmsVdpLeftBorder && cyc < 256 + SmsVdp::SmsVdpLeftBorder) {
		if(_state.Scanline < _state.VisibleScanlineCount) {
			DrawPixel();
		}
	} else if(cyc == 341) {
		ProcessEndOfScanline();
	} else if(cyc >= 313) {
		ProcessScanlineEvents();
	}
}

void SmsVdp::ProcessForcedBlankVblank()
{
	//When rendering is disabled (or during vblank), sprite eval runs every other slot (when in mode 4 only)
	uint16_t cyc = _state.Cycle;
	if(cyc < 256) {
		if(_state.UseMode4) {
			if(cyc == 0 || _evalCounter >= 0x40) {
				_evalCounter = 0;
				_inRangeSpriteCount = 0;
				memset(_inRangeSprites, 0, sizeof(_inRangeSprites));
			}

			if((cyc & 0x03) == 0) {
				ProcessSpriteEvaluation();
				ProcessSpriteEvaluation();
			}
		} else {
			//Outside of mode 4, the VDP fetches NT entries every other slot
			//This is not emulated (and shouldn't impact emulation)
		}

		if((cyc & 0x03) == 2) {
			//Unused slots are available for the CPU to read/write data
			ProcessVramAccess();
		}
	} else {
		if(!(cyc & 0x01)) {
			//Unused slots are available for the CPU to read/write data
			ProcessVramAccess();
		}
	}
}

uint8_t SmsVdp::ReadVram(uint16_t addr, SmsVdpMemAccess type)
{
	_memAccess[_state.Cycle] = type;
	return _videoRam[addr];
}

void SmsVdp::WriteVram(uint16_t addr, uint8_t value, SmsVdpMemAccess type)
{
	_memAccess[_state.Cycle] = type;
	_videoRam[addr] = value;
}

void SmsVdp::DebugProcessMemoryAccessView()
{
	//Store memory access buffer in ppu tools to display in tilemap viewer
	SmsVdpTools* ppuTools = ((SmsVdpTools*)_emu->InternalGetDebugger()->GetPpuTools(CpuType::Sms));
	ppuTools->SetMemoryAccessData(_state.Scanline, _memAccess, _scanlineCount);
}

void SmsVdp::ProcessVramAccess()
{
	_memAccess[_state.Cycle] = SmsVdpMemAccess::CpuSlot;
	if(_writePending != SmsVdpWriteType::None) {
		ProcessVramWrite();
	} else if(_readPending) {
		_readPending = false;
		_state.VramBuffer = _videoRam[_state.AddressReg];
		_state.AddressReg = (_state.AddressReg + 1) & 0x3FFF;
	}
}

void SmsVdp::ProcessVramWrite()
{
	if(_writePending == SmsVdpWriteType::Palette) {
		_emu->ProcessPpuWrite<CpuType::Sms>(_state.AddressReg & 0x1F, _state.VramBuffer, MemoryType::SmsPaletteRam);
		WriteSmsPalette(_state.AddressReg & 0x1F, _state.VramBuffer);
	} else {
		_emu->ProcessPpuWrite<CpuType::Sms>(_state.AddressReg, _state.VramBuffer, MemoryType::SmsVideoRam);
		WriteVram(_state.AddressReg, _state.VramBuffer, SmsVdpMemAccess::CpuSlot);
	}
	_state.AddressReg = (_state.AddressReg + 1) & 0x3FFF;
	_writePending = SmsVdpWriteType::None;
}

int SmsVdp::GetVisiblePixelIndex()
{
	return _state.Cycle - SmsVdp::SmsVdpLeftBorder;
}

void SmsVdp::LoadBgTilesSms()
{
	uint16_t cycle = _state.Cycle;
	switch(cycle & 0x07) {
		case 0: {
			uint8_t x;
			if(_state.Scanline < 16 && _state.HorizontalScrollLock) {
				x = cycle;
			} else {
				x = (uint8_t)(cycle - (_state.HorizontalScrollLatch & 0xF8));
			}

			uint16_t y = cycle >= 192 && _state.VerticalScrollLock ? _state.Scanline : _bgOffsetY;
			uint16_t ntAddr = (_state.EffectiveNametableAddress + ((x / 8) + (y / 8) * 32) * 2) & _state.NametableAddressMask;
			uint16_t ntData = ReadVram(ntAddr, SmsVdpMemAccess::BgLoadTable) | (ReadVram(ntAddr + 1, SmsVdpMemAccess::BgLoadTable) << 8);

			bool vMirror = ntData & 0x400;
			uint16_t tileIndex = ntData & 0x1FF;
			uint8_t tileRow = vMirror ? 7 - (y & 0x07) : (y & 0x07);

			_bgPriority |= ((ntData & 0x1000) ? 0xFF : 0) << (16 - _pixelsAvailable);
			_bgPalette |= ((ntData & 0x800) ? 0xFF : 0) << (16 - _pixelsAvailable);
			_bgTileAddr = tileIndex * 32 + tileRow * 4;
			_bgHorizontalMirror = (ntData & 0x200);
			break;
		}

		case 2:
			if(cycle & 0x18) {
				ProcessSpriteEvaluation();
				ProcessSpriteEvaluation();
			}
			break;

		case 4: {
			uint16_t addr = _revision == SmsRevision::Sms1 ? ((_bgTileAddr & _state.ColorTableAddress) | (_bgTileAddr & 0x3F)) : _bgTileAddr;
			if(_bgHorizontalMirror) {
				_bgShifters[0] |= ReverseBitOrder(ReadVram(addr, SmsVdpMemAccess::BgLoadTile)) << (16 - _pixelsAvailable);
				_bgShifters[1] |= ReverseBitOrder(ReadVram(addr+1, SmsVdpMemAccess::BgLoadTile)) << (16 - _pixelsAvailable);
			} else {
				_bgShifters[0] |= ReadVram(addr, SmsVdpMemAccess::BgLoadTile) << (16 - _pixelsAvailable);
				_bgShifters[1] |= ReadVram(addr+1, SmsVdpMemAccess::BgLoadTile) << (16 - _pixelsAvailable);
			}
			break;
		}

		case 6: {
			uint16_t addr = _revision == SmsRevision::Sms1 ? ((_bgTileAddr & _state.BgPatternTableAddress) | (_bgTileAddr & 0x7FF)) : _bgTileAddr;
			if(_bgHorizontalMirror) {
				_bgShifters[2] |= ReverseBitOrder(ReadVram(addr+2, SmsVdpMemAccess::BgLoadTile)) << (16 - _pixelsAvailable);
				_bgShifters[3] |= ReverseBitOrder(ReadVram(addr+3, SmsVdpMemAccess::BgLoadTile)) << (16 - _pixelsAvailable);
			} else {
				_bgShifters[2] |= ReadVram(addr+2, SmsVdpMemAccess::BgLoadTile) << (16 - _pixelsAvailable);
				_bgShifters[3] |= ReadVram(addr+3, SmsVdpMemAccess::BgLoadTile) << (16 - _pixelsAvailable);
			}

			if(_disableBackground) {
				memset(_bgShifters, 0, sizeof(_bgShifters));
				_bgPriority = 0;
			}

			_pixelsAvailable += 8;
			break;
		}
	}
}

void SmsVdp::LoadBgTilesSg()
{
	if(_state.M1_Use224LineMode) {
		LoadBgTilesSgTextMode();
		return;
	}

	uint16_t cycle = _state.Cycle;
	switch(cycle & 0x07) {
		case 0: {
			uint8_t x = (uint8_t)_state.Cycle;
			uint16_t y = _state.Scanline;
			uint8_t tilemapRow = (y / 8);
			uint16_t ntAddr = _state.NametableAddress + ((x / 8) + tilemapRow * 32);

			uint8_t tileRow = (y & 0x07);
			_bgTileIndex = ReadVram(ntAddr, SmsVdpMemAccess::BgLoadTable);
			if(_state.M3_Use240LineMode) {
				//Mode 3 - "Multicolor"
				_bgTileAddr = (_state.BgPatternTableAddress & 0x3800) + (_bgTileIndex * 8) + (tilemapRow & 0x03) * 2 + (tileRow >= 4 ? 1 : 0);
			} else if(_state.M2_AllowHeightChange) {
				//Mode 2 - "Graphic 2"
				//Move to the next 256 tiles after every 8 tile rows
				_bgTileIndex += (tilemapRow & 0x18) << 5;
				uint16_t mask = ((_state.BgPatternTableAddress >> 3) | 0xFF) & 0x3FF;
				_bgTileAddr = (_state.BgPatternTableAddress & 0x2000) + ((_bgTileIndex & mask) * 8) + tileRow;
			} else {
				//Mode 0 - "Graphic 1"
				_bgTileAddr = (_state.BgPatternTableAddress & 0x3800) + (_bgTileIndex * 8) + tileRow;
			}
			break;
		}

		case 2:
			if(cycle & 0x18) {
				//sprite evaluation (read Y pos)
				ProcessSpriteEvaluation();
			}
			break;

		case 4:
			_bgPatternData = ReadVram(_bgTileAddr, SmsVdpMemAccess::BgLoadTile);
			break;

		case 6: {
			uint8_t color;
			if(_state.M3_Use240LineMode) {
				//Mode 3 - "Multicolor"
				color = _bgPatternData;
			} else if(_state.M2_AllowHeightChange) {
				//Mode 2 - "Graphic 2"
				uint16_t mask = ((_state.ColorTableAddress >> 3) | 0x07) & 0x3FF;
				uint8_t tileRow = (_state.Scanline & 0x07);
				uint16_t colorTableAddr = (_state.ColorTableAddress & 0x2000) | ((_bgTileIndex & mask) << 3) + tileRow;
				color = ReadVram(colorTableAddr, SmsVdpMemAccess::BgLoadTile);
			} else {
				//Mode 0 - "Graphic 1"
				uint16_t colorTableAddr = (_state.ColorTableAddress & 0x3FC0) | ((_bgTileIndex >> 3) & 0x1F);
				color = ReadVram(colorTableAddr, SmsVdpMemAccess::BgLoadTable);
			}

			for(int i = 0; i < 8; i++) {
				uint8_t pixelColor;
				if(_state.M3_Use240LineMode) {
					pixelColor = i < 4 ? (color >> 4) : (color & 0xF);
				} else {
					pixelColor = (_bgPatternData & 0x80) ? (color >> 4) : (color & 0xF);
				}
				_bgPatternData <<= 1;
				PushBgPixel(pixelColor, i);
			}

			if(_disableBackground) {
				memset(_bgShifters, 0, sizeof(_bgShifters));
				_bgPriority = 0;
			}

			_pixelsAvailable += 8;
			break;
		}
	}
}

void SmsVdp::LoadBgTilesSgTextMode()
{
	if(_state.Cycle >= 240) {
		return;
	} else if(_state.Cycle == 0) {
		_textModeStep = 0;
	}

	switch(_textModeStep++) {
		case 0: {
			uint8_t x = (uint8_t)_state.Cycle;
			uint16_t y = _state.Scanline;
			uint8_t tilemapRow = (y / 8);
			uint16_t ntAddr = _state.NametableAddress + ((x / 6) + tilemapRow * 40);
			uint8_t tileRow = (y & 0x07);
			_bgTileIndex = ReadVram(ntAddr, SmsVdpMemAccess::BgLoadTable);
			_bgTileAddr = (_state.BgPatternTableAddress & 0x3800) + (_bgTileIndex * 8) + tileRow;
			break;
		}

		case 2:
			_bgPatternData = ReadVram(_bgTileAddr, SmsVdpMemAccess::BgLoadTile);

			for(int i = 0; i < 6; i++) {
				uint8_t color = (_bgPatternData & 0x80) ? _state.TextColorIndex : _state.BackgroundColorIndex;
				_bgPatternData <<= 1;
				PushBgPixel(color, i);
			}

			if(_disableBackground) {
				memset(_bgShifters, 0, sizeof(_bgShifters));
				_bgPriority = 0;
			}

			if(_state.Cycle == 238) {
				for(int i = 0; i < 8; i++) {
					PushBgPixel(_state.BackgroundColorIndex, i);
				}
				_pixelsAvailable += 8;
			}

			_pixelsAvailable += 6;
			break;

		case 4:
			//CPU slot
			break;

		case 5:
			_textModeStep = 0;
			break;
	}
}

void SmsVdp::PushBgPixel(uint8_t color, int index)
{
	_bgShifters[0] |= (color & 0x01) << (23 - index - _pixelsAvailable);
	_bgShifters[1] |= ((color >> 1) & 0x01) << (23 - index - _pixelsAvailable);
	_bgShifters[2] |= ((color >> 2) & 0x01) << (23 - index - _pixelsAvailable);
	_bgShifters[3] |= ((color >> 3) & 0x01) << (23 - index - _pixelsAvailable);
}

uint8_t SmsVdp::ReverseBitOrder(uint8_t val)
{
	constexpr static uint8_t lut[16] = { 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF };
	return (lut[val & 0xF] << 4) | lut[val >> 4];
}

void SmsVdp::DrawPixel()
{
	_currentOutputBuffer[_state.Scanline * 256 + GetVisiblePixelIndex()] = GetPixelColor();
	if(_needCramDot) {
		_currentOutputBuffer[_state.Scanline * 256 + GetVisiblePixelIndex()] = _cramDotColor;
	}
	_bgShifters[0] <<= 1;
	_bgShifters[1] <<= 1;
	_bgShifters[2] <<= 1;
	_bgShifters[3] <<= 1;
	_bgPriority <<= 1;
	_bgPalette <<= 1;
	_pixelsAvailable--;
}

void SmsVdp::ProcessScanlineEvents()
{
	switch(_state.Cycle) {
		case 313:
			_state.HorizontalScrollLatch = _state.HorizontalScroll;
			break;

		case 315:
			//vertical blank irq (on last visible scanline)
			if(_state.Scanline == _state.VisibleScanlineCount) {
				_state.VerticalBlankIrqPending = true;
				UpdateIrqState();
			}

			_state.VCounter++;
			if(_state.VCounter == _scanlineCount - 1) {
				if(_model == SmsModel::Sms) {
					_cpu->SetNmiLevel(_controlManager->IsPausePressed());
				}
			} else if(_state.VCounter >= _scanlineCount) {
				_state.VCounter = 0;
			}

			if(_spriteOverflowPending) {
				_spriteOverflowPending = false;
				if(_state.Scanline < _state.VisibleScanlineCount || _state.VCounter == 0) {
					//Don't trigger sprite overflow from the sprite eval fetches that run during vblank
					_state.SpriteOverflow = true;
				}
			}
			break;

		case 316:
			//horizontal irq
			if(_state.Scanline <= 191 || _state.Scanline == _scanlineCount - 1) {
				if(_state.ScanlineCounterLatch-- == 0) {
					_state.ScanlineCounterLatch = _state.ScanlineCounter;
					_state.ScanlineIrqPending = true;
					UpdateIrqState();
				}
			} else {
				_state.ScanlineCounterLatch = _state.ScanlineCounter;
			}
			break;
	}
}

void SmsVdp::ProcessEndOfScanline()
{
	if(_emu->IsDebugging()) {
		DebugProcessMemoryAccessView();
	}
	memset(_memAccess, 0, sizeof(_memAccess));

	//Set to cycle 0 temporarily (for event viewer, etc.), and then back to -1 at the end because the cycle gets incremented after this in Exec()
	_state.Cycle = 0;
	_state.Scanline++;
	if(_state.Scanline == _state.VisibleScanlineCount) {
		_emu->ProcessEvent(EventType::EndFrame, CpuType::Sms);
		_state.FrameCount++;

		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

		RenderedFrame frame(_currentOutputBuffer, 256, 240, 1.0, _state.FrameCount, _console->GetControlManager()->GetPortStates());
		bool rewinding = _emu->GetRewindManager()->IsRewinding();
		_emu->GetVideoDecoder()->UpdateFrame(frame, rewinding, rewinding);

		UpdateConfig();

		_console->ProcessEndOfFrame();
		_emu->ProcessEndOfFrame();
	} else if(_state.Scanline >= _scanlineCount) {
		_state.Scanline = 0;
		_state.VerticalScrollLatch = _state.VerticalScroll;
		_emu->ProcessEvent(EventType::StartFrame, CpuType::Sms);
		_currentOutputBuffer = _currentOutputBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
	}

	_bgShifters[0] = 0;
	_bgShifters[1] = 0;
	_bgShifters[2] = 0;
	_bgShifters[3] = 0;
	_bgPriority = 0;
	_bgPalette = 0;

	uint8_t borderWidth = 0;
	if(_state.UseMode4) {
		if(_state.Scanline < 16 && _state.HorizontalScrollLock) {
			borderWidth = 0;
		} else {
			borderWidth = (_state.HorizontalScrollLatch & 0x07);
		}
	} else {
		//Add 8 pixels of border on the left when in text mode
		borderWidth = _state.M1_Use224LineMode ? 8 : 0;
	}

	_pixelsAvailable = 0;
	for(int i = 0; i < borderWidth; i++) {
		PushBgPixel(_state.BackgroundColorIndex, i);
		_bgPalette |= 0x800000 >> i;
	}
	_pixelsAvailable = borderWidth;

	//Mask feature only works in mode 4
	bool maskFirstColumn = _state.UseMode4 ? _state.MaskFirstColumn : false;

	_minDrawCycle = _state.RenderingEnabled ? (maskFirstColumn ? (SmsVdp::SmsVdpLeftBorder + 8) : SmsVdp::SmsVdpLeftBorder) : 342;
	_bgOffsetY = _state.Scanline + _state.VerticalScrollLatch;
	if(_bgOffsetY >= _state.NametableHeight) {
		_bgOffsetY -= _state.NametableHeight;
	}

	_state.Cycle = -1;
}

void SmsVdp::ProcessSpriteEvaluation()
{
	if(!_state.UseMode4 && _state.M1_Use224LineMode) {
		//No sprites in text mode
		return;
	}

	//Fetch sprite 0 after a sprite with Y=$D0 is found
	//TODOSMS what sprite is actually fetched by hardware?
	uint8_t i = _evalCounter == 0xFF ? 0 : _evalCounter;

	uint16_t spriteAddr;
	uint8_t sprY;
	if(_state.UseMode4) {
		spriteAddr = _state.SpriteTableAddress & 0x3F00;
		sprY = ReadVram(spriteAddr + i, SmsVdpMemAccess::SpriteEval) + 17; //+17 to force wraparound to the top when sprite is at Y >= 241 (8x16 sprites)
	} else {
		spriteAddr = _state.SpriteTableAddress;
		sprY = ReadVram(spriteAddr + i * 4, SmsVdpMemAccess::SpriteEval) + 17; //+17 to force wraparound to the top when sprite is at Y >= 241 (8x16 sprites)
	}

	if(_state.VisibleScanlineCount == 192 && sprY == 0xD0 + 17) {
		//Don't check/draw any more sprites
		_evalCounter = 0xFF;
		return;
	}

	if(_evalCounter == 0xFF) {
		//Sprite evaluation was cancelled by a sprite with Y=$D0)
		return;
	}

	_evalCounter++;

	uint8_t spriteHeight = (_state.UseLargeSprites ? 16 : 8) << (uint8_t)_state.EnableDoubleSpriteSize;
	uint16_t scanline = (_state.VCounter + 17);
	if(scanline >= _scanlineCount) {
		scanline -= _scanlineCount;
	}

	if(scanline >= sprY && scanline < sprY + spriteHeight) {
		if(!_state.UseMode4) {
			if(_inRangeSpriteCount >= 4) {
				if(!_spriteOverflowPending) {
					_state.SpriteOverflowIndex = i;
				}
				_spriteOverflowPending = true;
				if(!_removeSpriteLimit) {
					return;
				}
			}
		} else {
			if(_inRangeSpriteCount >= 8) {
				_spriteOverflowPending = true;
				if(!_removeSpriteLimit) {
					return;
				}
			}
			_spriteShifters[_inRangeSpriteCount].SpriteRow = (scanline - sprY) >> (uint8_t)_state.EnableDoubleSpriteSize;
		}

		_inRangeSprites[_inRangeSpriteCount] = i;
		_inRangeSpriteCount++;
	}
}

uint16_t SmsVdp::GetSmsSpriteTileAddr(uint8_t sprTileIndex, uint8_t spriteRow, uint8_t i)
{
	if(_state.UseLargeSprites) {
		sprTileIndex &= ~0x01;
	}

	return (_state.SpritePatternSelector & 0x2000) | (sprTileIndex << 5) | (spriteRow << 2);
}

void SmsVdp::LoadSpriteTilesSms()
{
	uint16_t spriteAddr = _state.SpriteTableAddress & 0x3F00;

	//Cycles 264 to 325
	uint16_t cycle = _state.Cycle - 264;
	switch(cycle) {
		case 0: case 12:
		case 34: case 46: {
			//Load sprite N X value
			if(cycle == 0) {
				_spriteCount = 0;
			}

			_spriteShifters[_spriteCount].HardwareSprite = true;
			
			uint16_t loadAddr = spriteAddr + 0x80 + _inRangeSprites[_spriteCount] * 2;
			_spriteShifters[_spriteCount].SpriteX = ReadVram(loadAddr, SmsVdpMemAccess::SpriteLoadTable);
			uint8_t sprTileIndex = ReadVram(loadAddr + 1, SmsVdpMemAccess::SpriteLoadTable);
			_spriteShifters[_spriteCount].TileAddr = GetSmsSpriteTileAddr(sprTileIndex, _spriteShifters[_spriteCount].SpriteRow, _inRangeSprites[_spriteCount]);
			break;
		}

		case 2: case 14:
		case 36: case 48: {
			//Load sprite N+1 X value
			_spriteShifters[_spriteCount + 1].HardwareSprite = true;

			uint16_t loadAddr = spriteAddr + 0x80 + _inRangeSprites[_spriteCount + 1] * 2;
			_spriteShifters[_spriteCount + 1].SpriteX = ReadVram(loadAddr, SmsVdpMemAccess::SpriteLoadTable);
			uint8_t sprTileIndex = ReadVram(loadAddr + 1, SmsVdpMemAccess::SpriteLoadTable);
			_spriteShifters[_spriteCount + 1].TileAddr = GetSmsSpriteTileAddr(sprTileIndex, _spriteShifters[_spriteCount + 1].SpriteRow, _inRangeSprites[_spriteCount + 1]);
			break;
		}

		case 4: case 16:
		case 38: case 50:
			//Load sprite N tile (1st word)
			_spriteShifters[_spriteCount].TileData[0] = ReadVram(_spriteShifters[_spriteCount].TileAddr, SmsVdpMemAccess::SpriteLoadTile);
			_spriteShifters[_spriteCount].TileData[1] = ReadVram(_spriteShifters[_spriteCount].TileAddr + 1, SmsVdpMemAccess::SpriteLoadTile);
			break;

		case 6: case 18:
		case 40: case 52:
			//Load sprite N tile (2nd word)
			_spriteShifters[_spriteCount].TileData[2] = ReadVram(_spriteShifters[_spriteCount].TileAddr + 2, SmsVdpMemAccess::SpriteLoadTile);
			_spriteShifters[_spriteCount].TileData[3] = ReadVram(_spriteShifters[_spriteCount].TileAddr + 3, SmsVdpMemAccess::SpriteLoadTile);
			if(_state.ShiftSpritesLeft) {
				//Shift all sprites to the left by 8 pixels
				ShiftSprite(_spriteCount);
			}
			break;

		case 8: case 20:
		case 42: case 54:
			//Load sprite N+1 tile (1st word)
			_spriteShifters[_spriteCount + 1].TileData[0] = ReadVram(_spriteShifters[_spriteCount + 1].TileAddr, SmsVdpMemAccess::SpriteLoadTile);
			_spriteShifters[_spriteCount + 1].TileData[1] = ReadVram(_spriteShifters[_spriteCount + 1].TileAddr + 1, SmsVdpMemAccess::SpriteLoadTile);
			break;

		case 10: case 22:
		case 44: case 56:
			//Load sprite N+1 tile (2nd word)
			_spriteShifters[_spriteCount + 1].TileData[2] = ReadVram(_spriteShifters[_spriteCount + 1].TileAddr + 2, SmsVdpMemAccess::SpriteLoadTile);
			_spriteShifters[_spriteCount + 1].TileData[3] = ReadVram(_spriteShifters[_spriteCount + 1].TileAddr + 3, SmsVdpMemAccess::SpriteLoadTile);
			if(_state.ShiftSpritesLeft) {
				//Shift all sprites to the left by 8 pixels
				ShiftSprite(_spriteCount + 1);
			}

			_spriteCount += 2;
			if(cycle == 56) {
				_spriteCount = _inRangeSpriteCount;
				if(_inRangeSpriteCount > 8 && _removeSpriteLimit) {
					LoadExtraSpritesSms();
				}
			}
			break;

		case 24: case 26: case 28: case 30: case 32: case 58: case 60:
			//293, 295, 297, 299, 301, 327, 329
			//external access slots
			break;
	}
}

void SmsVdp::LoadExtraSpritesSms()
{
	uint16_t spriteAddr = _state.SpriteTableAddress & 0x3F00;
	for(int i = 8; i < _inRangeSpriteCount; i++) {
		_spriteShifters[i].SpriteX = _videoRam[spriteAddr + 0x80 + _inRangeSprites[i] * 2];
		
		uint8_t sprTileIndex = _videoRam[spriteAddr + 0x80 + _inRangeSprites[i] * 2 + 1];
		uint16_t sprTileAddr = GetSmsSpriteTileAddr(sprTileIndex, _spriteShifters[i].SpriteRow, _inRangeSprites[i]);
		_spriteShifters[i].TileData[0] = _videoRam[sprTileAddr];
		_spriteShifters[i].TileData[1] = _videoRam[sprTileAddr + 1];
		_spriteShifters[i].TileData[2] = _videoRam[sprTileAddr + 2];
		_spriteShifters[i].TileData[3] = _videoRam[sprTileAddr + 3];
		_spriteShifters[i].HardwareSprite = false;
		_spriteCount++;

		if(_state.ShiftSpritesLeft) {
			//Shift all sprites to the left by 8 pixels
			ShiftSprite(i);
		}
	}
}

void SmsVdp::LoadSpriteTilesSg()
{
	if(_state.M1_Use224LineMode) {
		//No sprites in text mode
		return;
	}

	uint16_t spriteAddr = _state.SpriteTableAddress;

	//Cycles 264 to 325
	uint16_t cycle = _state.Cycle - 264;
	switch(cycle) {
		case 0: case 12:
		case 34: case 46: {
			//Sprite Y value
			if(cycle == 0) {
				_spriteCount = 0;
				_spriteIndex = 0;
				_inRangeSpriteIndex = 0;
			}

			uint8_t sprY = ReadVram(spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4, SmsVdpMemAccess::SpriteLoadTable) + 17;
			uint16_t scanline = (_state.VCounter + 17);
			if(scanline >= _scanlineCount) {
				scanline -= _scanlineCount;
			}

			//Keep lowest bits only - Y may have changed and be out-of-range between sprite eval and sprite fetching
			uint8_t row = (scanline - sprY) & ((_state.UseLargeSprites ? 0x0F : 0x07) << (uint8_t)_state.EnableDoubleSpriteSize);
			_spriteShifters[_spriteIndex].SpriteRow = row >> (uint8_t)_state.EnableDoubleSpriteSize;
			_spriteShifters[_spriteIndex].HardwareSprite = true;
			break;
		}

		case 2: case 14:
		case 36: case 48:
			//Sprite X value
			_spriteShifters[_spriteIndex].SpriteX = ReadVram(spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4 + 1, SmsVdpMemAccess::SpriteLoadTable);
			break;

		case 4: case 16:
		case 38: case 50: {
			//Sprite tile index
			uint16_t sprTileIndex = ReadVram(spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4 + 2, SmsVdpMemAccess::SpriteLoadTable);
			if(_state.UseLargeSprites) {
				sprTileIndex &= ~0x03;
			}

			_spriteShifters[_spriteIndex].TileAddr = _state.SpritePatternSelector | (sprTileIndex << 3) | _spriteShifters[_spriteIndex].SpriteRow;
			break;
		}

		case 6: case 18:
		case 40: case 52:
			//Sprite attributes
			_spriteShifters[_spriteIndex].TileData[1] = ReadVram(spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4 + 3, SmsVdpMemAccess::SpriteLoadTable);
			break;

		case 8: case 20:
		case 42: case 54:
			//Sprite tile data (first byte)
			_spriteShifters[_spriteIndex].TileData[0] = ReadVram(_spriteShifters[_spriteIndex].TileAddr, SmsVdpMemAccess::SpriteLoadTile);
			break;

		case 10: case 22:
		case 44: case 56: {
			//Sprite tile data (second byte - for large sprites only)
			bool shiftSprite = _spriteShifters[_spriteIndex].TileData[1] & 0x80;
			_spriteShifters[_spriteIndex].TileData[1] &= 0x0F; //sprite color
			int16_t xPos = _spriteShifters[_spriteIndex].SpriteX;
			if(shiftSprite) {
				//Shift all sprites to the left by 8 pixels
				ShiftSpriteSg(_spriteIndex);
			}
			_spriteIndex++;

			if(_state.UseLargeSprites) {
				_spriteShifters[_spriteIndex] = _spriteShifters[_spriteIndex - 1];
				_spriteShifters[_spriteIndex].TileData[0] = ReadVram(_spriteShifters[_spriteIndex].TileAddr + 16, SmsVdpMemAccess::SpriteLoadTile);
				_spriteShifters[_spriteIndex].SpriteX = xPos + (_state.EnableDoubleSpriteSize ? 16 : 8);
				if(shiftSprite) {
					//Shift all sprites to the left by 8 pixels
					ShiftSpriteSg(_spriteIndex);
				}
				_spriteIndex++;
			}
			
			_inRangeSpriteIndex++;
			if(_inRangeSpriteCount > 0) {
				_inRangeSpriteCount--;
				_spriteCount += _state.UseLargeSprites ? 2 : 1;
			}

			if(cycle == 56 && _inRangeSpriteCount > 0 && _removeSpriteLimit) {
				LoadExtraSpritesSg();
			}
			break;
		}

		case 24: case 26: case 28: case 30: case 32: case 58: case 60:
			//293, 295, 297, 299, 301, 327, 329
			//external access slots
			break;
	}
}

void SmsVdp::LoadExtraSpritesSg()
{
	uint16_t spriteAddr = _state.SpriteTableAddress;
	uint16_t scanline = (_state.Scanline + 17);
	if(scanline >= _scanlineCount) {
		scanline -= _scanlineCount;
	}

	for(int i = 0; i < _inRangeSpriteCount; i++) {
		_spriteShifters[_spriteIndex].SpriteX = _videoRam[spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4 + 1];
		uint8_t sprY = _videoRam[spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4] + 17;
		_spriteShifters[_spriteIndex].SpriteRow = (scanline - sprY) >> (uint8_t)_state.EnableDoubleSpriteSize;

		uint16_t sprTileIndex = _videoRam[spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4 + 2];
		if(_state.UseLargeSprites) {
			sprTileIndex &= ~0x03;
		}
		
		_spriteShifters[_spriteIndex].HardwareSprite = false;
		_spriteShifters[_spriteIndex].TileAddr = _state.SpritePatternSelector | (sprTileIndex << 3) | _spriteShifters[_spriteIndex].SpriteRow;
		_spriteShifters[_spriteIndex].TileData[1] = _videoRam[spriteAddr + _inRangeSprites[_inRangeSpriteIndex] * 4 + 3];
		_spriteShifters[_spriteIndex].TileData[0] = _videoRam[_spriteShifters[_spriteIndex].TileAddr];

		bool shiftSprite = _spriteShifters[_spriteIndex].TileData[1] & 0x80;
		_spriteShifters[_spriteIndex].TileData[1] &= 0x0F; //sprite color
		int16_t xPos = _spriteShifters[_spriteIndex].SpriteX;
		if(shiftSprite) {
			//Shift all sprites to the left by 8 pixels
			ShiftSpriteSg(_spriteIndex);
		}

		_spriteCount++;
		_spriteIndex++;
		_inRangeSpriteIndex++;

		if(_state.UseLargeSprites) {
			_spriteShifters[_spriteIndex] = _spriteShifters[_spriteIndex - 1];
			_spriteShifters[_spriteIndex].TileData[0] = _videoRam[_spriteShifters[_spriteIndex].TileAddr + 16];
			_spriteShifters[_spriteIndex].SpriteX = xPos + (_state.EnableDoubleSpriteSize ? 16 : 8);
			if(shiftSprite) {
				//Shift all sprites to the left by 8 pixels
				ShiftSpriteSg(_spriteIndex);
			}

			_spriteCount++;
			_spriteIndex++;
		}
	}
}

void SmsVdp::ShiftSprite(uint8_t sprIndex)
{
	_spriteShifters[sprIndex].SpriteX -= 8;
	int16_t x = _spriteShifters[sprIndex].SpriteX;
	if(x < 0) {
		_spriteShifters[sprIndex].TileData[0] <<= -x;
		_spriteShifters[sprIndex].TileData[1] <<= -x;
		_spriteShifters[sprIndex].TileData[2] <<= -x;
		_spriteShifters[sprIndex].TileData[3] <<= -x;
	}
}

void SmsVdp::ShiftSpriteSg(uint8_t sprIndex)
{
	_spriteShifters[sprIndex].SpriteX -= 32;
	int16_t x = _spriteShifters[sprIndex].SpriteX;
	if(x < 0) {
		_spriteShifters[sprIndex].TileData[0] <<= -x;
	}
}

bool SmsVdp::IsZoomedSpriteAllowed(int spriteIndex)
{
	//On all revisions, all sprites are zoomed vertically, but the horizontal zoom effect is bugged on SMS1.
	//On SMS2/GG, all 8 sprites are zoomed as expected.
	//On SMS1, only up to 4 sprites are zoomed horizontally. Specifically, only "[scanline sprite count] - 4" sprites are zoomed, e.g:
	// -If the scanline has 4 sprites, none will be zoomed.
	// -If the scanline has 5 sprites, only 1 sprite will be zoomed.
	// -If the scanline has 8 sprites, 4 sprites will be zoomed.
	//Which sprites get zoomed is based on their draw priority (i.e the earlier entries in sprite ram get zoomed first)
	//See thread/test rom: https://www.smspower.org/forums/19189-SMS1DoubleSizeSpritesBitTest
	return _state.EnableDoubleSpriteSize && (!_state.UseMode4 || spriteIndex < ((int)_spriteCount - 4) || _revision != SmsRevision::Sms1);
}

uint16_t SmsVdp::GetPixelColor()
{
	if(!_state.RenderingEnabled || _state.Cycle < SmsVdp::SmsVdpLeftBorder) {
		return _internalPaletteRam[0x10 | _state.BackgroundColorIndex];
	}

	bool spriteDrawn = false;
	uint8_t spritePixelColor = 0;
	uint16_t xPos = GetVisiblePixelIndex();
	for(int i = 0; i < _spriteCount; i++) {
		if(xPos >= _spriteShifters[i].SpriteX && xPos < _spriteShifters[i].SpriteX + (8 << (uint8_t)IsZoomedSpriteAllowed(i))) {
			if(_state.UseMode4) {
				uint8_t sprColor = (
					((_spriteShifters[i].TileData[0] >> 7) & 0x01) |
					((_spriteShifters[i].TileData[1] >> 6) & 0x02) |
					((_spriteShifters[i].TileData[2] >> 5) & 0x04) |
					((_spriteShifters[i].TileData[3] >> 4) & 0x08)
				);

				if(!IsZoomedSpriteAllowed(i) || ((_spriteShifters[i].SpriteX - xPos) & 0x01)) {
					_spriteShifters[i].TileData[0] <<= 1;
					_spriteShifters[i].TileData[1] <<= 1;
					_spriteShifters[i].TileData[2] <<= 1;
					_spriteShifters[i].TileData[3] <<= 1;
				}

				if(sprColor != 0) {
					if(spriteDrawn) {
						_state.SpriteCollision |= _spriteShifters[i].HardwareSprite;
						continue;
					} else {
						spritePixelColor = sprColor;
						spriteDrawn = true;
					}
				}
			} else {
				uint8_t sprColor = ((_spriteShifters[i].TileData[0] >> 7) & 0x01);
				if(!_state.EnableDoubleSpriteSize || ((_spriteShifters[i].SpriteX - xPos) & 0x01)) {
					_spriteShifters[i].TileData[0] <<= 1;
				}

				if(sprColor != 0) {
					if(spriteDrawn) {
						_state.SpriteCollision |= _spriteShifters[i].HardwareSprite;
						continue;
					} else {
						uint8_t spritePalette = _spriteShifters[i].TileData[1];
						if(spritePalette != 0) {
							spritePixelColor = spritePalette;
							spriteDrawn = true;
						}
					}
				}
			}
		}
	}

	if(_state.Cycle < _minDrawCycle) {
		return _internalPaletteRam[0x10 | _state.BackgroundColorIndex];
	}

	uint8_t color = (
		((_bgShifters[0] >> 23) & 0x01) |
		((_bgShifters[1] >> 22) & 0x02) |
		((_bgShifters[2] >> 21) & 0x04) |
		((_bgShifters[3] >> 20) & 0x08)
	);

	bool highPriority = (_bgPriority & 0x800000);
	if(!spriteDrawn || (highPriority && color != 0) || _disableSprites) {
		uint8_t paletteOffset = (_bgPalette & 0x800000) ? 0x10 : 0;
		if(_state.UseMode4) {
			return _internalPaletteRam[paletteOffset + color];
		} else {
			return _activeSgPalette[color == 0 ? _state.BackgroundColorIndex : color];
		}
	}

	if(_state.UseMode4) {
		return _internalPaletteRam[0x10 + spritePixelColor];
	} else {
		return _activeSgPalette[spritePixelColor];
	}
}

void SmsVdp::WriteRegister(uint8_t reg, uint8_t value)
{
	if(reg >= 8 && _model == SmsModel::ColecoVision) {
		//These registers don't exist on the TMS9918A
		return;
	}

	switch(reg) {
		case 0:
			_state.SyncDisabled = (value & 0x01) != 0; //TODOSMS not implemented
			_state.M2_AllowHeightChange = (value & 0x02) != 0;

			if(_model == SmsModel::Sms || _model == SmsModel::GameGear) {
				_state.UseMode4 = (value & 0x04) != 0;
			} else {
				_state.UseMode4 = false;
			}

			_state.ShiftSpritesLeft = (value & 0x08) != 0;
			_state.EnableScanlineIrq = (value & 0x10) != 0;
			_state.MaskFirstColumn = (value & 0x20) != 0;
			_state.HorizontalScrollLock = (value & 0x40) != 0;
			_state.VerticalScrollLock = (value & 0x80) != 0;
			UpdateIrqState();
			break;

		case 1:
			_state.EnableDoubleSpriteSize = (value & 0x01) != 0;
			_state.UseLargeSprites = (value & 0x02) != 0;
			//bit 2 has no effect?
			_state.M3_Use240LineMode = (value & 0x08) != 0;
			_state.M1_Use224LineMode = (value & 0x10) != 0;
			_state.EnableVerticalBlankIrq = (value & 0x20) != 0;
			_state.RenderingEnabled = (value & 0x40) != 0;
			_state.Sg16KVramMode = (value & 0x80) != 0; //TODOSMS not implemented
			UpdateIrqState();
			UpdateDisplayMode();
			break;

		case 2:
			_state.NametableAddress = (value & 0x0F) << 10;
			UpdateDisplayMode();
			break;

		case 3: _state.ColorTableAddress = value << 6; break;
		case 4: _state.BgPatternTableAddress = (value & 0x07) << 11; break;
		case 5: _state.SpriteTableAddress = (value & 0x7F) << 7; break;
		case 6: _state.SpritePatternSelector = (value & 0x07) << 11; break;
		case 7:
			_state.TextColorIndex = (value >> 4) & 0x0F;
			_state.BackgroundColorIndex = value & 0x0F;
			break;

		case 8: _state.HorizontalScroll = value; break;
		case 9: _state.VerticalScroll = value; break;
		case 10: _state.ScanlineCounter = value; break;
	}
}

void SmsVdp::WriteSmsPalette(uint8_t addr, uint8_t value)
{
	_paletteRam[addr] = value & 0x3F;

	uint16_t rgbColor = ColorUtilities::Rgb222To555(value & 0x3F);
	_internalPaletteRam[addr] = rgbColor;

	_needCramDot = true;
	_cramDotColor = rgbColor;
}

void SmsVdp::WriteGameGearPalette(uint8_t addr, uint16_t value)
{
	_paletteRam[addr] = value & 0xFF;
	_paletteRam[addr + 1] = (value >> 8) & 0x0F;
	
	uint16_t rgbColor = ColorUtilities::Rgb444To555(value & 0xFFF);
	_internalPaletteRam[addr >> 1] = rgbColor;

	_needCramDot = true;
	_cramDotColor = rgbColor;
}

void SmsVdp::WritePort(uint8_t port, uint8_t value)
{
	if(port & 1) {
		//Control port
		if(_state.ControlPortMsbToggle) {
			_state.CodeReg = (value >> 6);
			_state.AddressReg = (_state.AddressReg & 0xFF) | ((value & 0x3F) << 8);

			if(_state.CodeReg == 0) {
				//"When the second byte is written, the value at the VRAM location specified
				//by the address register is retrieved and stored in a buffer, and the address
				//register is incremented"
				_readPending = true;
			} else if(_state.CodeReg == 2) {
				WriteRegister((_state.AddressReg & 0xF00) >> 8, _state.AddressReg & 0xFF);
			}
		} else {
			_state.AddressReg = (_state.AddressReg & 0x3F00) | value;
		}
		_state.ControlPortMsbToggle = !_state.ControlPortMsbToggle;
	} else {
		//Data port
		_state.ControlPortMsbToggle = false;

		//"An additional quirk is that writing to the data port will also load the buffer with the value written."
		_state.VramBuffer = value;

		//TODOSMS break option for write while previous write is pending?
		if(_state.CodeReg == 3) {
			//Palette write
			if(_model == SmsModel::GameGear) {
				//TODOSMS - does this also need an external cpu access slot on GG?
				if(_state.AddressReg & 0x01) {
					uint8_t addr = (_state.AddressReg & 0x3E);
					_emu->ProcessPpuWrite<CpuType::Sms>(addr, _state.PaletteLatch, MemoryType::SmsPaletteRam);
					_emu->ProcessPpuWrite<CpuType::Sms>(addr + 1, value, MemoryType::SmsPaletteRam);
					WriteGameGearPalette(addr, ((value & 0x0F) << 8) | _state.PaletteLatch);
				} else {
					_state.PaletteLatch = value;
				}
				_state.AddressReg = (_state.AddressReg + 1) & 0x3FFF;
			} else {
				_writePending = SmsVdpWriteType::Palette;
			}
		} else {
			//VRAM write
			_writePending = SmsVdpWriteType::Vram;
		}
	}
}

uint8_t SmsVdp::ReadPort(uint8_t port)
{
	switch(port & 0xC1) {
		case 0x40: return ReadVerticalCounter(); //V counter
		case 0x41:
			//H counter
			if(_latchRequest) {
				//Used by light phaser
				InternalLatchHorizontalCounter(_latchPos);
			}
			return _state.HCounterLatch;

		case 0x80: {
			//Data Port
			//"Any subsequent data port read will return the value in the buffer.
			//The value stored at the current VRAM location specified by the address
			//register is then copied to the buffer, and the address register is incremented"
			_state.ControlPortMsbToggle = false;
			uint8_t value = _state.VramBuffer;
			_state.VramBuffer = _videoRam[_state.AddressReg];
			_state.AddressReg = (_state.AddressReg + 1) & 0x3FFF;
			return value;
		}

		case 0x81: {
			//Control Port
			uint8_t value = (
				(_state.VerticalBlankIrqPending ? 0x80 : 0) |
				(_state.SpriteOverflow ? 0x40 : 0) |
				(_state.SpriteCollision ? 0x20 : 0) |
				(_state.UseMode4 ? (_memoryManager->GetOpenBus() & 0x1F) : _state.SpriteOverflowIndex)
			);
			_state.VerticalBlankIrqPending = false;
			_state.ScanlineIrqPending = false;
			_state.SpriteOverflow = false;
			_state.SpriteCollision = false;
			_state.ControlPortMsbToggle = false;
			_state.SpriteOverflowIndex = 0;
			UpdateIrqState();
			return value;
		}
	}
	return 0;
}

uint8_t SmsVdp::PeekPort(uint8_t port)
{
	switch(port & 0xC1) {
		case 0x40: return ReadVerticalCounter(); //V counter
		case 0x41: return _state.HCounterLatch; //H counter
		case 0x80: return _state.VramBuffer;

		case 0x81:
			//Control Port
			return (
				(_state.VerticalBlankIrqPending ? 0x80 : 0) |
				(_state.SpriteOverflow ? 0x40 : 0) |
				(_state.SpriteCollision ? 0x20 : 0) |
				(_state.UseMode4 ? (_memoryManager->GetOpenBus() & 0x1F) : _state.SpriteOverflowIndex)
			);
	}
	return 0;
}

void SmsVdp::SetLocationLatchRequest(uint8_t x)
{
	//Used by light phaser
	_latchRequest = true;
	_latchPos = x;
}

void SmsVdp::InternalLatchHorizontalCounter(uint16_t cycle)
{
	cycle += 3;
	if(cycle >= 342) {
		cycle -= 342;
	}

	uint16_t counter = cycle >> 1;
	_state.HCounterLatch = counter > 0x93 ? (counter + 0x55) : counter;
	_latchRequest = false;
}

void SmsVdp::LatchHorizontalCounter()
{
	InternalLatchHorizontalCounter(_state.Cycle);
}

void SmsVdp::SetRegion(ConsoleRegion region)
{
	if(region == ConsoleRegion::Pal) {
		_scanlineCount = 313;
	} else {
		_scanlineCount = 262;
	}
	_region = region;
}

void SmsVdp::DebugSendFrame()
{
	int offset = std::max(0, std::min((int)GetVisiblePixelIndex(), 256)) + (_state.Scanline * 256);
	int pixelsToClear = 256*240 - offset;
	if(pixelsToClear > 0) {
		memset(_currentOutputBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
	}
	
	RenderedFrame frame(_currentOutputBuffer, 256, 240, 1.0, _state.FrameCount);
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

uint32_t SmsVdp::GetPixelBrightness(uint8_t x, uint8_t y)
{
	//Used by light phaser, gives a rough approximation of the brightness level of the specific pixel
	uint32_t argbColor = ColorUtilities::Rgb555ToArgb(_currentOutputBuffer[y << 8 | x]);
	return (argbColor & 0xFF) + ((argbColor >> 8) & 0xFF) + ((argbColor >> 16) & 0xFF);
}

int SmsVdp::GetViewportYOffset()
{
	switch(_state.VisibleScanlineCount) {
		default: case 192: return 24;
		case 224: return 8;
		case 240: return 0;
	}
}

void SmsVdp::DebugWritePalette(uint8_t addr, uint8_t value)
{
	if(_model == SmsModel::GameGear) {
		if(addr & 0x01) {
			value &= 0x0F;
		}
		_paletteRam[addr] = value;
		_internalPaletteRam[addr >> 1] = ColorUtilities::Rgb444To555(_paletteRam[addr & ~0x01] | (_paletteRam[addr | 0x01] << 8));
	} else {
		_paletteRam[addr] = value & 0x3F;
		_internalPaletteRam[addr] = ColorUtilities::Rgb222To555(value);
	}
}

void SmsVdp::InitSmsPostBiosState()
{
	//mimic post-bios initial state when no bios is used
	_state.RenderingEnabled = true;
	_state.UseMode4 = true;
	_state.M2_AllowHeightChange = true;
	_state.SpriteOverflow = true;
	_state.VerticalBlankIrqPending = true;
	_state.CodeReg = 1;
	_state.AddressReg = 0x3D6A;
	_state.EnableScanlineIrq = true;
	_state.MaskFirstColumn = true;
	_state.EnableVerticalBlankIrq = true;
	_state.NametableAddress = 0x3C00;
	_state.SpriteTableAddress = 0x3F00;
	_state.SpritePatternSelector = 0x1800;
	_state.ScanlineCounter = 0xFF;
	_state.ScanlineCounterLatch = 0xFC;

	constexpr uint8_t biosPalRam[0x20] = {
		0x00, 0x3F, 0x3E, 0x3F, 0x30, 0x30, 0x38, 0x3F,
		0x37, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x03, 0x30, 0x0F, 0x07, 0x16, 0x3F, 0x02,
		0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	memcpy(_paletteRam, biosPalRam, 0x20);
	memset(_videoRam, 0, 0x4000);
}

void SmsVdp::InitGgPowerOnState()
{
	//This is needed for some games to work properly (e.g The Terminator's first screen is broken because it doesn't init the nametable address)
	uint8_t regs[16] = { 0x06, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00 };
	for(int i = 0; i < 16; i++) {
		WriteRegister(i, regs[i]);
	}
}

void SmsVdp::Serialize(Serializer& s)
{
	SVArray(_videoRam, 0x4000);
	SVArray(_paletteRam, 0x40);
	SVArray(_internalPaletteRam, 0x20);

	SV(_state.FrameCount);
	SV(_state.Cycle);
	SV(_state.Scanline);
	SV(_state.VCounter);
	SV(_state.AddressReg);
	SV(_state.CodeReg);
	SV(_state.ControlPortMsbToggle);
	SV(_state.VramBuffer);
	SV(_state.HCounterLatch);
	SV(_state.VerticalBlankIrqPending);
	SV(_state.ScanlineIrqPending);
	SV(_state.SpriteOverflow);
	SV(_state.SpriteCollision);
	SV(_state.SpriteTableAddress);
	SV(_state.SpritePatternSelector);
	SV(_state.NametableHeight);
	SV(_state.VisibleScanlineCount);
	SV(_state.TextColorIndex);
	SV(_state.BackgroundColorIndex);
	SV(_state.HorizontalScroll);
	SV(_state.HorizontalScrollLatch);
	SV(_state.VerticalScroll);
	SV(_state.VerticalScrollLatch);
	SV(_state.ScanlineCounter);
	SV(_state.ScanlineCounterLatch);
	SV(_state.SyncDisabled);
	SV(_state.M2_AllowHeightChange);
	SV(_state.UseMode4);
	SV(_state.ShiftSpritesLeft);
	SV(_state.EnableScanlineIrq);
	SV(_state.MaskFirstColumn);
	SV(_state.HorizontalScrollLock);
	SV(_state.VerticalScrollLock);
	SV(_state.Sg16KVramMode);
	SV(_state.RenderingEnabled);
	SV(_state.EnableVerticalBlankIrq);
	SV(_state.M1_Use224LineMode);
	SV(_state.M3_Use240LineMode);
	SV(_state.UseLargeSprites);
	SV(_state.EnableDoubleSpriteSize);
	SV(_state.NametableAddress);
	SV(_state.EffectiveNametableAddress);
	SV(_state.NametableAddressMask);
	SV(_state.ColorTableAddress);
	SV(_state.BgPatternTableAddress);

	if(s.GetFormat() != SerializeFormat::Map) {
		//Hide these entries from the Lua API
		SVArray(_bgShifters, 4);

		SV(_evalCounter);
		SV(_inRangeSpriteCount);
		SV(_spriteOverflowPending);

		SV(_spriteIndex);
		SV(_inRangeSpriteIndex);
		SV(_spriteCount);
		SVArray(_inRangeSprites, 64);
		for(int i = 0; i < 64; i++) {
			SVI(_spriteShifters[i].HardwareSprite);
			SVI(_spriteShifters[i].SpriteX);
			SVI(_spriteShifters[i].SpriteRow);
			SVI(_spriteShifters[i].TileAddr);
			SVI(_spriteShifters[i].TileData[0]);
			SVI(_spriteShifters[i].TileData[1]);
			SVI(_spriteShifters[i].TileData[2]);
			SVI(_spriteShifters[i].TileData[3]);
		}

		SV(_lastMasterClock);
		SV(_bgPriority);
		SV(_bgPalette);
		SV(_bgTileAddr);
		SV(_bgOffsetY);
		SV(_minDrawCycle);
		SV(_pixelsAvailable);
		SV(_bgHorizontalMirror);
		SV(_scanlineCount);
		SV(_region);
		SV(_revision);
		SV(_latchRequest);
		SV(_latchPos);
		SV(_readPending);
		SV(_writePending);
		SV(_bgTileIndex);
		SV(_bgPatternData);
		SV(_textModeStep);

		SV(_needCramDot);
		SV(_cramDotColor);
	}
}
