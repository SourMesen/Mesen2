#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

class JissenMahjongController : public BaseControlDevice
{
private:
	uint8_t _row = 0;
	uint32_t _stateBuffer = 0;

protected:
	enum Buttons { A = 0, B, C, D, E, F, G, H, I, J, K, L, M, N, Select, Start, Kan, Pon, Chii, Riichi, Ron };

	string GetKeyNames() override
	{
		return "ABCDEFGHIJKLMNSTkpcir";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			for(int i = 0; i < 21; i++) {
				SetPressedState(i, keyMapping.CustomKeys[i]);
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_row); SV(_stateBuffer);
	}

public:
	JissenMahjongController(Emulator* emu, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::JissenMahjong, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			StrobeProcessRead();
			uint8_t value = (_stateBuffer & 0x01) << 1;
			_stateBuffer >>= 1;
			return value;
		}
		return 0;
	}

	void RefreshStateBuffer() override
	{
		switch(_row) {
			default:
			case 0:
				_stateBuffer = 0;
				break;

			case 1:
				_stateBuffer =
					(IsPressed(JissenMahjongController::Buttons::N) ? 0x04 : 0) |
					(IsPressed(JissenMahjongController::Buttons::M) ? 0x08 : 0) |
					(IsPressed(JissenMahjongController::Buttons::L) ? 0x10 : 0) |
					(IsPressed(JissenMahjongController::Buttons::K) ? 0x20 : 0) |
					(IsPressed(JissenMahjongController::Buttons::J) ? 0x40 : 0) |
					(IsPressed(JissenMahjongController::Buttons::I) ? 0x80 : 0);
				break;

			case 2:
				_stateBuffer =
					(IsPressed(JissenMahjongController::Buttons::H) ? 0x01 : 0) |
					(IsPressed(JissenMahjongController::Buttons::G) ? 0x02 : 0) |
					(IsPressed(JissenMahjongController::Buttons::F) ? 0x04 : 0) |
					(IsPressed(JissenMahjongController::Buttons::E) ? 0x08 : 0) |
					(IsPressed(JissenMahjongController::Buttons::D) ? 0x10 : 0) |
					(IsPressed(JissenMahjongController::Buttons::C) ? 0x20 : 0) |
					(IsPressed(JissenMahjongController::Buttons::B) ? 0x40 : 0) |
					(IsPressed(JissenMahjongController::Buttons::A) ? 0x80 : 0);
				break;

			case 3:
				_stateBuffer =
					(IsPressed(JissenMahjongController::Buttons::Ron) ? 0x02 : 0) |
					(IsPressed(JissenMahjongController::Buttons::Riichi) ? 0x04 : 0) |
					(IsPressed(JissenMahjongController::Buttons::Chii) ? 0x08 : 0) |
					(IsPressed(JissenMahjongController::Buttons::Pon) ? 0x10 : 0) |
					(IsPressed(JissenMahjongController::Buttons::Kan) ? 0x20 : 0) |
					(IsPressed(JissenMahjongController::Buttons::Start) ? 0x40 : 0) |
					(IsPressed(JissenMahjongController::Buttons::Select) ? 0x80 : 0);
				break;
		}
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_row = (value & 0x6) >> 1;
		StrobeProcessWrite(value);
	}
};