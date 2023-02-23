#include "pch.h"
#include "NES/Mappers/FDS/FdsAudio.h"
#include "NES/Mappers/FDS/ModChannel.h"
#include "NES/Mappers/FDS/BaseFdsChannel.h"
#include "NES/NesConsole.h"
#include "NES/NesTypes.h"
#include "NES/NesMemoryManager.h"
#include "NES/APU/NesApu.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "Utilities/Serializer.h"

void FdsAudio::Serialize(Serializer& s)
{
	BaseExpansionAudio::Serialize(s);

	SVArray(_waveTable, 64);
	SV(_volume);
	SV(_mod);
	SV(_waveWriteEnabled); SV(_disableEnvelopes); SV(_haltWaveform); SV(_masterVolume); SV(_waveOverflowCounter); SV(_wavePitch); SV(_wavePosition); SV(_lastOutput);
}

void FdsAudio::ClockAudio()
{
	int frequency = _volume.GetFrequency();
	if(!_haltWaveform && !_disableEnvelopes) {
		_volume.TickEnvelope();
		if(_mod.TickEnvelope()) {
			_mod.UpdateOutput(frequency);
		}
	}

	if(_mod.TickModulator()) {
		//Modulator was ticked, update wave pitch
		_mod.UpdateOutput(frequency);
	}

	if(_haltWaveform) {
		_wavePosition = 0;
		UpdateOutput();
	} else {
		UpdateOutput();

		if(frequency + _mod.GetOutput() > 0 && !_waveWriteEnabled) {
			_waveOverflowCounter += frequency + _mod.GetOutput();
			if(_waveOverflowCounter < frequency + _mod.GetOutput()) {
				_wavePosition = (_wavePosition + 1) & 0x3F;
			}
		}
	}
}

void FdsAudio::UpdateOutput()
{
	uint32_t level = std::min((int)_volume.GetGain(), 32) * WaveVolumeTable[_masterVolume];
	uint8_t outputLevel = (_waveTable[_wavePosition] * level) / 1152;


	if(_lastOutput != outputLevel) {
		_console->GetApu()->AddExpansionAudioDelta(AudioChannel::FDS, outputLevel - _lastOutput);
		_lastOutput = outputLevel;
	}
}

FdsAudio::FdsAudio(NesConsole* console) : BaseExpansionAudio(console)
{
}

uint8_t FdsAudio::ReadRegister(uint16_t addr)
{
	uint8_t value = _console->GetMemoryManager()->GetOpenBus();
	if(addr <= 0x407F) {
		value &= 0xC0;
		if(_waveWriteEnabled) {
			value |= _waveTable[addr & 0x3F];
		} else {
			//"When writing is disabled ($4089.7), reading anywhere in 4040-407F returns the value at the current wave position"
			value |= _waveTable[_wavePosition];
		}
	} else if(addr == 0x4090) {
		value &= 0xC0;
		value |= _volume.GetGain();
	} else if(addr == 0x4092) {
		value &= 0xC0;
		value |= _mod.GetGain();
	}

	return value;
}

void FdsAudio::WriteRegister(uint16_t addr, uint8_t value)
{
	if(addr <= 0x407F) {
		if(_waveWriteEnabled) {
			_waveTable[addr & 0x3F] = value & 0x3F;
		}
	} else {
		switch(addr) {
			case 0x4080:
			case 0x4082:
				_volume.WriteReg(addr, value);
				break;

			case 0x4083:
				_disableEnvelopes = (value & 0x40) != 0;
				_haltWaveform = (value & 0x80) != 0;
				if(_disableEnvelopes) {
					_volume.ResetTimer();
					_mod.ResetTimer();
				}
				_volume.WriteReg(addr, value);
				break;

			case 0x4086:
			case 0x4087:
				_mod.WriteReg(addr, value);
				break;

			case 0x4084:
			case 0x4085:
				_mod.WriteReg(addr, value);
				
				//Need to update mod output if gain/speed were changed
				_mod.UpdateOutput(_volume.GetFrequency());
				break;

			case 0x4088:
				_mod.WriteModTable(value);
				break;

			case 0x4089:
				_masterVolume = value & 0x03;
				_waveWriteEnabled = (value & 0x80) == 0x80;
				break;

			case 0x408A:
				_volume.SetMasterEnvelopeSpeed(value);
				_mod.SetMasterEnvelopeSpeed(value);
				break;
		}
	}
}

void FdsAudio::GetMapperStateEntries(vector<MapperStateEntry>& entries)
{
	entries.push_back(MapperStateEntry("", "Audio"));
	entries.push_back(MapperStateEntry("$4080-$4083", "Volume"));
	entries.push_back(MapperStateEntry("$4080.0-5", "Envelope Speed", _volume.GetSpeed(), MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4080.6", "Envelope Direction", _volume.GetVolumeIncreaseFlag() ? "Increase" : "Decrease", _volume.GetVolumeIncreaseFlag()));
	entries.push_back(MapperStateEntry("$4080.7", "Envelope Disabled", _volume.IsEnvelopeDisabled(), MapperStateValueType::Bool));
	entries.push_back(MapperStateEntry("$4082/3.0-11", "Frequency", _volume.GetFrequency(), MapperStateValueType::Number16));
	entries.push_back(MapperStateEntry("$4083.6", "Volume/Mod Envelopes Disabled", _disableEnvelopes, MapperStateValueType::Bool));
	
	//todo emulation logic + this based on new info
	//entries.push_back(MapperStateEntry("$4083.7", "Halt Wave Form", _haltWaveform, MapperStateValueType::Bool));

	entries.push_back(MapperStateEntry("", "Gain", _volume.GetGain(), MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("$4084-$4088", "Modulation"));
	entries.push_back(MapperStateEntry("$4084.0-5", "Envelope Speed", _mod.GetSpeed(), MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4084.6", "Envelope Direction", _mod.GetVolumeIncreaseFlag() ? "Increase" : "Decrease", _mod.GetVolumeIncreaseFlag()));
	entries.push_back(MapperStateEntry("$4084.7", "Envelope Disabled", _mod.IsEnvelopeDisabled(), MapperStateValueType::Bool));
	
	int8_t modCounter = _mod.GetCounter();
	entries.push_back(MapperStateEntry("$4085.0-6", "Counter", std::to_string(modCounter), modCounter < 0 ? (modCounter + 128) : modCounter));

	entries.push_back(MapperStateEntry("$4086/7.0-11", "Frequency", _mod.GetFrequency(), MapperStateValueType::Number16));
	
	//todo emulation logic + this based on new info
	//entries.push_back(MapperStateEntry("$4087.6", "???", false, MapperStateValueType::Bool));

	entries.push_back(MapperStateEntry("$4087.7", "Disabled", _mod.IsModulationDisabled(), MapperStateValueType::Bool));
	entries.push_back(MapperStateEntry("", "Gain", _mod.GetGain(), MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("", "Mod Output", std::to_string(_mod.GetOutput())));

	entries.push_back(MapperStateEntry("$4089-$408A", "Misc. Audio"));
	entries.push_back(MapperStateEntry("$4089.0-2", "Master Volume", _masterVolume, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4089.7", "Wave Write Enabled", _waveWriteEnabled, MapperStateValueType::Bool));
	
	entries.push_back(MapperStateEntry("$408A", "Envelope Speed Multiplier", _volume.GetMasterSpeed(), MapperStateValueType::Number8));

}
