#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class Emulator;

class BaseEeprom24C0X : public ISerializable
{
protected:
	enum class Mode
	{
		Idle = 0,
		Address = 1,
		Read = 2,
		Write = 3,
		SendAck = 4,
		WaitAck = 5,
		ChipAddress = 6
	};

	Emulator* _emu = nullptr;

	Mode _mode = Mode::Idle;
	Mode _nextMode = Mode::Idle;
	uint8_t _chipAddress = 0;
	uint8_t _address = 0;
	uint8_t _data = 0;
	uint8_t _counter = 0;
	uint8_t _output = 0;
	uint8_t _prevScl = 0;
	uint8_t _prevSda = 0;
	uint8_t _romData[256] = {};

	void Serialize(Serializer& s) override
	{
		SVArray(_romData, 256);
		SV(_mode);
		SV(_nextMode);
		SV(_chipAddress);
		SV(_address);
		SV(_data);
		SV(_counter);
		SV(_output);
		SV(_prevScl);
		SV(_prevSda);
	}

public:
	virtual ~BaseEeprom24C0X() = default;

	virtual void Write(uint8_t scl, uint8_t sda) = 0;
	virtual void SaveBattery() = 0;
	
	uint8_t Read()
	{
		return _output;
	}

	void WriteScl(uint8_t scl)
	{
		Write(scl, _prevSda);
	}

	void WriteSda(uint8_t sda)
	{
		Write(_prevScl, sda);
	}
};

