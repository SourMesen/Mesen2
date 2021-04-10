#include "stdafx.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundResampler.h"
#include "Shared/RewindManager.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/Audio/WaveRecorder.h"
#include "Shared/Interfaces/IAudioProvider.h"
#include "Utilities/Equalizer.h"

SoundMixer::SoundMixer(Emulator* emu)
{
	_emu = emu;
	_audioDevice = nullptr;
	_resampler.reset(new SoundResampler(emu));
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

void SoundMixer::RegisterAudioProvider(IAudioProvider* provider)
{
	_audioProviders.push_back(provider);
}

void SoundMixer::UnregisterAudioProvider(IAudioProvider* provider)
{
	vector<IAudioProvider*>& vec = _audioProviders;
	vec.erase(std::remove(vec.begin(), vec.end(), provider), vec.end());
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

void SoundMixer::PlayAudioBuffer(int16_t* samples, uint32_t sampleCount, uint32_t sourceRate)
{
	AudioConfig cfg = _emu->GetSettings()->GetAudioConfig();

	uint32_t masterVolume = cfg.MasterVolume;
	if(_emu->GetSettings()->CheckFlag(EmulationFlags::InBackground)) {
		if(cfg.MuteSoundInBackground) {
			masterVolume = 0;
		} else if(cfg.ReduceSoundInBackground) {
			masterVolume = cfg.VolumeReduction == 100 ? 0 : masterVolume * (100 - cfg.VolumeReduction) / 100;
		}
	} else if(cfg.ReduceSoundInFastForward && _emu->GetSettings()->CheckFlag(EmulationFlags::TurboOrRewind)) {
		masterVolume = cfg.VolumeReduction == 100 ? 0 : masterVolume * (100 - cfg.VolumeReduction) / 100;
	}

	_leftSample = samples[0];
	_rightSample = samples[1];

	int16_t *out = _sampleBuffer;
	uint32_t count = _resampler->Resample(samples, sampleCount, sourceRate, cfg.SampleRate, out);

	uint32_t targetRate = (uint32_t)(cfg.SampleRate * _resampler->GetRateAdjustment());
	for(IAudioProvider* provider : _audioProviders) 	{
		provider->MixAudio(out, count, targetRate);
	}

	if(cfg.EnableEqualizer) {
		ProcessEqualizer(samples, sampleCount);
	}

	if(masterVolume < 100) {
		//Apply volume if not using the default value
		for(uint32_t i = 0; i < count * 2; i++) {
			out[i] = (int32_t)out[i] * (int32_t)masterVolume / 100;
		}
	}

	shared_ptr<RewindManager> rewindManager = _emu->GetRewindManager();
	if(!_emu->IsRunAheadFrame() && rewindManager && rewindManager->SendAudio(out, count)) {
		bool isRecording = _waveRecorder || _emu->GetVideoRenderer()->IsRecording();
		if(isRecording) {
			if(_waveRecorder) {
				_waveRecorder->WriteSamples(out, count, cfg.SampleRate, true);
			}
			_emu->GetVideoRenderer()->AddRecordingSound(out, count, cfg.SampleRate);
		}

		if(_audioDevice) {
			if(!cfg.EnableAudio) {
				_audioDevice->Stop();
				return;
			}

			_audioDevice->PlayBuffer(out, count, cfg.SampleRate, true);
			_audioDevice->ProcessEndOfFrame();
		}
	}
}

void SoundMixer::ProcessEqualizer(int16_t* samples, uint32_t sampleCount)
{
	AudioConfig cfg = _emu->GetSettings()->GetAudioConfig();
	if(!_equalizer) {
		_equalizer.reset(new Equalizer());
	}
	vector<double> bandGains = {
		cfg.Band1Gain, cfg.Band2Gain, cfg.Band3Gain, cfg.Band4Gain, cfg.Band5Gain,
		cfg.Band6Gain, cfg.Band7Gain, cfg.Band8Gain, cfg.Band9Gain, cfg.Band10Gain,
		cfg.Band11Gain, cfg.Band12Gain, cfg.Band13Gain, cfg.Band14Gain, cfg.Band15Gain,
		cfg.Band16Gain, cfg.Band17Gain, cfg.Band18Gain, cfg.Band19Gain, cfg.Band20Gain
	};
	
	_equalizer->UpdateEqualizers(bandGains, cfg.SampleRate);
	_equalizer->ApplyEqualizer(sampleCount, samples);
}

double SoundMixer::GetRateAdjustment()
{
	return _resampler->GetRateAdjustment();
}

void SoundMixer::StartRecording(string filepath)
{
	_waveRecorder.reset(new WaveRecorder(filepath, _emu->GetSettings()->GetAudioConfig().SampleRate, true));
}

void SoundMixer::StopRecording()
{
	_waveRecorder.reset();
}

bool SoundMixer::IsRecording()
{
	return _waveRecorder.get() != nullptr;
}

void SoundMixer::GetLastSamples(int16_t &left, int16_t &right)
{
	left = _leftSample;
	right = _rightSample;
}