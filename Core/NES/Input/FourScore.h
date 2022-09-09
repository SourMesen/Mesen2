#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControllerHub.h"
#include "Utilities/Serializer.h"

class FourScore : public ControllerHub<4>
{
private:
	uint8_t _signature[2] = {};
	uint8_t _sigCounter[2] = {};

protected:
	void Serialize(Serializer &s) override
	{
		ControllerHub::Serialize(s);
		SV(_signature[0]); SV(_signature[1]); SV(_sigCounter[0]); SV(_sigCounter[1]);
	}

	void RefreshStateBuffer() override
	{
		//Signature for port 0 = 0x10, reversed bit order => 0x08
		//Signature for port 1 = 0x20, reversed bit order => 0x04
		if(GetControllerType() == ControllerType::FourScore) {
			_signature[0] = 0x08;
			_signature[1] = 0x04;
		} else {
			//Signature is reversed for Hori 4p adapter
			_signature[0] = 0x04;
			_signature[1] = 0x08;
		}
		_sigCounter[0] = 16;
		_sigCounter[1] = 16;
	}


public:
	FourScore(Emulator* emu, ControllerType type, uint8_t port, ControllerConfig controllers[]) : ControllerHub(emu, type, port, controllers)
	{
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		value &= 0x01;
		ControllerHub::WriteRam(addr, value);
	}
	
	uint8_t ReadRam(uint16_t addr) override
	{
		StrobeProcessRead();
		uint8_t output = 0;
		uint8_t i = addr - 0x4016;

		if(_sigCounter[i] > 0) {
			_sigCounter[i]--;

			if(_sigCounter[i] < 8) {
				i += 2;
			}

			if(_ports[i]) {
				output |= ReadPort(i);
			}
		} else {
			output |= _signature[i] & 0x01;
			_signature[i] = (_signature[i] >> 1) | 0x80;
		}

		output &= 0x01;

		if(GetControllerType() == ControllerType::FourPlayerAdapter) {
			output <<= 1;
		}

		return output;
	}
};
