#pragma once
#include "pch.h"
#include "orfanidis_eq.h"

class Equalizer
{
private:
	unique_ptr<orfanidis_eq::freq_grid> _eqFrequencyGrid;
	unique_ptr<orfanidis_eq::eq1> _equalizerLeft;
	unique_ptr<orfanidis_eq::eq1> _equalizerRight;

	uint32_t _prevSampleRate = 0;
	vector<double> _prevEqualizerGains;

public:
	void ApplyEqualizer(uint32_t sampleCount, int16_t *samples);
	void UpdateEqualizers(vector<double> bandGains, uint32_t sampleRate);
};