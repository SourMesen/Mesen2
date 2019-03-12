#include "stdafx.h"
#include "SoundMixer.h"
#include "Console.h"
#include "EmuSettings.h"
#include "SoundResampler.h"
#include "RewindManager.h"
#include "../Utilities/Equalizer.h"
#include "../Utilities/blip_buf.h"

SoundMixer::SoundMixer(Console *console)
{
	_console = console;
	_audioDevice = nullptr;
	_resampler.reset(new SoundResampler(console));
	_sampleBuffer = new int16_t[0x10000];
}

SoundMixer::~SoundMixer()
{
	delete[] _sampleBuffer;
}

void SoundMixer::RegisterAudioDevice(IAudioDevice *audioDevice)
{
	_audioDevice = audioDevice;
}

AudioStatistics SoundMixer::GetStatistics()
{
	if(_audioDevice) {
		return _audioDevice->GetStatistics();
	} else {
		return AudioStatistics();
	}
}

void SoundMixer::StopAudio(bool clearBuffer)
{
	if(_audioDevice) {
		if(clearBuffer) {
			_audioDevice->Stop();
		} else {
			_audioDevice->Pause();
		}
	}
}

void SoundMixer::PlayAudioBuffer(int16_t* samples, uint32_t sampleCount)
{
	if(_audioDevice) {
		AudioConfig cfg = _console->GetSettings()->GetAudioConfig();
		
		if(!cfg.EnableAudio) {
			_audioDevice->Stop();
			return;
		}

		if(cfg.EnableEqualizer) {
			ProcessEqualizer(samples, sampleCount);
		}

		if(cfg.MasterVolume != 25) {
			//Apply volume if not using the default value
			for(uint32_t i = 0; i < sampleCount * 2; i++) {
				samples[i] = (int32_t)samples[i] * (int32_t)cfg.MasterVolume / 25;
			}
		}

		shared_ptr<RewindManager> rewindManager = _console->GetRewindManager();
		if(rewindManager && rewindManager->SendAudio(samples, (uint32_t)sampleCount)) {
			if(cfg.SampleRate == SoundResampler::SpcSampleRate && cfg.DisableDynamicSampleRate) {
				_audioDevice->PlayBuffer(samples, sampleCount, cfg.SampleRate, true);
			} else {
				uint32_t resampledCount = _resampler->Resample(samples, sampleCount, cfg.SampleRate, _sampleBuffer);
				_audioDevice->PlayBuffer(_sampleBuffer, resampledCount, cfg.SampleRate, true);
			}
		}
		_audioDevice->ProcessEndOfFrame();
	}
}

void SoundMixer::ProcessEqualizer(int16_t* samples, uint32_t sampleCount)
{
	AudioConfig cfg = _console->GetSettings()->GetAudioConfig();
	if(!_equalizer) {
		_equalizer.reset(new Equalizer());
	}
	vector<double> bandGains = {
		cfg.Band1Gain, cfg.Band2Gain, cfg.Band3Gain, cfg.Band4Gain, cfg.Band5Gain,
		cfg.Band6Gain, cfg.Band7Gain, cfg.Band8Gain, cfg.Band9Gain, cfg.Band10Gain,
		cfg.Band11Gain, cfg.Band12Gain, cfg.Band13Gain, cfg.Band14Gain, cfg.Band15Gain,
		cfg.Band16Gain, cfg.Band17Gain, cfg.Band18Gain, cfg.Band19Gain, cfg.Band20Gain
	};
	_equalizer->UpdateEqualizers(bandGains, SoundResampler::SpcSampleRate);
	_equalizer->ApplyEqualizer(sampleCount, samples);
}

double SoundMixer::GetRateAdjustment()
{
	return _resampler->GetRateAdjustment();
}