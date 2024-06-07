#include "pch.h"
#include "PCE/CdRom/PceCdAudioPlayer.h"
#include "PCE/CdRom/PceCdRom.h"
#include "PCE/CdRom/PceCdSeekDelay.h"
#include "PCE/PceTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/CdReader.h"
#include "Utilities/Serializer.h"

PceCdAudioPlayer::PceCdAudioPlayer(Emulator* emu, PceCdRom* cdrom, DiscInfo& disc)
{
	_emu = emu;
	_cdrom = cdrom;
	_disc = &disc;
	_state.Status = CdAudioStatus::Inactive;
}

void PceCdAudioPlayer::Play(uint32_t startSector, bool pause)
{
	int32_t track = _disc->GetTrack(startSector);
	if(track >= 0) {
		uint32_t startLba = _cdrom->GetCurrentSector();
		_seekDelay = (uint32_t)((PceCdSeekDelay::GetSeekTimeMs(startLba, startSector) / 1000.0) * _emu->GetMasterClockRate());

		_state.StartSector = startSector;
		_state.Status = pause ? CdAudioStatus::Paused : CdAudioStatus::Playing;

		_state.EndSector = _disc->GetTrackLastSector(track);
		_state.EndBehavior = CdPlayEndBehavior::Stop;

		_state.CurrentSample = 0;
		_state.CurrentSector = startSector;

		_clockCounter = 0;
	}
}

void PceCdAudioPlayer::SetEndPosition(uint32_t endSector, CdPlayEndBehavior endBehavior)
{
	if(endSector >= _disc->DiscSectorCount) {
		endSector = _disc->DiscSectorCount - 1;
	}

	_state.EndSector = endSector;
	_state.EndBehavior = endBehavior;
	_state.Status = CdAudioStatus::Playing;
}

void PceCdAudioPlayer::PlaySample()
{
	if(_state.Status == CdAudioStatus::Playing) {
		_state.LeftSample = _disc->ReadLeftSample(_state.CurrentSector, _state.CurrentSample);
		_state.RightSample = _disc->ReadRightSample(_state.CurrentSector, _state.CurrentSample);
		_samplesToPlay.push_back(_state.LeftSample);
		_samplesToPlay.push_back(_state.RightSample);
		_state.CurrentSample++;
		if(_state.CurrentSample == 588) {
			//588 samples per 2352-byte sector
			_state.CurrentSample = 0;
			_state.CurrentSector++;

			if(_state.CurrentSector > _state.EndSector) {
				if(_state.CurrentSector >= _disc->DiscSectorCount) {
					_state.CurrentSector = _disc->DiscSectorCount - 1;
				}
				switch(_state.EndBehavior) {
					case CdPlayEndBehavior::Stop: _state.Status = CdAudioStatus::Stopped; break;
					case CdPlayEndBehavior::Loop: _state.CurrentSector = _state.StartSector; break;

					case CdPlayEndBehavior::Irq:
						_state.Status = CdAudioStatus::Stopped;
						_cdrom->ClearIrqSource(PceCdRomIrqSource::DataTransferReady);
						_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferDone);
						break;

				}

			}
		}
	} else {
		_state.LeftSample = 0;
		_state.RightSample = 0;
	}
}

void PceCdAudioPlayer::ProcessAudioPlaybackStart()
{
	_cdrom->ProcessAudioPlaybackStart();
}

void PceCdAudioPlayer::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	double volume = _cdrom->GetAudioFader().GetVolume(PceAudioFaderTarget::CdAudio);
	_resampler.SetVolume(_emu->GetSettings()->GetPcEngineConfig().CdAudioVolume / 100.0 * volume);
	_resampler.SetSampleRates(44100, sampleRate);
	_resampler.Resample<true>(_samplesToPlay.data(), (uint32_t)_samplesToPlay.size() / 2, out, sampleCount, _state.Status == CdAudioStatus::Playing);
	_samplesToPlay.clear();
}

void PceCdAudioPlayer::Serialize(Serializer& s)
{
	SV(_state.Status);
	SV(_state.StartSector);
	SV(_state.EndSector);
	SV(_state.EndBehavior);
	SV(_state.CurrentSector);
	SV(_state.CurrentSample);
	SV(_state.LeftSample);
	SV(_state.RightSample);

	SV(_clockCounter);
}
