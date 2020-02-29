#include "stdafx.h"
#include "HermiteResampler.h"

//Adapted from http://paulbourke.net/miscellaneous/interpolation/
//Original author: Paul Bourke ("Any source code found here may be freely used provided credits are given to the author.")
int16_t HermiteResampler::HermiteInterpolate(double values[4], double mu)
{
	constexpr double tension = 0; //Tension: 1 is high, 0 normal, -1 is low
	constexpr double bias = 0; //Bias: 0 is even, positive is towards first segment, negative towards the other

	double m0, m1, mu2, mu3;
	double a0, a1, a2, a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (values[1] - values[0]) * (1 + bias) * (1 - tension) / 2;
	m0 += (values[2] - values[1]) * (1 - bias) * (1 - tension) / 2;
	m1 = (values[2] - values[1]) * (1 + bias) * (1 - tension) / 2;
	m1 += (values[3] - values[2]) * (1 - bias) * (1 - tension) / 2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;

	double output = a0 * values[1] + a1 * m0 + a2 * m1 + a3 * values[2];
	return (int16_t)std::max(std::min(output, 32767.0), -32768.0);
}

void HermiteResampler::PushSample(double prevValues[4], int16_t sample)
{
	prevValues[0] = prevValues[1];
	prevValues[1] = prevValues[2];
	prevValues[2] = prevValues[3];
	prevValues[3] = (double)sample;
}

void HermiteResampler::Reset()
{
	for(int i = 0; i < 4; i++) {
		_prevLeft[i] = 0.0;
		_prevRight[i] = 0.0;
	}
	_fraction = 0.0;
}

void HermiteResampler::SetSampleRates(double srcRate, double dstRate)
{
	_rateRatio = srcRate / dstRate;
}

uint32_t HermiteResampler::Resample(int16_t* in, uint32_t inSampleCount, int16_t* out)
{
	if(_rateRatio == 1.0) {
		memcpy(out, in, inSampleCount * 2 * sizeof(int16_t));
		return inSampleCount;
	}

	uint32_t outPos = 0;

	for(uint32_t i = 0; i < inSampleCount * 2; i += 2) {
		while(_fraction <= 1.0) {
			//Generate interpolated samples until we have enough samples for the current source sample
			out[outPos] = HermiteInterpolate(_prevLeft, _fraction);
			out[outPos + 1] = HermiteInterpolate(_prevRight, _fraction);
			outPos += 2;
			_fraction += _rateRatio;
		}

		//Move to the next source sample
		PushSample(_prevLeft, in[i]);
		PushSample(_prevRight, in[i + 1]);
		_fraction -= 1.0;
	}

	return outPos / 2;
}
