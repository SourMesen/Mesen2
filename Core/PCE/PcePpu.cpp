#include "stdafx.h"
#include "PCE/PcePpu.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceConstants.h"
#include "PCE/PceConsole.h"
#include "Shared/EmuSettings.h"
#include "Shared/RewindManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/NotificationManager.h"
#include "EventType.h"

PcePpu::PcePpu(Emulator* emu, PceConsole* console)
{
	_emu = emu;
	_console = console;
	_vram = new uint16_t[0x8000];
	_spriteRam = new uint16_t[0x100];
	_paletteRam = new uint16_t[0x200];
	
	_outBuffer[0] = new uint16_t[PceConstants::MaxScreenWidth * PceConstants::ScreenHeight];
	_outBuffer[1] = new uint16_t[PceConstants::MaxScreenWidth * PceConstants::ScreenHeight];
	_currentOutBuffer = _outBuffer[0];

	memset(_outBuffer[0], 0, PceConstants::MaxScreenWidth * PceConstants::ScreenHeight * sizeof(uint16_t));
	memset(_outBuffer[1], 0, PceConstants::MaxScreenWidth * PceConstants::ScreenHeight * sizeof(uint16_t));

	_emu->GetSettings()->InitializeRam(_vram, 0x10000);
	_emu->GetSettings()->InitializeRam(_spriteRam, 0x200);
	_emu->GetSettings()->InitializeRam(_paletteRam, 0x400);
	for(int i = 0; i < 0x200; i++) {
		_paletteRam[i] &= 0x1FF;
	}

	//These values can't ever be 0, init them to a possible value
	_state.ColumnCount = 32;
	_state.RowCount = 32;
	_state.VramAddrIncrement = 1;
	_state.VceClockDivider = 4;
	_state.HorizDisplayWidth = 0x1F;
	_state.VertDisplayWidth = 239;
	_state.VceScanlineCount = 262;
	_screenWidth = 256;
	UpdateFrameTimings();

	_emu->RegisterMemory(MemoryType::PceVideoRam, _vram, 0x8000 * sizeof(uint16_t));
	_emu->RegisterMemory(MemoryType::PcePaletteRam, _paletteRam, 0x200 * sizeof(uint16_t));
	_emu->RegisterMemory(MemoryType::PceSpriteRam, _spriteRam, 0x100 * sizeof(uint16_t));
}

PcePpu::~PcePpu()
{
	delete[] _vram;
	delete[] _paletteRam;
	delete[] _spriteRam;
	delete[] _outBuffer[0];
	delete[] _outBuffer[1];
}

PcePpuState& PcePpu::GetState()
{
	return _state;
}

uint16_t* PcePpu::GetScreenBuffer()
{
	return _currentOutBuffer;
}

uint16_t* PcePpu::GetPreviousScreenBuffer()
{
	return _currentOutBuffer == _outBuffer[0] ? _outBuffer[1] : _outBuffer[0];
}

uint16_t PcePpu::GetScreenWidth()
{
	return _screenWidth;
}

uint16_t PcePpu::DotsToClocks(int dots)
{
	return dots * _state.VceClockDivider;
}

void PcePpu::Exec()
{
	if(_state.SatbTransferRunning) {
		ProcessSatbTransfer();
	}

	if(_hModeCounter <= 3) {
		ProcessEvent();
	} else {
		_hModeCounter -= 3;
		_state.HClock += 3;
	}

	if(_state.HClock == 1365) {
		ProcessEndOfScanline();

		if(_needRcrIncrement) {
			IncrementRcrCounter();
		}

		//VCE sets HBLANK to low every 1365 clocks, interrupting what 
		//the VDC was doing and starting a 8-pixel HSW phase
		_hModeCounter = DotsToClocks(16);
		_hMode = PcePpuModeH::Hsw;
		
		if(_state.HorizDisplayStart < 2) {
			TriggerHdsIrqs();
		}

		_xStart = 0;

		if(_state.Scanline == _state.VceScanlineCount - 3) {
			//VCE sets VBLANK for 3 scanlines at the end of every frame
			if(_vMode == PcePpuModeV::Vdw) {
				_needVertBlankIrq = true;
			}
			_vMode = PcePpuModeV::Vsw;
			_vModeCounter = _state.VertSyncWidth + 1;
		}
	}

	_emu->ProcessPpuCycle<CpuType::Pce>();
}

void PcePpu::TriggerHdsIrqs()
{
	if(_needVertBlankIrq) {
		ProcessEndOfVisibleFrame();
	}
	if(_hasSpriteOverflow && _state.EnableOverflowIrq) {
		_state.SpriteOverflow = true;
		_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
	}
}

void PcePpu::ProcessEvent()
{
	for(int i = 0; i < 3; i++) {
		_state.HClock++;
		_hModeCounter--;

		if(_hModeCounter == 0) {
			DrawScanline();
			_hMode = (PcePpuModeH)(((int)_hMode + 1) % 8);
			switch(_hMode) {
				case PcePpuModeH::Hds:
					if(_state.HorizDisplayStart > 2) {
						_hModeCounter = DotsToClocks((_state.HorizDisplayStart - 2) * 8);
					} else {
						if(_state.HorizDisplayStart == 2) {
							TriggerHdsIrqs();
						}
						_hMode = PcePpuModeH::Hds_VerticalBlankIrq;
						_hModeCounter = DotsToClocks(2);
					}
					LogDebug("H: " + std::to_string(_state.HClock) + " - HDS");
					break;
				
				case PcePpuModeH::Hds_VerticalBlankIrq:
					TriggerHdsIrqs();
					_hModeCounter = DotsToClocks(2);
					break;

				case PcePpuModeH::Hds_ScrollYLatch:
					if(_state.Scanline < _state.VerticalBlankScanline) {
						IncScrollY();
					}
					_hModeCounter = DotsToClocks(2);
					LogDebug("H: " + std::to_string(_state.HClock) + " - Scroll Y Latch");
					break;

				case PcePpuModeH::Hds_ScrollXLatch:
					_state.BgScrollXLatch = _state.BgScrollX;
					if(!_state.BurstModeEnabled) {
						_state.BackgroundEnabled = _state.NextBackgroundEnabled;
						_state.SpritesEnabled = _state.NextSpritesEnabled;
					}
					if(_state.HorizDisplayStart > 2) {
						_hModeCounter = DotsToClocks(20);
					} else {
						_hModeCounter = DotsToClocks((_state.HorizDisplayStart + 1) * 8 - 4);
					}
					LogDebug("H: " + std::to_string(_state.HClock) + " - Scroll X Latch");
					break;

				case PcePpuModeH::Hdw:
					_needRcrIncrement = true;
					_hModeCounter = DotsToClocks((_state.HorizDisplayWidth - 1) * 8 + 2);
					LogDebug("H: " + std::to_string(_state.HClock) + " - HDW start");
					break;
				
				case PcePpuModeH::Hdw_RcrIrq:
					IncrementRcrCounter();
					_hModeCounter = DotsToClocks(2 * 8 - 2);
					LogDebug("H: " + std::to_string(_state.HClock) + " - RCR inc");
					break;

				case PcePpuModeH::Hde:
					_hModeCounter = DotsToClocks((_state.HorizDisplayEnd + 1) * 8);
					LogDebug("H: " + std::to_string(_state.HClock) + " - HDE");
					break;

				case PcePpuModeH::Hsw:
					_hModeCounter = DotsToClocks((_state.HorizSyncWidth + 1) * 8);
					LogDebug("H: " + std::to_string(_state.HClock) + " - HSW");
					break;
			}
		}
	}
}

void PcePpu::ProcessSatbTransfer()
{
	//This takes 1024 VDC cycles (so 2048/3072/4096 master clocks depending on VCE/VDC speed)
	//1 word transfered every 4 dots (8 to 16 master clocks, depending on VCE clock divider)
	_state.SatbTransferNextWordCounter += 3;
	if(_state.SatbTransferNextWordCounter / _state.VceClockDivider == 4) {
		_state.SatbTransferNextWordCounter -= _state.SatbTransferNextWordCounter / _state.VceClockDivider;

		int i = _state.SatbTransferOffset;
		uint16_t value = _vram[(_state.SatbBlockSrc + i) & 0x7FFF];
		_emu->ProcessPpuWrite<CpuType::Pce>(i << 1, value & 0xFF, MemoryType::PceSpriteRam);
		_emu->ProcessPpuWrite<CpuType::Pce>((i << 1) + 1, value >> 8, MemoryType::PceSpriteRam);
		_spriteRam[i] = value;

		_state.SatbTransferOffset++;

		if(_state.SatbTransferOffset == 0) {
			_state.SatbTransferRunning = false;

			if(_state.VramSatbIrqEnabled) {
				_state.SatbTransferDone = true;
				_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
			}
		}
	}
}

void PcePpu::IncrementRcrCounter()
{
	_state.RcrCounter++;

	if(_needBgScrollYInc) {
		IncScrollY();
	}
	_needBgScrollYInc = true;
	_needRcrIncrement = false;

	_vModeCounter--;
	if(_vModeCounter == 0) {
		_vMode = (PcePpuModeV)(((int)_vMode + 1) % 4);
		switch(_vMode) {
			default:
			case PcePpuModeV::Vds:
				_vModeCounter = _state.VertDisplayStart + 2;
				break;

			case PcePpuModeV::Vdw:
				_vModeCounter = _state.VertDisplayWidth + 1;
				_state.RcrCounter = 0;
				break;

			case PcePpuModeV::Vde:
				_vModeCounter = _state.VertEndPosVcr;
				break;

			case PcePpuModeV::Vsw:
				_vModeCounter = _state.VertSyncWidth + 1;
				break;
		}
	}

	if(_state.RcrCounter == _state.VertDisplayWidth + 1) {
		_needVertBlankIrq = true;
	}

	//This triggers ~12 VDC cycles before the end of the visible part of the scanline
	if(_state.EnableScanlineIrq && _state.RcrCounter == (int)_state.RasterCompareRegister - 0x40) {
		_state.ScanlineDetected = true;
		_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
	}
}

void PcePpu::IncScrollY()
{
	if(_state.RcrCounter == 0) {
		_state.BgScrollYLatch = _state.BgScrollY;
	} else {
		_state.BgScrollYLatch++;
	}
	_needBgScrollYInc = false;
}

void PcePpu::ProcessEndOfScanline()
{
	//TODO timing, approx end of scanline display
	//Draw entire scanline at the end of the scanline
	DrawScanline();

	ChangeResolution();
	_state.HClock = 0;
	_state.Scanline++;

	if(_state.Scanline == 256) {
		_state.FrameCount++;
		SendFrame();
	} else if(_state.Scanline >= _state.VceScanlineCount) {
		//Update flags that were locked during burst mode
		_state.Scanline = 0;
		_state.BurstModeEnabled = !_state.NextBackgroundEnabled && !_state.NextSpritesEnabled;

		_emu->ProcessEvent(EventType::StartFrame);

		if(_state.NextBackgroundEnabled || _state.NextSpritesEnabled) {
			//Reset current width to current frame data only if rendering is enabled
			_screenWidth = GetCurrentScreenWidth();
		}
	}
}

void PcePpu::ProcessEndOfVisibleFrame()
{
	//End of display, trigger irq?
	if(_state.SatbTransferPending || _state.RepeatSatbTransfer) {
		_state.SatbTransferPending = false;
		_state.SatbTransferRunning = true;
		_state.SatbTransferNextWordCounter = 0;
		_state.SatbTransferOffset = 0;
	}

	if(_state.EnableVerticalBlankIrq) {
		_state.VerticalBlank = true;
		_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
	}

	_needVertBlankIrq = false;
}

template<uint8_t bpp>
uint8_t GetTilePixelColor(const uint16_t* chrData, const uint8_t shift)
{
	uint8_t color;
	if(bpp == 2) {
		//TODO unused
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
	} else if(bpp == 4) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
		color |= ((chrData[8] >> shift) & 0x01) << 2;
		color |= ((chrData[8] >> (7 + shift)) & 0x02) << 2;
	} else if(bpp == 5) {
		//TODO hack, fix
		color = (chrData[0] >> shift) & 0x01;
		color |= ((chrData[16] >> shift) & 0x01) << 1;
		color |= ((chrData[32] >> shift) & 0x01) << 2;
		color |= ((chrData[48] >> shift) & 0x01) << 3;
	} else {
		throw std::runtime_error("unsupported bpp");
	}
	return color;
}

void PcePpu::DrawScanline()
{
	if(_state.Scanline < 14 || _state.Scanline >= 256) {
		//Only 242 rows can be shown
		return;
	}

	uint16_t rowWidth = 1365 / _state.VceClockDivider;
	uint16_t* out = _rowBuffer;

	uint16_t xMax = std::min<uint16_t>(rowWidth, _state.HClock / _state.VceClockDivider);
	bool hasBg[PceConstants::MaxScreenWidth] = {};
	uint16_t screenY = (_state.BgScrollYLatch) & ((_state.RowCount * 8) - 1);
	bool inPicture = _vMode == PcePpuModeV::Vdw && (_hMode == PcePpuModeH::Hdw || _hMode == PcePpuModeH::Hdw_RcrIrq) && !_state.BurstModeEnabled;

	if(inPicture && _state.BackgroundEnabled) {
		for(uint16_t x = _xStart; x < xMax; x++) {
			uint16_t screenX = _state.BgScrollXLatch & ((_state.ColumnCount * 8) - 1);

			uint16_t batEntry = _vram[(screenY >> 3) * _state.ColumnCount + (screenX >> 3)];
			uint8_t palette = batEntry >> 12;
			uint16_t tileIndex = (batEntry & 0xFFF);

			uint16_t tileAddr = tileIndex * 16;
			uint8_t color = GetTilePixelColor<4>(_vram + ((tileAddr + (screenY & 0x07)) & 0x7FFF), 7 - (screenX & 0x07));
			if(color != 0) {
				hasBg[x] = true;
				out[x] = _paletteRam[palette * 16 + color];
			} else {
				out[x] = _paletteRam[0];
			}

			_state.BgScrollXLatch++;
		}
	} else if(inPicture) {
		uint16_t color = _paletteRam[0];
		for(uint16_t x = _xStart; x < xMax; x++) {
			//In picture, but BG is not enabled, draw bg color
			out[x] = color;
		}
	} else {
		uint16_t color = _paletteRam[16 * 16];
		for(uint16_t x = _xStart; x < xMax; x++) {
			//Output hasn't started yet, display overscan color
			out[x] = color;
		}
	}
/*
	if(_xStart == 0) {
		_hasSpriteOverflow = false;
	}

	if(_state.SpritesEnabled) {
		bool hasSprite[PceConstants::MaxScreenWidth] = {};
		bool hasSprite0[PceConstants::MaxScreenWidth] = {};

		uint32_t rowSpriteCount = 0;
		for(int i = 0; i < 64; i++) {
			int16_t y = (int16_t)(_spriteRam[i * 4] & 0x3FF) - 64;
			if(row < y) {
				//Sprite not visible on this line
				continue;
			}

			uint8_t height;
			uint16_t flags = _spriteRam[i * 4 + 3];
			switch((flags >> 12) & 0x03) {
				default:
				case 0: height = 16; break;
				case 1: height = 32; break;
				case 2: case 3: height = 64; break;
			}

			if(row >= y + height) {
				//Sprite not visible on this line
				continue;
			}

			uint16_t tileIndex = (_spriteRam[i * 4 + 2] & 0x7FF) >> 1;
			uint8_t width = (flags & 0x100) ? 32 : 16;
			int16_t spriteX = (int16_t)(_spriteRam[i * 4 + 1] & 0x3FF) - 32;
			
			if(_xStart == 0) {
				if(rowSpriteCount >= 16) {
					//Sprite limit reached, stop displaying sprites, trigger overflow irq
					_hasSpriteOverflow = true;
					break;
				} else {
					if(rowSpriteCount == 15 && width == 32) {
						//Hide second half of sprite due to sprite limit
						width = 16;
					}
					rowSpriteCount += (flags & 0x100) ? 2 : 1;
				}
			}

			if(spriteX + width <= 0 || spriteX >= (int32_t)xMax) {
				//Sprite off-screen
				continue;
			}

			uint16_t spriteRow = row - y;
			if(width == 32) {
				tileIndex &= ~0x01;
			}
			if(height == 32) {
				tileIndex &= ~0x02;
			} else if(height == 64) {
				tileIndex &= ~0x06;
			}

			bool verticalMirror = (flags & 0x8000) != 0;
			bool horizontalMirror = (flags & 0x800) != 0;
			bool priority = (flags & 0x80) != 0;
			int yOffset = 0;
			int rowOffset = 0;
			if(verticalMirror) {
				yOffset = (height - spriteRow - 1) & 0x0F;
				rowOffset = (height - spriteRow - 1) >> 4;
			} else {
				yOffset = spriteRow & 0x0F;
				rowOffset = spriteRow >> 4;
			}

			uint16_t spriteTileY = tileIndex | (rowOffset << 1);

			for(int x = 0; x < width; x++) {
				if(spriteX + x < _xStart) {
					continue;
				} else if(spriteX + x >= (int32_t)xMax) {
					//Offscreen
					break;
				}

				uint8_t xOffset;
				int columnOffset;
				if(horizontalMirror) {
					xOffset = (width - x - 1) & 0x0F;
					columnOffset = (width - x - 1) >> 4;
				} else {
					xOffset = x & 0x0F;
					columnOffset = x >> 4;
				}

				uint16_t spriteTile = spriteTileY | columnOffset;
				uint16_t tileStart = spriteTile * 64;

				uint16_t pixelStart = tileStart + yOffset;
				uint8_t shift = 15 - xOffset;

				uint8_t color = GetTilePixelColor<5>(_vram + (pixelStart & 0x7FFF), shift);

				if(color != 0) {
					if(hasSprite[spriteX + x]) {
						if(hasSprite0[spriteX + x] && _state.EnableCollisionIrq) {
							_state.Sprite0Hit = true;
							_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
						}
					} else {
						if(priority || hasBg[spriteX + x] == 0) {
							out[outputOffset + spriteX + x] = _paletteRam[color + (flags & 0x0F) * 16 + 16 * 16];
						}
						
						hasSprite[spriteX + x] = true;
						if(i == 0) {
							hasSprite0[spriteX + x] = true;
						}
					}
				}
			}
		}
	}
	*/

	if(_state.HClock == 1365) {
		if(_state.Scanline == 14) {
			_currentOutBuffer = _currentOutBuffer == _outBuffer[0] ? _outBuffer[1] : _outBuffer[0];
		}
		
		uint32_t offset = ((int)_state.Scanline - 14) * _screenWidth;
		int leftOverscan = 5 * 8 * 4 / _state.VceClockDivider;

		//Scanline is less wide than previous scanlines, center it (and clear both sides)
		uint16_t pictureWidth = GetCurrentScreenWidth();
		int gap = 0;// (_screenWidth - pictureWidth) / 2;
		memset(_currentOutBuffer + offset, 0, gap * sizeof(uint16_t));
		memset(_currentOutBuffer + offset + pictureWidth + gap, 0, gap * sizeof(uint16_t));
		memcpy(_currentOutBuffer + offset + gap, _rowBuffer + leftOverscan, pictureWidth * sizeof(uint16_t));

		_emu->AddDebugEvent<CpuType::Pce>(DebugEventType::ScanlineEnd);
	}

	_xStart = xMax;
}

uint32_t PcePpu::GetCurrentScreenWidth()
{
	switch(_state.VceClockDivider) {
		case 2: return 64 * 8;
		case 3: return 44*8;

		default:
		case 4: return 32*8;
	}
}

void PcePpu::ChangeResolution()
{
	uint32_t newWidth = GetCurrentScreenWidth();
	int lastDrawnRow = (int)_state.Scanline - 14;

	if(newWidth > _screenWidth && lastDrawnRow >= 0 && lastDrawnRow < PceConstants::ScreenHeight) {
		int gap = (newWidth - _screenWidth) / 2;
		//Move pixels in buffer to match new resolution, draw old lines in the center
		for(int i = lastDrawnRow; i >= 0; i--) {
			uint32_t src = i * _screenWidth;
			uint32_t dst = i * newWidth;
			memmove(_currentOutBuffer + dst + gap, _currentOutBuffer + src, _screenWidth * sizeof(uint16_t));
			memset(_currentOutBuffer + dst, 0, gap * sizeof(uint16_t));
			memset(_currentOutBuffer + dst + _screenWidth + gap, 0, gap * sizeof(uint16_t));
		}
		_screenWidth = newWidth;
	}
}

void PcePpu::SendFrame()
{
	_emu->ProcessEvent(EventType::EndFrame);
	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone, _currentOutBuffer);

	bool forRewind = _emu->GetRewindManager()->IsRewinding();

	RenderedFrame frame(_currentOutBuffer, _screenWidth, PceConstants::ScreenHeight, 1.0, _state.FrameCount, _console->GetControlManager()->GetPortStates());
	_emu->GetVideoDecoder()->UpdateFrame(frame, forRewind, forRewind);

	_emu->ProcessEndOfFrame();

	_console->GetControlManager()->UpdateInputState();
}

void PcePpu::UpdateFrameTimings()
{
	_state.DisplayStart = _state.VertDisplayStart + _state.VertSyncWidth;
	_state.VerticalBlankScanline = _state.DisplayStart + _state.VertDisplayWidth + 1;
	if(_state.VerticalBlankScanline > 261) {
		_state.VerticalBlankScanline = 261;
	}
}

void PcePpu::LoadReadBuffer()
{
	//TODO timing - this needs to be done in-between rendering reads (based on mode, etc.)
	_state.ReadBuffer = _vram[_state.MemAddrRead & 0x7FFF];
	_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1), (uint8_t)_state.ReadBuffer, MemoryType::PceVideoRam);
	_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1) + 1, (uint8_t)(_state.ReadBuffer >> 8), MemoryType::PceVideoRam);
}

uint8_t PcePpu::ReadVdc(uint16_t addr)
{
	DrawScanline();

	switch(addr & 0x03) {
		default:
		case 0: {
			//TODO BSY
			uint8_t result = 0;
			result |= _state.VerticalBlank ? 0x20 : 0x00;
			result |= _state.VramTransferDone ? 0x10 : 0x00;
			result |= _state.SatbTransferDone ? 0x08 : 0x00;
			result |= _state.ScanlineDetected ? 0x04 : 0x00;
			result |= _state.SpriteOverflow ? 0x02 : 0x00;
			result |= _state.Sprite0Hit ? 0x01 : 0x00;

			_state.VerticalBlank = false;
			_state.VramTransferDone = false;
			_state.SatbTransferDone = false;
			_state.ScanlineDetected = false;
			_state.SpriteOverflow = false;
			_state.Sprite0Hit = false;

			_console->GetMemoryManager()->ClearIrqSource(PceIrqSource::Irq1);
			return result;
		}

		case 1: return 0; //Unused, reads return 0

		//Reads to 2/3 will always return the read buffer, but the
		//read address will only increment when register 2 is selected
		case 2: return (uint8_t)_state.ReadBuffer;

		case 3:
			uint8_t value = _state.ReadBuffer >> 8;
			if(_state.CurrentReg == 0x02) {
				_state.MemAddrRead += _state.VramAddrIncrement;
				LoadReadBuffer();
			}
			return value;
	}
}

void PcePpu::WriteVdc(uint16_t addr, uint8_t value)
{
	DrawScanline();

	switch(addr & 0x03) {
		case 0: _state.CurrentReg = value & 0x1F; break;

		case 1: break; //Unused, writes do nothing

		case 2:
		case 3:
			bool msb = (addr & 0x03) == 0x03;

			switch(_state.CurrentReg) {
				case 0x00: UpdateReg(_state.MemAddrWrite, value, msb); break;

				case 0x01:
					UpdateReg(_state.MemAddrRead, value, msb);
					if(msb) {
						LoadReadBuffer();
					}
					break;

				case 0x02:
					UpdateReg(_state.VramData, value, msb);
					if(msb) {
						if(_state.MemAddrWrite < 0x8000) {
							//Ignore writes to mirror at $8000+
							//TODO timing
							_emu->ProcessPpuWrite<CpuType::Pce>(_state.MemAddrWrite << 1, _state.VramData & 0xFF, MemoryType::PceVideoRam);
							_emu->ProcessPpuWrite<CpuType::Pce>((_state.MemAddrWrite << 1) + 1, value, MemoryType::PceVideoRam);
							_vram[_state.MemAddrWrite] = _state.VramData;
						}

						_state.MemAddrWrite += _state.VramAddrIncrement;
					}
					break;

				case 0x05:
					if(msb) {
						//TODO output select
						//TODO dram refresh
						switch((value >> 3) & 0x03) {
							case 0: _state.VramAddrIncrement = 1; break;
							case 1: _state.VramAddrIncrement = 0x20; break;
							case 2: _state.VramAddrIncrement = 0x40; break;
							case 3: _state.VramAddrIncrement = 0x80; break;
						}
					} else {
						_state.EnableCollisionIrq = (value & 0x01) != 0;
						_state.EnableOverflowIrq = (value & 0x02) != 0;
						_state.EnableScanlineIrq = (value & 0x04) != 0;
						_state.EnableVerticalBlankIrq = (value & 0x08) != 0;
						
						_state.OutputVerticalSync = ((value & 0x30) >> 4) >= 2;
						_state.OutputHorizontalSync = ((value & 0x30) >> 4) >= 1;

						_state.NextSpritesEnabled = (value & 0x40) != 0;
						_state.NextBackgroundEnabled = (value & 0x80) != 0;
					}
					break;

				case 0x06: UpdateReg<0x3FF>(_state.RasterCompareRegister, value, msb); break;
				case 0x07: UpdateReg<0x3FF>(_state.BgScrollX, value, msb); break;
				case 0x08:
					UpdateReg<0x1FF>(_state.BgScrollY, value, msb);
					_state.BgScrollYUpdatePending = true;
					break;

				case 0x09:
					if(!msb) {
						switch((value >> 4) & 0x03) {
							case 0: _state.ColumnCount = 32; break;
							case 1: _state.ColumnCount = 64; break;
							case 2: case 3: _state.ColumnCount = 128; break;
						}

						_state.RowCount = (value & 0x40) ? 64 : 32;

						_state.VramAccessMode = value & 0x03;
						_state.SpriteAccessMode = (value >> 2) & 0x03;
						_state.CgMode = (value & 0x80) != 0;
					}
					break;

				case 0x0A:
					if(msb) {
						//TODO - this probably has an impact on timing within the scanline?
						_state.HorizDisplayStart = value & 0x7F;
					} else {
						_state.HorizSyncWidth = value & 0x1F;
					}
					break;

				case 0x0B:
					if(msb) {
						_state.HorizDisplayEnd = value & 0x7F;
					} else {
						_state.HorizDisplayWidth = value & 0x7F;
						UpdateFrameTimings();
					}
					break;

				case 0x0C: 
					if(msb) {
						_state.VertDisplayStart = value;
					} else {
						_state.VertSyncWidth = value & 0x1F;
					}
					UpdateFrameTimings();
					break;

				case 0x0D:
					UpdateReg<0x1FF>(_state.VertDisplayWidth, value, msb);
					UpdateFrameTimings();
					break;

				case 0x0E: 
					if(!msb) {
						_state.VertEndPosVcr = value;
					}
					break;

				case 0x0F:
					if(!msb) {
						_state.VramSatbIrqEnabled = (value & 0x01) != 0;
						_state.VramVramIrqEnabled = (value & 0x02) != 0;
						_state.DecrementSrc = (value & 0x04) != 0;
						_state.DecrementDst = (value & 0x08) != 0;
						_state.RepeatSatbTransfer = (value & 0x10) != 0;
					}
					break;

				case 0x10: UpdateReg(_state.BlockSrc, value, msb); break;
				case 0x11: UpdateReg(_state.BlockDst, value, msb); break;
				case 0x12:
					UpdateReg(_state.BlockLen, value, msb);

					if(msb) {
						//TODO DMA TIMING
						do {
							_state.BlockLen--;

							if(_state.BlockDst < 0x8000) {
								//Ignore writes over $8000
								_vram[_state.BlockDst] = _vram[_state.BlockSrc];
							}

							_state.BlockSrc += (_state.DecrementSrc ? -1 : 1);
							_state.BlockDst += (_state.DecrementDst ? -1 : 1);

						} while(_state.BlockLen != 0xFFFF);

						if(_state.VramVramIrqEnabled) {
							_state.VramTransferDone = true;
							_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
						}
					}
					break;

				case 0x13:
					UpdateReg(_state.SatbBlockSrc, value, msb);
					if(msb) {
						_state.SatbTransferPending = true;
					}
					break;
			}
	}
}

uint8_t PcePpu::ReadVce(uint16_t addr)
{
	DrawScanline();

	switch(addr & 0x07) {
		default:
		case 0: return 0xFF; //write-only, reads return $FF
		case 1: return 0xFF; //unused, reads return $FF
		case 2: return 0xFF; //write-only, reads return $FF
		case 3: return 0xFF; //write-only, reads return $FF

		case 4: return _paletteRam[_state.PalAddr] & 0xFF;
		
		case 5: {
			uint8_t val = (_paletteRam[_state.PalAddr] >> 8) & 0x01;
			_state.PalAddr = (_state.PalAddr + 1) & 0x1FF;

			//Bits 1 to 7 are set to 1 when reading MSB
			return 0xFE | val;
		}

		case 6: return 0xFF; //unused, reads return $FF
		case 7: return 0xFF; //unused, reads return $FF
	}
}

void PcePpu::WriteVce(uint16_t addr, uint8_t value)
{
	DrawScanline();

	switch(addr & 0x07) {
		case 0x00:
			_state.VceScanlineCount = (value & 0x04) ? 263 : 262;
			switch(value & 0x03) {
				case 0: _state.VceClockDivider = 4; break;
				case 1: _state.VceClockDivider = 3; break;
				case 2: case 3: _state.VceClockDivider = 2; break;
			}
			UpdateFrameTimings();
			//LogDebug("[Debug] VCE Clock divider: " + HexUtilities::ToHex(_state.VceClockDivider) + "  SL: " + std::to_string(_state.Scanline));
			break;

		case 0x01: break; //Unused, writes do nothing

		case 0x02: _state.PalAddr = (_state.PalAddr & 0x100) | value; break;
		case 0x03: _state.PalAddr = (_state.PalAddr & 0xFF) | ((value & 0x01) << 8); break;

		case 0x04:
			_emu->ProcessPpuWrite<CpuType::Pce>((_state.PalAddr << 1), value, MemoryType::PceVideoRam);
			_paletteRam[_state.PalAddr] = (_paletteRam[_state.PalAddr] & 0x100) | value;
			break;

		case 0x05:
			_emu->ProcessPpuWrite<CpuType::Pce>((_state.PalAddr << 1) + 1, value, MemoryType::PceVideoRam);
			_paletteRam[_state.PalAddr] = (_paletteRam[_state.PalAddr] & 0xFF) | ((value & 0x01) << 8);
			_state.PalAddr = (_state.PalAddr + 1) & 0x1FF;
			break;

		case 0x06: break; //Unused, writes do nothing
		case 0x07: break; //Unused, writes do nothing
	}
}
