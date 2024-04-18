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
#include "FDS_LUT_norm.h"

void FdsAudio::Serialize(Serializer& s)
{
	BaseExpansionAudio::Serialize(s);

	SVArray(_waveTable, 64);
	SV(_volume);
	SV(_mod);
	SV(_waveWriteEnabled); SV(_disableEnvelopes); SV(_haltWaveform); SV(_masterVolume); SV(_waveAccumulator); SV(_waveM2Counter); SV(_wavePitch); SV(_wavePosition); SV(_lastOutput);
}

void FdsAudio::ClockAudio()
{
	int frequency = _volume.GetFrequency();
	if(!_haltWaveform && !_disableEnvelopes) {
		_volume.TickEnvelope(_wavePosition);
		if(_mod.TickEnvelope(_wavePosition)) {
			_mod.UpdateOutput(frequency);
		}
	}

	if(_mod.TickModulator()) {
		//Modulator was ticked, update wave pitch
		_mod.UpdateOutput(frequency);
	}

	if(_haltWaveform) {
		_waveAccumulator = 0;
		UpdateOutput();
	} else {
		UpdateOutput();

		if(!_waveWriteEnabled) {
			if(++_waveM2Counter == 16) {
				_waveAccumulator += (frequency * _mod.GetOutput()) & 0xFFFFF;
				if(_waveAccumulator > 0xFFFFFF) {
					_waveAccumulator -= 0x1000000;
				}

				_wavePosition = (_waveAccumulator >> 18) & 0x3F;
				_waveM2Counter = 0;
			}
		}
	}
}

void FdsAudio::UpdateOutput()
{
	uint32_t level = std::min((int)_volume.GetGain(), 32);
	uint8_t outputLevel = uint8_t(DACTable[_waveTable[_wavePosition]][_masterVolume] * level);


	if(_lastOutput != outputLevel) {
		_console->GetApu()->AddExpansionAudioDelta(AudioChannel::FDS, outputLevel - _lastOutput);
		_lastOutput = outputLevel;
	}
}

FdsAudio::FdsAudio(NesConsole* console) : BaseExpansionAudio(console)
{
	// initialize DAC LUT
	// data comes from plgDavid's DC capture of an FDS's DAC output
	// data capture shared from the NESDev server

	for(int masterlevel = 0; masterlevel < 4; masterlevel++)
		for(int wavelevel = 0; wavelevel < 64; wavelevel++)
			DACTable[wavelevel][masterlevel] = (FDS_LUT_norm[wavelevel] * 64.0 * float(WaveVolumeTable[masterlevel])) / 1152.0;
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
	} else if(addr == 0x4091) {
		// Wave accumulator
		value &= 0xC0;
		value |= (_waveAccumulator >> 12) & 0xFF;
	} else if(addr == 0x4092) {
		value &= 0xC0;
		value |= _mod.GetGain();
	} else if(addr == 0x4093) {
		// Mod accumulator
		value &= 0xC0;
		value |= (_mod.GetModAccumulator() >> 5) & 0x7F;
	} else if(addr == 0x4094) {
		// wave pitch intermediate result
		value &= 0xC0;
		value |= (_mod.GetOutput() >> 4) & 0xFF;
	} else if(addr == 0x4096) {
		// wavetable position
		value &= 0xC0;
		value |= _wavePosition & 0x3F;
	} else if(addr == 0x4097) {
		// mod counter value
		value &= 0xC0;
		value |= _mod.GetCounter() & 0x7F;
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
	
	entries.push_back(MapperStateEntry("$4083.7", "Halt Wave Form", _haltWaveform, MapperStateValueType::Bool));

	entries.push_back(MapperStateEntry("", "Gain", _volume.GetGain(), MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("$4084-$4088", "Modulation"));
	entries.push_back(MapperStateEntry("$4084.0-5", "Envelope Speed", _mod.GetSpeed(), MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4084.6", "Envelope Direction", _mod.GetVolumeIncreaseFlag() ? "Increase" : "Decrease", _mod.GetVolumeIncreaseFlag()));
	entries.push_back(MapperStateEntry("$4084.7", "Envelope Disabled", _mod.IsEnvelopeDisabled(), MapperStateValueType::Bool));
	
	int8_t modCounter = _mod.GetCounter();
	entries.push_back(MapperStateEntry("$4085.0-6", "Counter", std::to_string(modCounter), modCounter < 0 ? (modCounter + 128) : modCounter));

	entries.push_back(MapperStateEntry("$4086/7.0-11", "Frequency", _mod.GetFrequency(), MapperStateValueType::Number16));
	
	entries.push_back(MapperStateEntry("$4087.6", "Force Tick Modulator", _mod.GetForceCarryOut(), MapperStateValueType::Bool));

	entries.push_back(MapperStateEntry("$4087.7", "Disabled", _mod.IsModulationDisabled(), MapperStateValueType::Bool));
	entries.push_back(MapperStateEntry("", "Gain", _mod.GetGain(), MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("", "Mod Output", std::to_string(_mod.GetOutput())));

	entries.push_back(MapperStateEntry("$4089-$408A", "Misc. Audio"));
	entries.push_back(MapperStateEntry("$4089.0-2", "Master Volume", _masterVolume, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4089.7", "Wave Write Enabled", _waveWriteEnabled, MapperStateValueType::Bool));
	
	entries.push_back(MapperStateEntry("$408A", "Envelope Speed Multiplier", _volume.GetMasterSpeed(), MapperStateValueType::Number8));

}
