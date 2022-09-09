#include "pch.h"
#include "ReverbFilter.h"

void ReverbFilter::ResetFilter()
{
	for(int i = 0; i < 5; i++) {
		_delay[i].Reset();
	}
}

void ReverbFilter::ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, uint32_t sampleRate, double reverbStrength, double reverbDelay)
{
	for(int i = 0; i < 2; i++) {
		_delay[i*5].SetParameters(550 * reverbDelay, 0.25 * reverbStrength, sampleRate);
		_delay[i*5+1].SetParameters(330 * reverbDelay, 0.15 * reverbStrength, sampleRate);
		_delay[i*5+2].SetParameters(485 * reverbDelay, 0.12 * reverbStrength, sampleRate);
		_delay[i*5+3].SetParameters(150 * reverbDelay, 0.20 * reverbStrength, sampleRate);
		_delay[i*5+4].SetParameters(285 * reverbDelay, 0.05 * reverbStrength, sampleRate);
	}

	for(int i = 0; i < 5; i++) {
		_delay[i].ApplyReverb(stereoBuffer, sampleCount);
		_delay[i+5].ApplyReverb(stereoBuffer+1, sampleCount);
	}
	for(int i = 0; i < 5; i++) {
		_delay[i].AddSamples(stereoBuffer, sampleCount);
		_delay[i+5].AddSamples(stereoBuffer+1, sampleCount);
	}
}