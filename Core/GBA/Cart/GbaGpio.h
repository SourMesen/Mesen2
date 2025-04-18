#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Utilities/ISerializable.h"

class Emulator;
class GbaRtc;

class GbaGpio final : public ISerializable
{
private:
	GbaGpioState _state = {};
	GbaRtc* _rtc = nullptr;

	void UpdateDataPins();

public:
	GbaGpio(GbaRtc* rtc);

	GbaGpioState& GetState() { return _state; }

	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};