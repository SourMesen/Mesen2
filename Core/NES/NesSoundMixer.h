#pragma once
#include "../stdafx.h"
#include "../../Utilities/ISerializable.h"
#include "NesTypes.h"

class NesSoundMixer : public ISerializable
{
public:
	void Reset();
	void AddDelta(AudioChannel channel, uint32_t time, int16_t delta);

	void Serialize(Serializer& s) override;
};