#include "pch.h"
#include "HermiteResampler.h"

//Adapted from http://paulbourke.net/miscellaneous/interpolation/
//Original author: Paul Bourke ("Any source code found here may be freely used provided credits are given to the author.")
int16_t HermiteResampler::HermiteInterpolate(double values[4], double mu)
{
	double m0, m1, mu2, mu3;
	double a0, a1, a2, a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (values[1] - values[0]) / 2 + (values[2] - values[1]) / 2;
	m1 = (values[2] - values[1]) / 2 + (values[3] - values[2]) / 2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;

	double output = a0 * values[1] + a1 * m0 + a2 * m1 + a3 * values[2];
	return (int16_t)std::clamp(output, -32768.0, 32767.0);
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

void HermiteResampler::SetVolume(double volume)
{
	_volume = (int32_t)(volume * 256);
}

void HermiteResampler::SetSampleRates(double srcRate, double dstRate)
{
	_rateRatio = srcRate / dstRate;
}

uint32_t HermiteResampler::GetPendingCount()
{
	return (uint32_t)_pendingSamples.size() / 2;
}

template<bool addMode>
void HermiteResampler::WriteSample(int16_t* out, uint32_t pos, int16_t left, int16_t right)
{
	if(addMode) {
		out[pos] = (int16_t)std::clamp<int32_t>(out[pos] + ((left * _volume) >> 8), INT16_MIN, INT16_MAX);
		out[pos + 1] = (int16_t)std::clamp<int32_t>(out[pos + 1] + ((right * _volume) >> 8), INT16_MIN, INT16_MAX);
	} else {
		out[pos] = (int16_t)std::clamp<int32_t>((left * _volume) >> 8, INT16_MIN, INT16_MAX);
		out[pos + 1] = (int16_t)std::clamp<int32_t>((right * _volume) >> 8, INT16_MIN, INT16_MAX);
	}
}

template<bool addMode>
uint32_t HermiteResampler::Resample(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount, bool fillToMax)
{
	maxOutSampleCount *= 2;
	if(_pendingSamples.size() >= maxOutSampleCount) {
		_pendingSamples.clear();
	}

	uint32_t outPos = (uint32_t)_pendingSamples.size();
	for(uint32_t i = 0; i < outPos; i += 2) {
		WriteSample<addMode>(out, i, _pendingSamples[i], _pendingSamples[i + 1]);
	}
	_pendingSamples.clear();

	if(_rateRatio == 1.0) {
		if(inSampleCount > 0) {
			uint32_t count = std::min((uint32_t)maxOutSampleCount - outPos, inSampleCount * 2);
			memcpy(out+outPos, in, count * sizeof(int16_t));
			for(uint32_t i = count; i < inSampleCount * 2; i += 2) {
				_pendingSamples.push_back(in[i]);
				_pendingSamples.push_back(in[i + 1]);
			}
			_left = in[inSampleCount * 2 - 2];
			_right = in[inSampleCount * 2 - 1];
			outPos += count;
		}
	} else {
		for(uint32_t i = 0; i < inSampleCount * 2; i += 2) {
			while(_fraction <= 1.0) {
				//Generate interpolated samples until we have enough samples for the current source sample
				_left = HermiteInterpolate(_prevLeft, _fraction);
				_right = HermiteInterpolate(_prevRight, _fraction);
				if(outPos <= maxOutSampleCount - 2) {
					WriteSample<addMode>(out, outPos, _left, _right);
					outPos += 2;
				} else {
					_pendingSamples.push_back(_left);
					_pendingSamples.push_back(_right);
				}

				_fraction += _rateRatio;
			}

			//Move to the next source sample
			PushSample(_prevLeft, in[i]);
			PushSample(_prevRight, in[i + 1]);
			_fraction -= 1.0;
		}
	}

	if(fillToMax) {
		while(outPos < maxOutSampleCount) {
			WriteSample<addMode>(out, outPos, _left, _right);
			outPos += 2;
		}
	}

	return outPos / 2;
}

template uint32_t HermiteResampler::Resample<true>(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount, bool fillToMax);
template uint32_t HermiteResampler::Resample<false>(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount, bool fillToMax);