#include "stdafx.h"
#include "PCE/PcePpu.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceConsole.h"
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
	_outBuffer = new uint16_t[256 * 240];

	memset(_vram, 0, 0x10000);
	memset(_paletteRam, 0, 0x400);

	_emu->RegisterMemory(MemoryType::PceVideoRam, _vram, 0x8000 * sizeof(uint16_t));
	_emu->RegisterMemory(MemoryType::PcePaletteRam, _paletteRam, 0x200 * sizeof(uint16_t));
	_emu->RegisterMemory(MemoryType::PceSpriteRam, _spriteRam, 0x100 * sizeof(uint16_t));
}

PcePpu::~PcePpu()
{
	delete[] _vram;
	delete[] _paletteRam;
	delete[] _spriteRam;
	delete[] _outBuffer;
}

PcePpuState& PcePpu::GetState()
{
	return _state;
}

void PcePpu::Exec()
{
	_cycle++;
	if(_cycle == 341) {
		_cycle = 0;

		if(_scanline < 240) {
			DrawScanline();
		}

		_scanline++;

		if(_scanline + 64 == _state.ScanlineIrqValue) {
			_state.ScanlineDetected = true;
			if(_state.EnableScanlineIrq) {
				_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
			}
		}

		if(_scanline == 240) {
			_state.VerticalBlank = true;
			if(_state.EnableVerticalBlankIrq) {
				_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
			}

			if(_state.SatbTransferPending || _state.RepeatSatbTransfer) {
				for(int i = 0; i < 256; i++) {
					uint16_t value = _vram[(_state.SatbBlockSrc + i) & 0x7FFF];
					_emu->ProcessPpuWrite<CpuType::Pce>(i << 1, value & 0xFF, MemoryType::PceSpriteRam);
					_emu->ProcessPpuWrite<CpuType::Pce>((i << 1) + 1, value >> 8, MemoryType::PceSpriteRam);
					_spriteRam[i] = value;
				}
				_state.SatbTransferPending = false;
				_state.SatbTransferDone = true;

				if(_state.VramSatbIrqEnabled) {
					_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
				}
			}

			_state.FrameCount++;
			SendFrame();
		} else if(_scanline == 263) {
			_scanline = 0;
		}
	}

	_emu->ProcessPpuCycle<CpuType::Pce>();
}

template<uint8_t bpp>
uint8_t GetTilePixelColor(const uint16_t* chrData, const uint8_t shift)
{
	uint8_t color;
	if(bpp == 2) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
	} else if(bpp == 4) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
		color |= ((chrData[8] >> shift) & 0x01) << 2;
		color |= ((chrData[8] >> (7 + shift)) & 0x02) << 2;
	} else if(bpp == 5) {
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
	int columnCount;
	switch((_state.MemAccessWidth >> 4) & 0x03) {
		case 0: columnCount = 32; break;
		case 1: columnCount = 64; break;
		case 2:
		case 3: columnCount = 128; break;
	}

	int rowCount = (_state.MemAccessWidth & 0x40) ? 64 : 32;

	uint16_t screenY = (_scanline + _state.BgScrollY) & ((rowCount * 8) - 1);

	bool hasBg[256] = {};

	for(int column = 0; column < 256; column++) {
		uint16_t screenX = (column + _state.BgScrollX) & ((columnCount * 8) - 1);

		uint16_t batEntry = _vram[(screenY >> 3) * columnCount + (screenX >> 3)];
		uint8_t palette = batEntry >> 12;
		uint16_t tileIndex = (batEntry & 0xFFF);

		uint16_t tileAddr = tileIndex * 16;
		uint8_t color = GetTilePixelColor<4>(_vram + ((tileAddr + (screenY & 0x07)) & 0x7FFF), 7 - (screenX & 0x07));
		if(color) {
			hasBg[column] = true;
			_outBuffer[_scanline * 256 + column] = _paletteRam[palette * 16 + color];
		} else {
			_outBuffer[_scanline * 256 + column] = _paletteRam[0];
		}
	}

	for(int i = 0; i < 64; i++) {
		int16_t y = (int16_t)(_spriteRam[i * 4] & 0x3FF) - 64;
		if(_scanline < y) {
			//Sprite not visible on this line
			continue;
		}

		uint8_t height;
		uint16_t flags = _spriteRam[i * 4 + 3];
		switch((flags >> 12) & 0x03) {
			case 0: height = 16; break;
			case 1: height = 32; break;
			case 2: case 3: height = 64; break;
		}

		if(_scanline >= y + height) {
			//Sprite not visible on this line
			continue;
		}

		uint16_t tileIndex = (_spriteRam[i * 4 + 2] & 0x7FF) >> 1;
		uint8_t width = (flags & 0x100) ? 32 : 16;
		int16_t spriteX = (int16_t)(_spriteRam[i * 4 + 1] & 0x3FF) - 32;

		if(spriteX + width <= 0 || spriteX >= 256) {
			//Sprite off-screen
			continue;
		}

		uint16_t spriteRow = _scanline - y;
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
			if(spriteX + x < 0) {
				continue;
			} else if(spriteX + x >= 256) {
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
				if(priority || hasBg[spriteX + x] == 0) {
					_outBuffer[_scanline * 256 + spriteX + x] = _paletteRam[color + (flags & 0x0F) * 16 + 16 * 16];
				}
			}
		}
	}
}

void PcePpu::SendFrame()
{
	_emu->ProcessEvent(EventType::EndFrame);
	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone, _outBuffer);

	static int _frameCount = 0;
	_frameCount++;
	RenderedFrame frame(_outBuffer, 256, 240, 1.0, _frameCount, _console->GetControlManager()->GetPortStates());
	_emu->GetVideoDecoder()->UpdateFrame(frame, true, false);

	_emu->ProcessEndOfFrame();

	_console->GetControlManager()->UpdateInputState();
}

uint8_t PcePpu::ReadVdc(uint16_t addr)
{
	switch(addr & 0x03) {
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
			
		case 2:
		case 3:
			if(_state.CurrentReg == 0x02) {
				if(addr & 0x01) {
					uint8_t value = _vram[_state.MemAddrRead] >> 8;
					_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1) + 1, value, MemoryType::PceVideoRam);
					_state.MemAddrRead += _state.VramAddrIncrement;
					return value;
				} else {
					uint8_t value = (uint8_t)_vram[_state.MemAddrRead];
					_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1), value, MemoryType::PceVideoRam);
					return value;
				}
			} else {
				MessageManager::Log("read vdc reg 2/3, invalid reg selected");
				return 0;
			}

		default:
			MessageManager::Log("read vdc unknown reg: " + HexUtilities::ToHex(addr));
			return 0;
	}
}

void PcePpu::WriteVdc(uint16_t addr, uint8_t value)
{
	switch(addr & 0x03) {
		case 0: _state.CurrentReg = value & 0x1F; break;

		case 1: break; //NOP

		case 2:
		case 3:
			bool msb = (addr & 0x03) == 0x03;

			switch(_state.CurrentReg) {
				case 0x00: UpdateReg(_state.MemAddrWrite, value, msb); break;
				case 0x01: UpdateReg(_state.MemAddrRead, value, msb); break;

				case 0x02:
					UpdateReg(_state.VramData, value, msb);
					if(msb) {
						if(_state.MemAddrWrite < 0x8000) {
							//Ignore writes to mirror at $8000+
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
						//TODO external sync bits
						_state.SpritesEnabled = (value & 0x40) != 0;
						_state.BackgroundEnabled = (value & 0x80) != 0;
					}
					break;

				case 0x06: UpdateReg<0x3FF>(_state.ScanlineIrqValue, value, msb); break;
				case 0x07: UpdateReg<0x3FF>(_state.BgScrollX, value, msb); break;
				case 0x08: UpdateReg<0x1FF>(_state.BgScrollY, value, msb); break;

				case 0x09:
					if(!msb) {
						_state.MemAccessWidth = value;
					}
					break;

				case 0x0A: UpdateReg<0x7F1F>(_state.HorizSync, value, msb); break;
				case 0x0B: UpdateReg<0x7F7F>(_state.HorizDisplay, value, msb); break;
				case 0x0C: UpdateReg<0xFF1F>(_state.VertSync, value, msb); break;
				case 0x0D: UpdateReg<0x1FF>(_state.VertDisplay, value, msb); break;
				case 0x0E: _state.VertEndPos = value; break;

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
					//todo trigger dma
					MessageManager::Log("vram-vram dma");
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
	switch(addr & 0x07) {
		case 4: return _paletteRam[_state.PalAddr] & 0xFF;
		
		case 5: {
			uint8_t val = (_paletteRam[_state.PalAddr] >> 8) & 0xFF;
			_state.PalAddr = (_state.PalAddr + 1) & 0x1FF;
			return val;
		}

		default:
			MessageManager::Log("read vce unknown reg: " + HexUtilities::ToHex(addr));
			return 0;
	}
}

void PcePpu::WriteVce(uint16_t addr, uint8_t value)
{
	switch(addr & 0x07) {
		case 0x00:
		case 0x01:
			//TODO
			LogDebug("[Debug] Write - VCE missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;

		case 0x02: _state.PalAddr = (_state.PalAddr & 0x100) | value; break;
		case 0x03: _state.PalAddr = (_state.PalAddr & 0xFF) | ((value & 0x01) << 8); break;

		case 0x04:
			_state.PalData = (_state.PalData & 0x100) | value;
			break;

		case 0x05:
			_state.PalData = (_state.PalData & 0xFF) | ((value & 0x01) << 8);
			
			_emu->ProcessPpuWrite<CpuType::Pce>((_state.PalAddr << 1), _state.PalData & 0xFF, MemoryType::PceVideoRam);
			_emu->ProcessPpuWrite<CpuType::Pce>((_state.PalAddr << 1) + 1, value & 0x01, MemoryType::PceVideoRam);

			_paletteRam[_state.PalAddr] = _state.PalData;
			_state.PalAddr = (_state.PalAddr + 1) & 0x1FF;
			break;
	}
}
