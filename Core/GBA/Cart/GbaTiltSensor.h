#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"
#include "Shared/KeyManager.h"
#include "Shared/BaseControlDevice.h"

class GbaTiltSensor : public BaseControlDevice
{
private:
	vector<uint16_t> _xAxisCodes;
	vector<uint16_t> _yAxisCodes;

	uint16_t _xAccel = 0x8000;
	uint16_t _yAccel = 0x8000;
	bool _latched = false;

protected:
	bool HasCoordinates() override { return true; }

	void InternalSetStateFromInput() override
	{
		MouseMovement mov = {};

		for(uint16_t code : _xAxisCodes) {
			if(code != 0) {
				optional<int16_t> xAxis = KeyManager::GetAxisPosition(code);
				if(xAxis.has_value()) {
					mov.dx = xAxis.value();
				}
			}
		}

		for(uint16_t code : _yAxisCodes) {
			if(code != 0) {
				optional<int16_t> yAxis = KeyManager::GetAxisPosition(code);
				if(yAxis.has_value()) {
					mov.dy = yAxis.value();
				}
			}
		}

		SetMovement(mov);
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_xAccel);
		SV(_yAccel);
		SV(_latched);
	}

public:
	GbaTiltSensor(Emulator* emu) : BaseControlDevice(emu, ControllerType::GameboyAccelerometer, BaseControlDevice::MapperInputPort)
	{
		//TODO add configuration in UI
		_xAxisCodes = {
			KeyManager::GetKeyCode("Pad1 LT X"),
			KeyManager::GetKeyCode("Joy1 X"),
			KeyManager::GetKeyCode("Pad1 X"),
		};

		_yAxisCodes = {
			KeyManager::GetKeyCode("Pad1 LT Y"),
			KeyManager::GetKeyCode("Joy1 Y"),
			KeyManager::GetKeyCode("Pad1 Y"),
		};
	}

	uint8_t ReadRam(uint16_t addr) override { return 0; }
	void WriteRam(uint16_t addr, uint8_t value) override {}

	uint8_t Read(uint32_t addr)
	{
		switch(addr & 0xF00) {
			case 0x200: return _xAccel & 0xFF;
			case 0x300: return 0x80 | ((_xAccel >> 8) & 0x0F);
			case 0x400: return _yAccel & 0xFF;
			case 0x500: return (_yAccel >> 8) & 0x0F;
		}

		return 0;
	}

	void Write(uint32_t addr, uint8_t value)
	{
		switch(addr & 0xF00) {
			case 0x000:
				if(value == 0x55) {
					//Reset accelerometer latch
					_xAccel = 0x8000;
					_yAccel = 0x8000;
					_latched = false;
				}
				break;

			case 0x100:
				if(!_latched && value == 0xAA) {
					//Latch accelerometer value
					MouseMovement mov = GetMovement();
					_xAccel = -(mov.dx / 143) + 0x392;
					_yAccel = mov.dy / 143 + 0x3A0;
					_latched = true;
				}
				break;
		}
	}
};
