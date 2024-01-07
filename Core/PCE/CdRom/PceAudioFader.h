#pragma once
#include "pch.h"
#include "PCE/PceTypes.h"
#include "Utilities/ISerializable.h"

class PceConsole;

class PceAudioFader : public ISerializable
{
private:
	PceConsole* _console = nullptr;
	PceAudioFaderState _state = {};

public:
	PceAudioFader(PceConsole* console);

	PceAudioFaderState& GetState() { return _state; }

	uint8_t Read();
	void Write(uint8_t value);
	double GetVolume(PceAudioFaderTarget target);
	void Serialize(Serializer& s);
};