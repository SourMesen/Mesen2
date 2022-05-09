#include "stdafx.h"
#include "PCE/PceCdAudioPlayer.h"
#include "PCE/PceCdRom.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/CdReader.h"

PceCdAudioPlayer::PceCdAudioPlayer(Emulator* emu, PceCdRom* cdrom, DiscInfo& disc)
{
	_emu = emu;
	_cdrom = cdrom;
	_disc = &disc;
}

void PceCdAudioPlayer::Play(uint32_t startSector)
{
	int32_t track = _disc->GetTrack(startSector);
	if(track >= 0) {
		_startSector = startSector;
		_playing = true;
		_clockCounter = 0;

		_endSector = _disc->GetTrackLastSector(track);
		_endBehavior = CdPlayEndBehavior::Stop;

		_currentSample = 0;
		_currentSector = startSector;
	}
}

void PceCdAudioPlayer::SetEndPosition(uint32_t endSector, CdPlayEndBehavior endBehavior)
{
	_endSector = endSector;
	_endBehavior = endBehavior;
	_playing = true;
}

void PceCdAudioPlayer::Stop()
{
	_playing = false;
}

void PceCdAudioPlayer::PlaySample()
{
	if(_playing) {
		_leftSample = _disc->ReadLeftSample(_currentSector, _currentSample);
		_rightSample = _disc->ReadRightSample(_currentSector, _currentSample);
		_samplesToPlay.push_back(_leftSample);
		_samplesToPlay.push_back(_rightSample);
		_currentSample++;
		if(_currentSample == 588) {
			//588 samples per 2352-byte sector
			_currentSample = 0;
			_currentSector++;

			if(_currentSector > _endSector) {
				switch(_endBehavior) {
					case CdPlayEndBehavior::Stop: _playing = false; break;
					case CdPlayEndBehavior::Loop: _currentSector = _startSector; break;

					case CdPlayEndBehavior::Irq:
						_playing = false;
						_cdrom->ClearIrqSource(PceCdRomIrqSource::DataTransferReady);
						_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferDone);
						break;

				}

			}
		}
	} else {
		_leftSample = 0;
		_rightSample = 0;
	}
}

void PceCdAudioPlayer::Exec()
{
	_clockCounter += 3;
	if(_clockCounter > 487) {
		//Output one sample every 487 master clocks (~44101.1hz)
		PlaySample();
		_clockCounter -= 487;
	}
}

void PceCdAudioPlayer::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	_resampler.SetVolume(_emu->GetSettings()->GetPcEngineConfig().CdAudioVolume / 100.0);
	_resampler.SetSampleRates(44100, sampleRate);
	_resampler.Resample<true>(_samplesToPlay.data(), (uint32_t)_samplesToPlay.size() / 2, out, sampleCount);
	_samplesToPlay.clear();
}
