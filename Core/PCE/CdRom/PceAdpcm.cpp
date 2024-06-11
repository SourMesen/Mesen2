#include "pch.h"
#include "PCE/CdRom/PceAdpcm.h"
#include "PCE/CdRom/PceScsiBus.h"
#include "PCE/CdRom/PceCdRom.h"
#include "PCE/PceConsole.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

using namespace ScsiSignal;

PceAdpcm::PceAdpcm(PceConsole* console, Emulator* emu, PceCdRom* cdrom, PceScsiBus* scsi)
{
	_state = {};
	_console = console;
	_cdrom = cdrom;
	_scsi = scsi;
	_emu = emu;

	_ram = new uint8_t[0x10000];
	console->InitializeRam(_ram, 0x10000);
	emu->RegisterMemory(MemoryType::PceAdpcmRam, _ram, 0x10000);
}

PceAdpcm::~PceAdpcm()
{
	delete[] _ram;
}

void PceAdpcm::Reset()
{
	_state.WriteClockCounter = 0;
	_state.ReadClockCounter = 0;
	_state.ReadAddress = 0;
	_state.WriteAddress = 0;
	_state.AddressPort = 0;
	SetEndReached(false);
	SetHalfReached(false);
	_state.AdpcmLength = 0;
	_state.Nibble = false;
	_currentOutput = 0;
	_magnitude = 0;
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

bool PceAdpcm::IsLengthLatchEnabled()
{
	return (_state.Control & 0x10) != 0;
}

void PceAdpcm::ProcessFlags()
{
	if(IsLengthLatchEnabled()) {
		_state.AdpcmLength = _state.AddressPort;
		SetEndReached(false);
	}

	if(_state.Control & 0x80) {
		Reset();
	}

	if(_state.Control & 0x20) {
		_needExec = true;
	}
}

void PceAdpcm::SetControl(uint8_t value)
{
	if((value & 0x02) && !(_state.Control & 0x02)) {
		_state.WriteAddress = _state.AddressPort - ((value & 0x01) ? 0 : 1);
		LogDebug("[ADPCM] Update write addr");
	}

	if((value & 0x08) && !(_state.Control & 0x08)) {
		_state.ReadAddress = _state.AddressPort - ((value & 0x04) ? 0 : 1);
		LogDebug("[ADPCM] Update read addr");
	}

	_state.Control = value;
	ProcessFlags();
}

void PceAdpcm::ProcessReadOperation()
{
	_state.ReadClockCounter--;
	if(_state.ReadClockCounter == 0) {
		_state.ReadBuffer = _ram[_state.ReadAddress];
		_emu->ProcessMemoryAccess<CpuType::Pce, MemoryType::PceAdpcmRam, MemoryOperationType::Read>(_state.ReadAddress, _state.ReadBuffer);
		_state.ReadAddress++;

		if(!IsLengthLatchEnabled()) {
			if(_state.AdpcmLength > 0) {
				_state.AdpcmLength--;
				SetHalfReached(_state.AdpcmLength < 0x8000);
			} else {
				SetEndReached(true);
				SetHalfReached(false);
			}
		}
	}
}

void PceAdpcm::ProcessWriteOperation()
{
	_state.WriteClockCounter--;
	if(_state.WriteClockCounter == 0) {
		_emu->ProcessMemoryAccess<CpuType::Pce, MemoryType::PceAdpcmRam, MemoryOperationType::Write>(_state.WriteAddress, _state.WriteBuffer);
		_ram[_state.WriteAddress] = _state.WriteBuffer;
		_state.WriteAddress++;

		if(_state.AdpcmLength == 0) {
			SetEndReached(true);
		}
		SetHalfReached(_state.AdpcmLength < 0x8000);

		if(!IsLengthLatchEnabled()) {
			_state.AdpcmLength = (_state.AdpcmLength + 1) & 0x1FFFF;
		}
	}
}

void PceAdpcm::ProcessDmaRequest()
{
	if(_dmaWriteCounter) {
		if(--_dmaWriteCounter == 0) {
			if(!_state.WriteClockCounter) {
				_state.WriteClockCounter = GetClocksToNextSlot(false);
				_state.WriteBuffer = _scsi->GetDataPort();
				_scsi->SetAckWithAutoClear();

				if(!_scsi->IsDataBlockReady()) {
					//Reset bit 0 only
					//Resetting bit 1 breaks Record of Lodoss War (freezes when start is pressed)
					//Not resetting bit 0 breaks Seiya Monogatari - Anearth (freezes on logo)
					_state.DmaControl &= ~0x01;
				}
			} else {
				_dmaWriteCounter++;
			}
		}
	} else if(!_scsi->CheckSignal(Ack) && !_scsi->CheckSignal(Cd) && _scsi->CheckSignal(Io) && _scsi->CheckSignal(Req)) {
		//Some delay is required here according to test rom
		//Valid range is 10-14 (higher or lower fails the test)
		_dmaWriteCounter = 12;
	}
}

uint8_t PceAdpcm::GetClocksToNextSlot(bool forRead)
{
	uint64_t slotIndex = _console->GetMasterClock() / 9;
	uint8_t slotType = slotIndex & 0x03;
	uint8_t gap;
	switch(slotType) {
		default: case 0: gap = forRead ? 3 : 1; break; //Refresh slot (ADPCM uses DRAM - needs periodic refresh)
		case 1: gap = forRead ? 2 : 1; break; //Write slot
		case 2: gap = forRead ? 1 : 3; break; //Write slot
		case 3: gap = forRead ? 4 : 2; break; //Read slot
	}

	uint64_t nextSlotClock = (slotIndex + gap) * 9;
	return (nextSlotClock - _console->GetMasterClock()) / 3;
}

void PceAdpcm::Exec()
{
	//Called every 3 master clocks
	ProcessFlags();

	if(_state.Playing || (_state.Control & 0x20)) {
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
	if(dmaRequested) {
		ProcessDmaRequest();
	}

	ProcessFlags();

	_needExec = _state.Playing || (_state.Control & 0x20) || _state.ReadClockCounter || _state.WriteClockCounter || dmaRequested || _dmaWriteCounter;
}

void PceAdpcm::Write(uint16_t addr, uint8_t value)
{
	switch(addr & 0x3FF) {
		case 0x08: _state.AddressPort = (_state.AddressPort & 0xFF00) | value; break;
		case 0x09: _state.AddressPort = (_state.AddressPort & 0xFF) | (value << 8); break;

		case 0x0A:
			_state.WriteClockCounter = GetClocksToNextSlot(false);
			_state.WriteBuffer = value;
			_needExec = true;
			break;

		case 0x0B:
			if(!_scsi->IsDataBlockReady()) {
				//Bit 0 can't be set if a transfer isn't already running
				value &= ~0x01;
			}

			_state.DmaControl = value;
			_needExec = true;
			LogDebugIf((value & 0x03), "[ADPCM] DMA");
			break;

		case 0x0D: SetControl(value); break;
		case 0x0E: {
			_state.PlaybackRate = value;
			double freq = 32000.0 / (16 - (_state.PlaybackRate & 0x0F));
			_clocksPerSample = PceConstants::MasterClockRate / freq;
			break;
		}

		default:
			LogDebug("Write unimplemented ADPCM register: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

uint8_t PceAdpcm::Read(uint16_t addr)
{
	switch(addr & 0x3FF) {
		case 0x0A:
			_state.ReadClockCounter = GetClocksToNextSlot(true);
			_needExec = true;
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

		case 0x0E: return _state.PlaybackRate;

		default:
			LogDebug("Read unimplemented ADPCM register: " + HexUtilities::ToHex(addr));
			return 0;
	}
}

void PceAdpcm::PlaySample()
{
	if(_state.Control & 0x80) {
		//Reset flag is enabled
		_state.Playing = (_state.Control & 0x20) != 0;
		return;
	}

	if((_state.Control & 0x40) && _state.AdpcmLength == 0) {
		_state.Control &= ~0x20;
	}

	if(!(_state.Control & 0x20)) {
		_state.Playing = false;
		return;
	}

	if(!_state.Playing) {
		_state.Playing = true;
		_currentOutput = 2048;
	}

	uint8_t data;
	if(_state.Nibble) {
		_state.Nibble = false;
		data = _ram[_state.ReadAddress] & 0x0F;
		_state.ReadAddress++;
		_state.AdpcmLength = (_state.AdpcmLength - 1) & 0x1FFFF;
	} else {
		_state.Nibble = true;
		data = (_ram[_state.ReadAddress] >> 4) & 0x0F;
	}

	uint8_t value = data & 0x07;
	int8_t sign = data & 0x08 ? -1 : 1;

	int16_t adjustment = _stepSize[(_magnitude << 3) | value] * sign;
	_currentOutput += adjustment;
	
	_magnitude = std::clamp(_magnitude + _stepFactor[value], 0, 48);

	SetHalfReached(_state.AdpcmLength <= 0x8000);
	if(_state.AdpcmLength == 0) {
		SetEndReached(true);
	}

	int16_t out = (std::clamp<int16_t>(_currentOutput, 0, 4095) - 2048) * 10;
	_samplesToPlay.push_back(out);
	_samplesToPlay.push_back(out);
}

void PceAdpcm::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	double freq = 32000.0 / (16 - (_state.PlaybackRate & 0x0F));
	double volume = _cdrom->GetAudioFader().GetVolume(PceAudioFaderTarget::Adpcm);
	_resampler.SetVolume(_emu->GetSettings()->GetPcEngineConfig().AdpcmVolume / 100.0 * volume);
	_resampler.SetSampleRates(freq, sampleRate);
	_resampler.Resample<true>(_samplesToPlay.data(), (uint32_t)_samplesToPlay.size() / 2, out, sampleCount, _state.Playing);
	_samplesToPlay.clear();
}

void PceAdpcm::Serialize(Serializer& s)
{
	SVArray(_ram, 0x10000);

	SV(_state.Nibble);
	SV(_state.ReadAddress);
	SV(_state.WriteAddress);
	SV(_state.AddressPort);
	SV(_state.DmaControl);
	SV(_state.Control);
	SV(_state.PlaybackRate);
	SV(_state.AdpcmLength);
	SV(_state.EndReached);
	SV(_state.HalfReached);
	SV(_state.Playing);
	SV(_state.ReadBuffer);
	SV(_state.ReadClockCounter);
	SV(_state.WriteBuffer);
	SV(_state.WriteClockCounter);

	SV(_currentOutput);
	SV(_magnitude);
	SV(_clocksPerSample);
	SV(_nextSampleCounter);
	SV(_dmaWriteCounter);

	if(!s.IsSaving()) {
		_needExec = true;
	}
}
