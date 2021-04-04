#pragma once
#include "../stdafx.h"
#include "../../Utilities/ISerializable.h"
#include "NesTypes.h"

class NesSoundMixer : public ISerializable
{
public:
	static constexpr uint32_t CycleLength = 10000;
	static constexpr uint32_t BitsPerSample = 16;

private:
	static constexpr uint32_t MaxSampleRate = 96000;
	static constexpr uint32_t MaxSamplesPerFrame = MaxSampleRate / 60 * 4 * 2; //x4 to allow CPU overclocking up to 10x, x2 for panning stereo
	static constexpr uint32_t MaxChannelCount = 11;

public:
	void Reset();
	void AddDelta(AudioChannel channel, uint32_t time, int16_t delta);

	void SetNesModel(NesModel model);

	void PlayAudioBuffer(uint32_t cycle);

	void Serialize(Serializer& s) override;
};