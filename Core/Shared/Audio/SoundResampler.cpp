#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Audio/SoundResampler.h"
#include "Shared/Video/VideoRenderer.h"
#include "Utilities/Audio/HermiteResampler.h"

SoundResampler::SoundResampler(Emulator* emu)
{
	_emu = emu;
}

SoundResampler::~SoundResampler()
{
}

double SoundResampler::GetRateAdjustment()
{
	return _rateAdjustment;
}

uint32_t SoundResampler::GetTargetRate()
{
	return (uint32_t)_previousTargetRate;
}


double SoundResampler::GetTargetRateAdjustment()
{
	AudioConfig cfg = _emu->GetSettings()->GetAudioConfig();
	bool isRecording = _emu->GetSoundMixer()->IsRecording() || _emu->GetVideoRenderer()->IsRecording();
	if(!isRecording && !cfg.DisableDynamicSampleRate) {
		//Don't deviate from selected sample rate while recording
		//TODO: Have 2 output streams (one for recording, one for the speakers)
		AudioStatistics stats = _emu->GetSoundMixer()->GetStatistics();

		if(stats.AverageLatency > 0 && _emu->GetSettings()->GetEmulationSpeed() == 100) {
			//Try to stay within +/- 3ms of requested latency
			constexpr int32_t maxGap = 3;
			constexpr int32_t maxSubAdjustment = 3600;

			int32_t requestedLatency = (int32_t)cfg.AudioLatency;
			double latencyGap = stats.AverageLatency - requestedLatency;
			double adjustment = std::min(0.0025, (std::ceil((std::abs(latencyGap) - maxGap) * 8)) * 0.00003125);

			if(latencyGap < 0 && _underTarget < maxSubAdjustment) {
				_underTarget++;
			} else if(latencyGap > 0 && _underTarget > -maxSubAdjustment) {
				_underTarget--;
			}

			//For every ~1 second spent under/over target latency, further adjust rate (GetTargetRate is called approx. 3x per frame) 
			//This should slowly get us closer to the actual output rate of the sound card
			double subAdjustment = 0.00003125 * _underTarget / 180;

			if(adjustment > 0) {
				if(latencyGap > maxGap) {
					_rateAdjustment = 1 - adjustment + subAdjustment;
				} else if(latencyGap < -maxGap) {
					_rateAdjustment = 1 + adjustment + subAdjustment;
				}
			} else if(std::abs(latencyGap) < 1) {
				//Restore normal rate once we get within +/- 1ms
				_rateAdjustment = 1.0 + subAdjustment;
			}
		} else {
			_underTarget = 0;
			_rateAdjustment = 1.0;
		}
	} else {
		_underTarget = 0;
		_rateAdjustment = 1.0;
	}
	return _rateAdjustment;
}

void SoundResampler::UpdateTargetSampleRate(uint32_t sourceRate, uint32_t sampleRate)
{
	double inputRate = sourceRate;
	
	if(_emu->GetSettings()->GetVideoConfig().IntegerFpsMode) {
		//Adjust input sample rate when using integer fps values
		double baseFps = _emu->GetConsoleUnsafe()->GetFps();
		double roundedFps = _emu->GetFps();
		inputRate = sourceRate * (roundedFps / baseFps);
	}

	double targetRate = sampleRate * GetTargetRateAdjustment();
	if(targetRate != _previousTargetRate || inputRate != _prevInputRate) {
		_previousTargetRate = targetRate;
		_prevInputRate = inputRate;
		_resampler.SetSampleRates(inputRate, targetRate);
	}
}

uint32_t SoundResampler::Resample(int16_t *inSamples, uint32_t sampleCount, uint32_t sourceRate, uint32_t sampleRate, int16_t *outSamples, uint32_t maxOutCount)
{
	UpdateTargetSampleRate(sourceRate, sampleRate);
	return _resampler.Resample<false>(inSamples, sampleCount, outSamples, maxOutCount);
}