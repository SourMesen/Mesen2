#include "stdafx.h"
#include "SNES/Input/SnesController.h"
#include "Shared/Emulator.h"

SnesController::SnesController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, port, keyMappings)
{
	_turboSpeed = keyMappings.TurboSpeed;
}

string SnesController::GetKeyNames()
{
	return "ABXYLRSTUDLR";
}

void SnesController::InternalSetStateFromInput()
{
	for(KeyMapping keyMapping : _keyMappings) {
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
		bool turboOn = (uint8_t)(_emu->GetPpuFrame().FrameCount % turboFreq) < turboFreq / 2;
		if(turboOn) {
			SetPressedState(Buttons::A, keyMapping.TurboA);
			SetPressedState(Buttons::B, keyMapping.TurboB);
			SetPressedState(Buttons::X, keyMapping.TurboX);
			SetPressedState(Buttons::Y, keyMapping.TurboY);
			SetPressedState(Buttons::L, keyMapping.TurboL);
			SetPressedState(Buttons::R, keyMapping.TurboR);
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
	s.Stream(_stateBuffer);
}

void SnesController::RefreshStateBuffer()
{
	_stateBuffer = (uint32_t)ToByte();
}

ControllerType SnesController::GetControllerType()
{
	return ControllerType::SnesController;
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
