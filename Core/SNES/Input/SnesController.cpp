#include "pch.h"
#include "SNES/Input/SnesController.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"

SnesController::SnesController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::SnesController, port, keyMappings)
{
	_turboSpeed = keyMappings.TurboSpeed;
}

string SnesController::GetKeyNames()
{
	return "ABXYLRSTUDLR";
}

void SnesController::InternalSetStateFromInput()
{
	for(KeyMapping& keyMapping : _keyMappings) {
		SetPressedState(Buttons::A, keyMapping.A);
		SetPressedState(Buttons::B, keyMapping.B);
		SetPressedState(Buttons::X, keyMapping.X);
		SetPressedState(Buttons::Y, keyMapping.Y);
		SetPressedState(Buttons::L, keyMapping.L);
		SetPressedState(Buttons::R, keyMapping.R);
		SetPressedState(Buttons::Start, keyMapping.Start);
		SetPressedState(Buttons::Select, keyMapping.Select);
		SetPressedState(Buttons::Up, keyMapping.Up);
		SetPressedState(Buttons::Down, keyMapping.Down);
		SetPressedState(Buttons::Left, keyMapping.Left);
		SetPressedState(Buttons::Right, keyMapping.Right);

		uint8_t turboFreq = 1 << (4 - _turboSpeed);
		bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
		if(turboOn) {
			SetPressedState(Buttons::A, keyMapping.TurboA);
			SetPressedState(Buttons::B, keyMapping.TurboB);
			SetPressedState(Buttons::X, keyMapping.TurboX);
			SetPressedState(Buttons::Y, keyMapping.TurboY);
			SetPressedState(Buttons::L, keyMapping.TurboL);
			SetPressedState(Buttons::R, keyMapping.TurboR);
		}

		bool allowInvalidInput = _emu->GetConsoleType() == ConsoleType::Nes ? _emu->GetSettings()->GetNesConfig().AllowInvalidInput : _emu->GetSettings()->GetSnesConfig().AllowInvalidInput;
		if(!allowInvalidInput) {
			//If both U+D or L+R are pressed at the same time, act as if neither is pressed
			if(IsPressed(Buttons::Up) && IsPressed(Buttons::Down)) {
				ClearBit(Buttons::Down);
				ClearBit(Buttons::Up);
			}
			if(IsPressed(Buttons::Left) && IsPressed(Buttons::Right)) {
				ClearBit(Buttons::Left);
				ClearBit(Buttons::Right);
			}
		}
	}
}

uint16_t SnesController::ToByte()
{
	//"A Super NES controller returns a 16-bit report in a similar order: B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R, then four 0 bits."

	return
		(uint8_t)IsPressed(Buttons::B) |
		((uint8_t)IsPressed(Buttons::Y) << 1) |
		((uint8_t)IsPressed(Buttons::Select) << 2) |
		((uint8_t)IsPressed(Buttons::Start) << 3) |
		((uint8_t)IsPressed(Buttons::Up) << 4) |
		((uint8_t)IsPressed(Buttons::Down) << 5) |
		((uint8_t)IsPressed(Buttons::Left) << 6) |
		((uint8_t)IsPressed(Buttons::Right) << 7) |
		((uint8_t)IsPressed(Buttons::A) << 8) |
		((uint8_t)IsPressed(Buttons::X) << 9) |
		((uint8_t)IsPressed(Buttons::L) << 10) |
		((uint8_t)IsPressed(Buttons::R) << 11);
}

void SnesController::Serialize(Serializer & s)
{
	BaseControlDevice::Serialize(s);
	SV(_stateBuffer);
}

void SnesController::RefreshStateBuffer()
{
	_stateBuffer = (uint32_t)ToByte();
}

uint8_t SnesController::ReadRam(uint16_t addr)
{
	uint8_t output = 0;

	if(IsCurrentPort(addr)) {
		StrobeProcessRead();

		if(_port >= 2) {
			output = (_stateBuffer & 0x01) << 1;  //P3/P4 are reported in bit 2
		} else {
			output = _stateBuffer & 0x01;
		}
		_stateBuffer >>= 1;

		//"All subsequent reads will return D=1 on an authentic controller but may return D=0 on third party controllers."
		_stateBuffer |= 0x8000;
	}

	return output;
}

void SnesController::WriteRam(uint16_t addr, uint8_t value)
{
	StrobeProcessWrite(value);
}

void SnesController::InternalDrawController(InputHud& hud)
{
	hud.DrawOutline(35, 14);

	hud.DrawButton(5, 3, 3, 3, IsPressed(Buttons::Up));
	hud.DrawButton(5, 9, 3, 3, IsPressed(Buttons::Down));
	hud.DrawButton(2, 6, 3, 3, IsPressed(Buttons::Left));
	hud.DrawButton(8, 6, 3, 3, IsPressed(Buttons::Right));
	hud.DrawButton(5, 6, 3, 3, false);

	hud.DrawButton(27, 3, 3, 3, IsPressed(Buttons::X));
	hud.DrawButton(27, 9, 3, 3, IsPressed(Buttons::B));
	hud.DrawButton(30, 6, 3, 3, IsPressed(Buttons::A));
	hud.DrawButton(24, 6, 3, 3, IsPressed(Buttons::Y));

	hud.DrawButton(4, 0, 5, 2, IsPressed(Buttons::L));
	hud.DrawButton(26, 0, 5, 2, IsPressed(Buttons::R));

	hud.DrawButton(13, 9, 4, 2, IsPressed(Buttons::Select));
	hud.DrawButton(18, 9, 4, 2, IsPressed(Buttons::Start));

	hud.DrawNumber(hud.GetControllerIndex() + 1, 16, 2);
}

vector<DeviceButtonName> SnesController::GetKeyNameAssociations()
{
	return {
		{ "a", Buttons::A },
		{ "b", Buttons::B },
		{ "x", Buttons::X },
		{ "y", Buttons::Y },
		{ "l", Buttons::L },
		{ "r", Buttons::R },
		{ "start", Buttons::Start },
		{ "select", Buttons::Select },
		{ "up", Buttons::Up },
		{ "down", Buttons::Down },
		{ "left", Buttons::Left },
		{ "right", Buttons::Right },
	};
}