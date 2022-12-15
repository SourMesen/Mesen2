#pragma once
#include "pch.h"
#include "NES/Input/NesController.h"

class PachinkoController : public NesController
{
private:
	uint8_t _analogData = 0;
	uint16_t _pachinkoState = 0;

protected:
	enum PachinkoButtons { Press = 8, Release = 9 };
	
	void Serialize(Serializer& s) override
	{
		NesController::Serialize(s);
		SV(_pachinkoState); SV(_analogData);
	}

	void InternalSetStateFromInput() override
	{
		NesController::InternalSetStateFromInput();

		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(PachinkoButtons::Press, keyMapping.CustomKeys[0]);
			SetPressedState(PachinkoButtons::Release, keyMapping.CustomKeys[1]);
		}
	}

public:
	PachinkoController(Emulator* emu, KeyMappingSet keyMappings) : NesController(emu, ControllerType::Pachinko, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;
		if(addr == 0x4016) {
			StrobeProcessRead();
			output = (_pachinkoState & 0x01) << 1;
			_pachinkoState >>= 1;
		}
		return output;
	}

	void RefreshStateBuffer() override
	{
		if(_analogData < 0x63 && IsPressed(PachinkoController::PachinkoButtons::Press)) {
			_analogData++;
		} else if(_analogData > 0 && IsPressed(PachinkoController::PachinkoButtons::Release)) {
			_analogData--;
		}

		uint8_t analogData =
			((_analogData & 0x01) << 7) |
			((_analogData & 0x02) << 5) |
			((_analogData & 0x04) << 3) |
			((_analogData & 0x08) << 1) |
			((_analogData & 0x10) >> 1) |
			((_analogData & 0x20) >> 3) |
			((_analogData & 0x40) >> 5) |
			((_analogData & 0x80) >> 7);

		NesController::RefreshStateBuffer();
		_pachinkoState = (GetControllerStateBuffer() & 0xFF) | (~analogData << 8);
	}
};