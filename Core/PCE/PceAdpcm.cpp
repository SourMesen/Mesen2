#include "stdafx.h"
#include "PCE/PceAdpcm.h"
#include "PCE/PceScsiBus.h"
#include "PCE/PceCdRom.h"
#include "PCE/PceConstants.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"

using namespace ScsiSignal;

PceAdpcm::PceAdpcm(Emulator* emu, PceCdRom* cdrom, PceScsiBus* scsi)
{
	_state = {};
	_cdrom = cdrom;
	_scsi = scsi;

	_ram = new uint8_t[0x10000];
	emu->GetSettings()->InitializeRam(_ram, 0x10000);
	emu->RegisterMemory(MemoryType::PceAdpcmRam, _ram, 0x10000);
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
	SetEndReached(false);
	SetHalfReached(false);
	_state.Nibble = false;
	_currentOutput = 0;
	_magnitude = 0;
	//TODO
}

void PceAdpcm::SetHalfReached(bool value)
{
	if(_state.HalfReached != value) {
		_state.HalfReached = value;
		
		if(value) {
			_cdrom->SetIrqSource(PceCdRomIrqSource::Adpcm);
		} else {
			_cdrom->ClearIrqSource(PceCdRomIrqSource::Adpcm);
		}
	}
}

void PceAdpcm::SetEndReached(bool value)
{
	if(_state.EndReached != value) {
		_state.EndReached = value;

		if(value) {
			_cdrom->SetIrqSource(PceCdRomIrqSource::Stop);
		} else {
			_cdrom->ClearIrqSource(PceCdRomIrqSource::Stop);
		}
	}
}

void PceAdpcm::SetControl(uint8_t value)
{
	if((value & 0x02) && !(_state.Control & 0x02)) {
		_state.WriteAddress = _state.AddressPort - ((value & 0x01) ? 0 : 1);
		LogDebug("[ADPCM] Update write addr");
	}

	if(value & 0x08) {
		_state.ReadAddress = _state.AddressPort - ((value & 0x04) ? 0 : 1);
		LogDebug("[ADPCM] Update read addr");
	}

	if(value & 0x10) {
		_state.AdpcmLength = _state.AddressPort;
		SetEndReached(false);
		LogDebug("[ADPCM] Update length");
	}

	if((value & 0x20) == 0) {
		_state.Playing = false;
		LogDebugIf(_state.Playing, "[ADPCM] Stop");
	} else if(!_state.Playing) {
		LogDebug("[ADPCM] Play");
		_state.Playing = true;
		_currentOutput = 2048;
	}

	if(value & 0x80) {
		LogDebug("[ADPCM] Reset");
		Reset();
	}

	_state.Control = value;
}

void PceAdpcm::ProcessReadOperation()
{
	_state.ReadClockCounter--;
	if(_state.ReadClockCounter == 0) {
		_state.ReadBuffer = _ram[_state.ReadAddress];
		_state.ReadAddress++;

		if(_state.AdpcmLength > 0) {
			_state.AdpcmLength--;
		} else {
			SetEndReached(true);
			SetHalfReached(false);
		}
	}
}

void PceAdpcm::ProcessWriteOperation()
{
	_state.WriteClockCounter--;
	if(_state.WriteClockCounter == 0) {
		_ram[_state.WriteAddress] = _state.WriteBuffer;
		_state.WriteAddress++;

		if(_state.AdpcmLength < 0xFFFF) {
			_state.AdpcmLength++;
		}

		SetHalfReached(_state.AdpcmLength < 0x8000);
	}
}

void PceAdpcm::ProcessDmaRequest()
{
	if(!_scsi->CheckSignal(Ack) && !_scsi->CheckSignal(Cd) && _scsi->CheckSignal(Io) && _scsi->CheckSignal(Req)) {
		_state.WriteClockCounter = 24;
		_state.WriteBuffer = _scsi->GetDataPort();
		_scsi->SetSignalValue(Ack, false);
		_scsi->SetSignalValue(Req, false);
	}

	if(!_scsi->IsDataTransferInProgress()) {
		_state.DmaControl = 0;
	}
}

void PceAdpcm::Exec()
{
	//Called every 3 master clocks
	if(_state.Playing) {
		_nextSampleCounter += 3;
		if(_nextSampleCounter >= _clocksPerSample) {
			PlaySample();
			_nextSampleCounter -= _clocksPerSample;
		}
	}

	if(_state.ReadClockCounter > 0) {
		ProcessReadOperation();
	}

	if(_state.WriteClockCounter > 0) {
		ProcessWriteOperation();
	}

	bool dmaRequested = (_state.DmaControl & 0x03) != 0;
	if(dmaRequested && !_state.WriteClockCounter) {
		ProcessDmaRequest();
	}
}

void PceAdpcm::Write(uint16_t addr, uint8_t value)
{
	LogDebug("Write ADPCM register: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));

	switch(addr & 0x3FF) {
		case 0x08: _state.AddressPort = (_state.AddressPort & 0xFF00) | value; break;
		case 0x09: _state.AddressPort = (_state.AddressPort & 0xFF) | (value << 8); break;

		case 0x0A:
			_state.WriteClockCounter = 24;
			_state.WriteBuffer = value;
			break;

		case 0x0B:
			_state.DmaControl = value;
			LogDebugIf((value & 0x03), "[ADPCM] DMA");
			break;

		case 0x0D: SetControl(value); break;
		case 0x0E: {
			_state.PlaybackRate = value & 0x0F;
			double freq = 32000.0 / (16 - _state.PlaybackRate);
			_clocksPerSample = PceConstants::MasterClockRate / freq;
			break;
		}

		case 0x0F: _state.FadeTimer = value; break;

		default:
			LogDebug("Write unimplemented ADPCM register: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

uint8_t PceAdpcm::Read(uint16_t addr)
{
	LogDebug("Read ADPCM register: " + HexUtilities::ToHex(addr));

	switch(addr & 0x3FF) {
		case 0x0A:
			_state.ReadClockCounter = 24;
			return _state.ReadBuffer;

		case 0x0B: return _state.DmaControl;

		case 0x0C:
			return (
				(_state.EndReached ? 0x01 : 0) |
				(_state.WriteClockCounter ? 0x04 : 0) |
				(_state.Playing ? 0x08 : 0) |
				(_state.ReadClockCounter ? 0x80 : 0)
			);

		case 0x0D:
			return _state.Control;

		default:
			LogDebug("Read unimplemented ADPCM register: " + HexUtilities::ToHex(addr));
			return 0;
	}
}

void PceAdpcm::PlaySample()
{
	uint8_t data;
	if(_state.Nibble) {
		_state.Nibble = false;
		data = _ram[_state.ReadAddress] & 0x0F;
		_state.ReadAddress++;
		_state.AdpcmLength--;
	} else {
		_state.Nibble = true;
		data = (_ram[_state.ReadAddress] >> 4) & 0x0F;
	}

	uint8_t value = data & 0x07;
	int8_t sign = data & 0x08 ? -1 : 1;

	int16_t adjustment = _stepSize[(_magnitude << 3) | value] * sign;
	_currentOutput = std::clamp(_currentOutput + adjustment, 0, 4095);
	
	_magnitude = std::clamp(_magnitude + _stepFactor[value], 0, 48);

	SetHalfReached(_state.AdpcmLength < 0x8000);
	if(_state.AdpcmLength == 0) {
		_state.Playing = false;
		SetEndReached(true);
		SetHalfReached(false);
	}

	int16_t out = (_currentOutput - 2048) * 10;
	_samplesToPlay.push_back(out);
	_samplesToPlay.push_back(out);
}

void PceAdpcm::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	double freq = 32000.0 / (16 - _state.PlaybackRate);
	_resampler.SetSampleRates(freq, sampleRate);
	_resampler.Resample<true>(_samplesToPlay.data(), (uint32_t)_samplesToPlay.size() / 2, out, sampleCount);
	_samplesToPlay.clear();
}
