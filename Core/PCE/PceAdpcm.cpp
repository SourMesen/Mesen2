#include "stdafx.h"
#include "PCE/PceAdpcm.h"
#include "PCE/PceScsiBus.h"
#include "PCE/PceCdRom.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"

using namespace ScsiSignal;

PceAdpcm::PceAdpcm(PceCdRom* cdrom, PceScsiBus* scsi)
{
	_state = {};
	_cdrom = cdrom;
	_scsi = scsi;
	_ram = new uint8_t[0x10000];
}

PceAdpcm::~PceAdpcm()
{
	delete[] _ram;
}

void PceAdpcm::Reset()
{
	_state.ReadAddress = 0;
	_state.WriteAddress = 0;
	_state.AddressPort = 0;
	_state.Playing = false;
	_state.EndReached = false;
	_state.HalfReached = false;
	//TODO
}

void PceAdpcm::SetControl(uint8_t value)
{
	if((_state.Control & 0x80) && !(value & 0x80)) {
		Reset();
	}

	if((value & 0x02) && !(_state.Control & 0x02)) {
		_state.WriteAddress = _state.AddressPort - ((value & 0x01) ? 0 : 1);
	}

	if(value & 0x08) {
		_state.ReadAddress = _state.AddressPort - ((value & 0x04) ? 0 : 1);
	}

	if(value & 0x10) {
		_state.AdpcmLength = _state.AddressPort;
		_state.EndReached = false;
	}

	if((value & 0x20) == 0) {
		_state.Playing = false;
	} else if(!_state.Playing) {
		_state.Playing = true;
		//TODO
		//_state.CurrentSample = 2048;
	}

	_state.Control = value;
}

void PceAdpcm::Exec()
{
	//TODO decode sample process

	//every 3 master clocks?
	if(_state.ReadClockCounter > 0) {
		_state.ReadClockCounter -= 3;
		if(_state.ReadClockCounter == 0) {
			_state.ReadBuffer = _ram[_state.ReadAddress];
			_state.ReadAddress++;

			if(_state.AdpcmLength > 0) {
				_state.AdpcmLength--;
			} else {
				_state.EndReached = true;
				_state.HalfReached = true;
			}
		}
	}

	if(_state.WriteClockCounter > 0) {
		_state.WriteClockCounter -= 3;
		if(_state.WriteClockCounter == 0) {
			_ram[_state.WriteAddress] = _state.WriteBuffer;

			if(_state.AdpcmLength < 0xFFFF) {
				_state.AdpcmLength++;
			}

			_state.HalfReached = _state.AdpcmLength < 0x8000;
		}
	}

	bool dmaRequested = (_state.DmaControl & 0x03) != 0;
	if(dmaRequested) {
		if(!_scsi->CheckSignal(Ack) && !_scsi->CheckSignal(Cd) && _scsi->CheckSignal(Io) && _scsi->CheckSignal(Req)) {
			_ram[_state.WriteAddress] = _scsi->GetDataPort();
			_state.WriteAddress++;
			_state.AdpcmLength++; //limit?

			_scsi->SetSignalValue(Ack, false);
			_scsi->SetSignalValue(Req, false);
		}

		if(!_scsi->IsDataTransferInProgress()) {
			_state.DmaControl = 0;
		}
	}

	if(_state.HalfReached) {
		_cdrom->SetIrqSource(PceCdRomIrqSource::Adpcm);
	} else {
		_cdrom->ClearIrqSource(PceCdRomIrqSource::Adpcm);
	}

	if(_state.EndReached) {
		_cdrom->SetIrqSource(PceCdRomIrqSource::Stop);
	} else {
		_cdrom->ClearIrqSource(PceCdRomIrqSource::Stop);
	}
}

void PceAdpcm::Write(uint16_t addr, uint8_t value)
{
	LogDebug("Write ADPCM register: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));

	switch(addr & 0x3FF) {
		case 0x08: _state.AddressPort = (_state.AddressPort & 0xFF00) | value; break;
		case 0x09: _state.AddressPort = (_state.AddressPort & 0xFF) | (value << 8); break;

		case 0x0A:
			_state.WriteClockCounter = 72;
			_state.DataPort = value;
			break;

		case 0x0B: _state.DmaControl = value; break;
		case 0x0D: _state.Control = value; break;
		case 0x0E: _state.PlaybackRate = value; break;
		case 0x0F: _state.FadeTimer = value; break;

		default:
			LogDebug("Write unimplemented ADPCM register: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

uint8_t PceAdpcm::Read(uint16_t addr)
{
	switch(addr & 0x3FF) {
		case 0x0A:
			_state.ReadClockCounter = 72;
			return _state.DataPort;

		case 0x0B: return _state.DmaControl;

		case 0x0C:
			return (
				(_state.EndReached ? 0x01 : 0) |
				(_state.Writing ? 0x04 : 0) |
				(_state.Playing ? 0x08 : 0) |
				(_state.Reading ? 0x80 : 0)
				);

		default:
			LogDebug("Read unimplemented ADPCM register: " + HexUtilities::ToHex(addr));
			return 0;
	}
}
