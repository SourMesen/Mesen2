#include "stdafx.h"
#include "PCE/PceCdRom.h"
#include "PCE/PceConsole.h"
#include "PCE/PceMemoryManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"

using namespace ScsiSignal;

PceCdRom::PceCdRom(PceConsole* console, DiscInfo& disc) : _scsi(console, this, disc), _adpcm(this, &_scsi)
{
	_console = console;
}

void PceCdRom::Exec()
{
	_scsi.Exec();
	_adpcm.Exec();
}

void PceCdRom::SetIrqSource(PceCdRomIrqSource src)
{
	//LogDebug("Set IRQ source: " + HexUtilities::ToHex((uint8_t)src));
	if((_state.ActiveIrqs & (uint8_t)src) == 0) {
		_state.ActiveIrqs |= (uint8_t)src;
		UpdateIrqState();
	}
}

void PceCdRom::ClearIrqSource(PceCdRomIrqSource src)
{
	//LogDebug("Clear IRQ source: " + HexUtilities::ToHex((uint8_t)src));
	if(_state.ActiveIrqs & (uint8_t)src) {
		_state.ActiveIrqs &= ~(uint8_t)src;
		UpdateIrqState();
	}
}

void PceCdRom::UpdateIrqState()
{
	if((_state.EnabledIrqs & _state.ActiveIrqs) != 0) {
		_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq2);
	} else {
		_console->GetMemoryManager()->ClearIrqSource(PceIrqSource::Irq2);
	}
}

void PceCdRom::Write(uint16_t addr, uint8_t value)
{
	switch(addr & 0x3FF) {
		case 0x00: //SCSI Control
			_scsi.SetSignalValue(Sel, true);
			_scsi.Exec();
			_scsi.SetSignalValue(Sel, false);
			_scsi.Exec();
			break;

		case 0x01: //CDC/SCSI Command
			_scsi.SetDataPort(value);
			_scsi.Exec();
			break;

		case 0x02: //ACK
			_scsi.SetSignalValue(Ack, (value & 0x80) != 0);
			_scsi.Exec();

			_state.EnabledIrqs = value & 0x7C;
			UpdateIrqState();
			break;

		case 0x03: break; //BRAM lock, CD Status (readonly)

		case 0x04: {
			//Reset
			bool reset = (value & 0x02) != 0;
			_scsi.SetSignalValue(Rst, reset);
			_scsi.Exec();
			if(reset) {
				//Clear enabled IRQs flags for SCSI drive (SubChannel + DataTransferDone + DataTransferReady)
				_state.EnabledIrqs &= 0x8F;
				UpdateIrqState();
			}
			break;
		}

		case 0x05:
		case 0x06:
			//Latch CD data
			break;

		case 0x07:
			//BRAM unlock
			if((value & 0x80) != 0) {
				_state.BramLocked = false;
			}
			break;

		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			_adpcm.Write(addr, value);
			break;
	}
}

uint8_t PceCdRom::Read(uint16_t addr)
{
	switch(addr & 0x3FF) {
		case 0x00: return _scsi.GetStatus();
		case 0x01: return _scsi.GetDataPort();
		case 0x02: return _state.EnabledIrqs;
		case 0x03:
			_state.BramLocked = true;
			_state.ReadRightChannel = !_state.ReadRightChannel;

			return (
				_state.ActiveIrqs |
				(_state.ReadRightChannel ? 0 : 0x02)
			);

		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			LogDebug("Read unimplemented CDROM register: " + HexUtilities::ToHex(addr));
			break;

		case 0x08:
		{
			uint8_t val = _scsi.GetDataPort();
			if(_scsi.CheckSignal(Req) && _scsi.CheckSignal(Io) && !_scsi.CheckSignal(Cd)) {
				_scsi.SetSignalValue(Ack, false);
				_scsi.SetSignalValue(Req, false);
				_scsi.Exec();
			}
			return val;
		}

		case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			return _adpcm.Read(addr);

		case 0xC1: return 0xAA;
		case 0xC2: return 0x55;
		case 0xC3: return 0x00;
		case 0xC5: return 0xAA;
		case 0xC6: return 0x55;
		case 0xC7: return 0x03;

		default:
			LogDebug("Read unknown CDROM register: " + HexUtilities::ToHex(addr));
			break;
	}

	return 0xFF;
}
