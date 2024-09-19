#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

class Emulator;
class GbaRtc;

struct GbaGpioState
{
	uint8_t Data;
	uint8_t WritablePins;
	bool ReadWrite;
};

class GbaGpio final : public ISerializable
{
private:
	GbaGpioState _state = {};
	GbaRtc* _rtc = nullptr;

public:
	GbaGpio(GbaRtc* rtc);

	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};