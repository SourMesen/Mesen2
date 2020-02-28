#include "stdafx.h"
#include "SoundResampler.h"
#include "Console.h"
#include "Spc.h"
#include "EmuSettings.h"
#include "SoundMixer.h"
#include "VideoRenderer.h"
#include "../Utilities/blip_buf.h"

SoundResampler::SoundResampler(Console *console)
{
	_console = console;
	_blipBufLeft = blip_new(Spc::SpcSampleRate);
	_blipBufRight = blip_new(Spc::SpcSampleRate);
}

SoundResampler::~SoundResampler()
{
	blip_delete(_blipBufLeft);
	blip_delete(_blipBufRight);
}

double SoundResampler::GetRateAdjustment()
{
	return _rateAdjustment;
}

double SoundResampler::GetTargetRateAdjustment()
{
	AudioConfig cfg = _console->GetSettings()->GetAudioConfig();
	bool isRecording = _console->GetSoundMixer()->IsRecording() || _console->GetVideoRenderer()->IsRecording();
	if(!isRecording && !cfg.DisableDynamicSampleRate) {
		//Don't deviate from selected sample rate while recording
		//TODO: Have 2 output streams (one for recording, one for the speakers)
		AudioStatistics stats = _console->GetSoundMixer()->GetStatistics();

		if(stats.AverageLatency > 0 && _console->GetSettings()->GetEmulationSpeed() == 100) {
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

void SoundResampler::UpdateTargetSampleRate(uint32_t sampleRate)
{
	uint32_t spcSampleRate = Spc::SpcSampleRate;
	if(_console->GetSettings()->GetVideoConfig().IntegerFpsMode) {
		//Adjust sample rate when running at 60.0 fps instead of 60.1
		switch(_console->GetRegion()) {
			default:
			case ConsoleRegion::Ntsc: spcSampleRate = (uint32_t)(Spc::SpcSampleRate * (60.0 / 60.0988118623484)); break;
			case ConsoleRegion::Pal: spcSampleRate = (uint32_t)(Spc::SpcSampleRate * (50.0 / 50.00697796826829)); break;
		}
	}

	double targetRate = sampleRate * GetTargetRateAdjustment();
	if(targetRate != _previousTargetRate || spcSampleRate != _prevSpcSampleRate) {
		blip_set_rates(_blipBufLeft, spcSampleRate, targetRate);
		blip_set_rates(_blipBufRight, spcSampleRate, targetRate);
		_previousTargetRate = targetRate;
		_prevSpcSampleRate = spcSampleRate;
	}
}

uint32_t SoundResampler::Resample(int16_t *inSamples, uint32_t sampleCount, uint32_t sampleRate, int16_t *outSamples)
{
	UpdateTargetSampleRate(sampleRate);

	blip_add_delta(_blipBufLeft, 0, inSamples[0] - _lastSampleLeft);
	blip_add_delta(_blipBufRight, 0, inSamples[1] - _lastSampleRight);

	for(uint32_t i = 1; i < sampleCount; i++) {
		blip_add_delta(_blipBufLeft, i, inSamples[i * 2] - inSamples[i * 2 - 2]);
		blip_add_delta(_blipBufRight, i, inSamples[i * 2 + 1] - inSamples[i * 2 - 1]);
	}

	_lastSampleLeft = inSamples[(sampleCount - 1) * 2];
	_lastSampleRight = inSamples[(sampleCount - 1) * 2 + 1];

	blip_end_frame(_blipBufLeft, sampleCount);
	blip_end_frame(_blipBufRight, sampleCount);

	uint32_t resampledCount = blip_read_samples(_blipBufLeft, outSamples, Spc::SpcSampleRate, true);
	blip_read_samples(_blipBufRight, outSamples + 1, Spc::SpcSampleRate, true);

	return resampledCount;
}