#include "pch.h"
#include "PCE/PceVce.h"
#include "PCE/PceConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/CpuType.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryType.h"

PceVce::PceVce(Emulator* emu, PceConsole* console)
{
	_emu = emu;
	_console = console;

	_paletteRam = new uint16_t[0x200];

	_state.ClockDivider = 4;
	_state.ScanlineCount = 262;

	_console->InitializeRam(_paletteRam, 0x400);
	for(int i = 0; i < 0x200; i++) {
		_paletteRam[i] &= 0x1FF;
	}

	_emu->RegisterMemory(MemoryType::PcePaletteRam, _paletteRam, 0x200 * sizeof(uint16_t));
}

PceVce::~PceVce()
{
	delete[] _paletteRam;
}

uint8_t PceVce::Read(uint16_t addr)
{
	switch(addr & 0x07) {
		default:
		case 0: return 0xFF; //write-only, reads return $FF
		case 1: return 0xFF; //unused, reads return $FF
		case 2: return 0xFF; //write-only, reads return $FF
		case 3: return 0xFF; //write-only, reads return $FF

		case 4: return _paletteRam[_state.PalAddr] & 0xFF;

		case 5:
		{
			uint8_t val = (_paletteRam[_state.PalAddr] >> 8) & 0x01;
			_state.PalAddr = (_state.PalAddr + 1) & 0x1FF;

			//Bits 1 to 7 are set to 1 when reading MSB
			return 0xFE | val;
		}

		case 6: return 0xFF; //unused, reads return $FF
		case 7: return 0xFF; //unused, reads return $FF
	}
}

void PceVce::Write(uint16_t addr, uint8_t value)
{
	switch(addr & 0x07) {
		case 0x00:
			_state.ScanlineCount = (value & 0x04) ? 263 : 262;
			_state.Grayscale = (value & 0x80) != 0;
			switch(value & 0x03) {
				case 0: _state.ClockDivider = 4; break;
				case 1: _state.ClockDivider = 3; break;
				case 2: case 3: _state.ClockDivider = 2; break;
			}
			//LogDebug("[Debug] VCE Clock divider: " + HexUtilities::ToHex(_state.VceClockDivider) + "  SL: " + std::to_string(_state.Scanline));
			break;

		case 0x01: break; //Unused, writes do nothing

		case 0x02: _state.PalAddr = (_state.PalAddr & 0x100) | value; break;
		case 0x03: _state.PalAddr = (_state.PalAddr & 0xFF) | ((value & 0x01) << 8); break;

		case 0x04:
			_emu->ProcessPpuWrite<CpuType::Pce>((_state.PalAddr << 1), value, MemoryType::PcePaletteRam);
			_paletteRam[_state.PalAddr] = (_paletteRam[_state.PalAddr] & 0x100) | value;
			break;

		case 0x05:
			_emu->ProcessPpuWrite<CpuType::Pce>((_state.PalAddr << 1) + 1, value, MemoryType::PcePaletteRam);
			_paletteRam[_state.PalAddr] = (_paletteRam[_state.PalAddr] & 0xFF) | ((value & 0x01) << 8);
			_state.PalAddr = (_state.PalAddr + 1) & 0x1FF;
			break;

		case 0x06: break; //Unused, writes do nothing
		case 0x07: break; //Unused, writes do nothing
	}
}

void PceVce::Serialize(Serializer& s)
{
	SVArray(_paletteRam, 0x200);
	SV(_state.ClockDivider);
	SV(_state.Grayscale);
	SV(_state.PalAddr);
	SV(_state.ScanlineCount);
}
