#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"

class VirtualBoyController : public BaseControlDevice
{
private:
	uint32_t _stateBuffer = 0;

protected:
	enum Buttons { Down1 = 0, Left1, Select, Start, Up0, Down0, Left0, Right0, Right1, Up1, L, R, B, A };

	string GetKeyNames() override
	{
		return "dlSTUDLRruLRBA";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			for(int i = 0; i < 14; i++) 			{
				SetPressedState(i, keyMapping.CustomKeys[i]);
			}

			if(!_emu->GetSettings()->GetNesConfig().AllowInvalidInput) {
				//If both U+D or L+R are pressed at the same time, act as if neither is pressed
				if(IsPressed(Buttons::Up0) && IsPressed(Buttons::Down0)) {
					ClearBit(Buttons::Down0);
					ClearBit(Buttons::Up0);
				}
				if(IsPressed(Buttons::Left0) && IsPressed(Buttons::Right0)) {
					ClearBit(Buttons::Left0);
					ClearBit(Buttons::Right0);
				}
				if (IsPressed(Buttons::Up1) && IsPressed(Buttons::Down1)) {
					ClearBit(Buttons::Down1);
					ClearBit(Buttons::Up1);
				}
				if (IsPressed(Buttons::Left1) && IsPressed(Buttons::Right1)) {
					ClearBit(Buttons::Left1);
					ClearBit(Buttons::Right1);
				}
			}
		}
	}

	uint16_t ToByte()
	{
		//"A Virtual Boy controller returns a 16-bit report in a similar order as SNES, with two additional buttons."

		return
			(uint8_t)IsPressed(Buttons::Down1) |
			((uint8_t)IsPressed(Buttons::Left1) << 1) |
			((uint8_t)IsPressed(Buttons::Select) << 2) |
			((uint8_t)IsPressed(Buttons::Start) << 3) |
			((uint8_t)IsPressed(Buttons::Up0) << 4) |
			((uint8_t)IsPressed(Buttons::Down0) << 5) |
			((uint8_t)IsPressed(Buttons::Left0) << 6) |
			((uint8_t)IsPressed(Buttons::Right0) << 7) |
			((uint8_t)IsPressed(Buttons::Right1) << 8) |
			((uint8_t)IsPressed(Buttons::Up1) << 9) |
			((uint8_t)IsPressed(Buttons::L) << 10) |
			((uint8_t)IsPressed(Buttons::R) << 11) |
			((uint8_t)IsPressed(Buttons::B) << 12) |
			((uint8_t)IsPressed(Buttons::A) << 13) |
			(1 << 14);
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_stateBuffer);
	}

	void RefreshStateBuffer() override
	{
		_stateBuffer = (uint32_t)ToByte();
	}

public:
	VirtualBoyController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::VirtualBoyController, port, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;

		if(IsCurrentPort(addr)) {
			StrobeProcessRead();

			output = _stateBuffer & 0x01;
			_stateBuffer >>= 1;

			//"All subsequent reads will return D=1 on an authentic controller but may return D=0 on third party controllers."
			_stateBuffer |= 0x8000;
		}

		return output;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}
};