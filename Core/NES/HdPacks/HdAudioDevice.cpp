#include "pch.h"
#include "NES/HdPacks/HdAudioDevice.h"
#include "NES/HdPacks/HdData.h"
#include "NES/HdPacks/OggMixer.h"
#include "NES/NesConsole.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/Serializer.h"

HdAudioDevice::HdAudioDevice(Emulator* emu, HdPackData* hdData)
{
	_emu = emu;
	_hdData = hdData;
	_album = 0;
	_playbackOptions = 0;
	_trackError = false;
	_sfxVolume = 128;
	_bgmVolume = 128;

	_oggMixer.reset(new OggMixer());
	_oggMixer->SetBgmVolume(_bgmVolume);
	_oggMixer->SetSfxVolume(_sfxVolume);
	_emu->GetSoundMixer()->RegisterAudioProvider(_oggMixer.get());
}

HdAudioDevice::~HdAudioDevice()
{
	_emu->GetSoundMixer()->UnregisterAudioProvider(_oggMixer.get());
}

void HdAudioDevice::Serialize(Serializer& s)
{
	int32_t trackOffset = 0;
	if(s.IsSaving()) {
		trackOffset = _oggMixer->GetBgmOffset();
		if(trackOffset < 0) {
			_lastBgmTrack = -1;
		}
		SV(_album); SV(_lastBgmTrack); SV(trackOffset); SV(_sfxVolume); SV(_bgmVolume); SV(_playbackOptions);
	} else {
		SV(_album); SV(_lastBgmTrack); SV(trackOffset); SV(_sfxVolume); SV(_bgmVolume); SV(_playbackOptions);
		if(_lastBgmTrack != -1 && trackOffset > 0) {
			PlayBgmTrack(_lastBgmTrack, trackOffset);
		}
		_oggMixer->SetBgmVolume(_bgmVolume);
		_oggMixer->SetSfxVolume(_sfxVolume);
		_oggMixer->SetPlaybackOptions(_playbackOptions);
	}
}

bool HdAudioDevice::PlayBgmTrack(int trackId, uint32_t startOffset)
{
	auto result = _hdData->BgmFilesById.find(trackId);
	if(result != _hdData->BgmFilesById.end()) {
		if(_oggMixer->Play(result->second.Filename, false, startOffset, result->second.LoopPosition)) {
			_lastBgmTrack = trackId;
			return true;
		}
	} else {
		MessageManager::Log("[HDPack] Invalid album+track combination: " + std::to_string(_album) + ":" + std::to_string(trackId & 0xFF));
	}
	return false;
}

bool HdAudioDevice::PlaySfx(uint8_t sfxNumber)
{
	auto result = _hdData->SfxFilesById.find(_album * 256 + sfxNumber);
	if(result != _hdData->SfxFilesById.end()) {
		return !_oggMixer->Play(result->second, true, 0, 0);
	} else {
		MessageManager::Log("[HDPack] Invalid album+sfx number combination: " + std::to_string(_album) + ":" + std::to_string(sfxNumber));
		return false;
	}
}

void HdAudioDevice::ProcessControlFlags(uint8_t flags)
{
	_oggMixer->SetPausedFlag((flags & 0x01) == 0x01);
	if(flags & 0x02) {
		_oggMixer->StopBgm();
	}
	if(flags & 0x04) {
		_oggMixer->StopSfx();
	}
}

void HdAudioDevice::GetMemoryRanges(MemoryRanges & ranges)
{
	bool useAlternateRegisters = (_hdData->OptionFlags & (int)HdPackOptions::AlternateRegisterRange) == (int)HdPackOptions::AlternateRegisterRange;
	ranges.SetAllowOverride();

	if(useAlternateRegisters) {
		for(int i = 0; i < 7; i++) {
			ranges.AddHandler(MemoryOperation::Write, 0x3002 + i * 0x10);
		}
		ranges.AddHandler(MemoryOperation::Read, 0x4018);
		ranges.AddHandler(MemoryOperation::Read, 0x4019);
	} else {
		ranges.AddHandler(MemoryOperation::Any, 0x4100, 0x4106);
	}
}

void HdAudioDevice::WriteRam(uint16_t addr, uint8_t value)
{
	//$4100/$3002: Playback Options
	//$4101/$3012: Playback Control
	//$4102/$3022: BGM Volume
	//$4103/$3032: SFX Volume
	//$4104/$3042: Album Number
	//$4105/$3052: Play BGM Track
	//$4106/$3062: Play SFX Track
	int regNumber = addr > 0x4100 ? (addr & 0xF) : ((addr & 0xF0) >> 4);

	switch(regNumber) {
		//Playback Options
		//Bit 0: Loop BGM
		//Bit 1-7: Unused, reserved - must be 0
		case 0:
			_playbackOptions = value;
			_oggMixer->SetPlaybackOptions(_playbackOptions);
			break;

		//Playback Control
		//Bit 0: Toggle Pause/Resume (only affects BGM)
		//Bit 1: Stop BGM
		//Bit 2: Stop all SFX
		//Bit 3-7: Unused, reserved - must be 0
		case 1: ProcessControlFlags(value); break;

		//BGM Volume: 0 = mute, 255 = max
		//Also has an immediate effect on currently playing BGM
		case 2: 
			_bgmVolume = value;
			_oggMixer->SetBgmVolume(value);
			break;

		//SFX Volume: 0 = mute, 255 = max
		//Also has an immediate effect on all currently playing SFX
		case 3:
			_sfxVolume = value;
			_oggMixer->SetSfxVolume(value);
			break;

		//Album number: 0-255 (Allows for up to 64k BGM and SFX tracks)
		//No immediate effect - only affects subsequent $4FFE/$4FFF writes
		case 4: _album = value; break;

		//Play BGM track (0-255 = track number)
		//Stop the current BGM and starts a new track
		case 5: _trackError = PlayBgmTrack(_album * 256 + value, 0); break;

		//Play sound effect (0-255 = sfx number)
		//Plays a new sound effect (no limit to the number of simultaneous sound effects)
		case 6: _trackError = PlaySfx(value); break;
	}
}

uint8_t HdAudioDevice::ReadRam(uint16_t addr)
{
	//$4100/$4018: Status
	//$4101/$4019: Revision
	//$4102: 'N' (signature to help detection)
	//$4103: 'E'
	//$4103: 'A'
	switch(addr & 0x7) {
		case 0:
			//Status
			return (
				(_oggMixer->IsBgmPlaying() ? 1 : 0) |
				(_oggMixer->IsSfxPlaying() ? 2 : 0) |
				(_trackError ? 4 : 0)
			);

		case 1: return 1; //Revision
		case 2: return 'N'; //NES
		case 3: return 'E'; //Enhanced
		case 4: return 'A'; //Audio
	}

	return 0;
}
