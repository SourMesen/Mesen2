#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"

static constexpr double PI = 3.14159265358979323846;

class OnePoleLowPassFilter : public ISerializable
{
private:
	double _a0 = 1;
	double _b1 = 0;
	double _prevSample = 0;

public:
	void SetCutoffFrequency(double freq, double sampleRate)
	{
		_b1 = std::exp(-2.0 * PI * (freq / sampleRate));
		_a0 = 1.0 - _b1;
	}

	double Process(double nextSample)
	{
		return _prevSample = nextSample * _a0 + _prevSample * _b1;
	}

	void Serialize(Serializer& s) override
	{
		SV(_a0);
		SV(_b1);
		SV(_prevSample);
	}
};