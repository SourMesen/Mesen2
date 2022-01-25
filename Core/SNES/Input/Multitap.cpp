#include "stdafx.h"
#include "SNES/Input/Multitap.h"
#include "SNES/Input/SnesController.h"
#include "SNES/SnesConsole.h"
#include "SNES/InternalRegisters.h"
#include "Shared/Emulator.h"

string Multitap::GetKeyNames()
{
	//Repeat key names 4x, once for each controller
	return "ABXYLRSTUDLR:ABXYLRSTUDLR:ABXYLRSTUDLR:ABXYLRSTUDLR";
}

void Multitap::InternalSetStateFromInput()
{
	for(int i = 0; i < 4; i++) {
		int offset = Multitap::ButtonCount * i;
		for(KeyMapping& keyMapping : _mappings[i]) {
			SetPressedState(Buttons::A + offset, keyMapping.A);
			SetPressedState(Buttons::B + offset, keyMapping.B);
			SetPressedState(Buttons::X + offset, keyMapping.X);
			SetPressedState(Buttons::Y + offset, keyMapping.Y);
			SetPressedState(Buttons::L + offset, keyMapping.L);
			SetPressedState(Buttons::R + offset, keyMapping.R);
			SetPressedState(Buttons::Start + offset, keyMapping.Start);
			SetPressedState(Buttons::Select + offset, keyMapping.Select);
			SetPressedState(Buttons::Up + offset, keyMapping.Up);
			SetPressedState(Buttons::Down + offset, keyMapping.Down);
			SetPressedState(Buttons::Left + offset, keyMapping.Left);
			SetPressedState(Buttons::Right + offset, keyMapping.Right);

			uint8_t turboFreq = 1 << (4 - _turboSpeed[i]);
			bool turboOn = (uint8_t)(_emu->GetPpuFrame().FrameCount % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::A + offset, keyMapping.TurboA);
				SetPressedState(Buttons::B + offset, keyMapping.TurboB);
				SetPressedState(Buttons::X + offset, keyMapping.TurboX);
				SetPressedState(Buttons::Y + offset, keyMapping.TurboY);
				SetPressedState(Buttons::L + offset, keyMapping.TurboL);
				SetPressedState(Buttons::R + offset, keyMapping.TurboR);
			}
		}
	}
}

void Multitap::UpdateControllerState(uint8_t controllerNumber, SnesController &controller)
{
	int offset = Multitap::ButtonCount * controllerNumber;
	SetBitValue(Buttons::A + offset, controller.IsPressed(Buttons::A));
	SetBitValue(Buttons::B + offset, controller.IsPressed(Buttons::B));
	SetBitValue(Buttons::X + offset, controller.IsPressed(Buttons::X));
	SetBitValue(Buttons::Y + offset, controller.IsPressed(Buttons::Y));
	SetBitValue(Buttons::L + offset, controller.IsPressed(Buttons::L));
	SetBitValue(Buttons::R + offset, controller.IsPressed(Buttons::R));
	SetBitValue(Buttons::Start + offset, controller.IsPressed(Buttons::Start));
	SetBitValue(Buttons::Select + offset, controller.IsPressed(Buttons::Select));
	SetBitValue(Buttons::Up + offset, controller.IsPressed(Buttons::Up));
	SetBitValue(Buttons::Down + offset, controller.IsPressed(Buttons::Down));
	SetBitValue(Buttons::Left + offset, controller.IsPressed(Buttons::Left));
	SetBitValue(Buttons::Right + offset, controller.IsPressed(Buttons::Right));
}

uint16_t Multitap::ToByte(uint8_t port)
{
	//"A Super NES controller returns a 16-bit report in a similar order: B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R, then four 0 bits."

	int offset = port * Multitap::ButtonCount;

	return
		(uint8_t)IsPressed(Buttons::B + offset) |
		((uint8_t)IsPressed(Buttons::Y + offset) << 1) |
		((uint8_t)IsPressed(Buttons::Select + offset) << 2) |
		((uint8_t)IsPressed(Buttons::Start + offset) << 3) |
		((uint8_t)IsPressed(Buttons::Up + offset) << 4) |
		((uint8_t)IsPressed(Buttons::Down + offset) << 5) |
		((uint8_t)IsPressed(Buttons::Left + offset) << 6) |
		((uint8_t)IsPressed(Buttons::Right + offset) << 7) |
		((uint8_t)IsPressed(Buttons::A + offset) << 8) |
		((uint8_t)IsPressed(Buttons::X + offset) << 9) |
		((uint8_t)IsPressed(Buttons::L + offset) << 10) |
		((uint8_t)IsPressed(Buttons::R + offset) << 11);
}

void Multitap::Serialize(Serializer & s)
{
	BaseControlDevice::Serialize(s);
	s.Stream(_stateBuffer[0], _stateBuffer[1], _stateBuffer[2], _stateBuffer[3]);
}

void Multitap::RefreshStateBuffer()
{
	for(int i = 0; i < 4; i++) {
		_stateBuffer[i] = ToByte(i);
	}
}

Multitap::Multitap(SnesConsole* console, uint8_t port, KeyMappingSet keyMappings1, KeyMappingSet keyMappings2, KeyMappingSet keyMappings3, KeyMappingSet keyMappings4) : BaseControlDevice(console->GetEmulator(), ControllerType::Multitap, port, keyMappings1)
{
	_turboSpeed[0] = keyMappings1.TurboSpeed;
	_turboSpeed[1] = keyMappings2.TurboSpeed;
	_turboSpeed[2] = keyMappings3.TurboSpeed;
	_turboSpeed[3] = keyMappings4.TurboSpeed;

	_mappings[0] = keyMappings1.GetKeyMappingArray();
	_mappings[1] = keyMappings2.GetKeyMappingArray();
	_mappings[2] = keyMappings3.GetKeyMappingArray();
	_mappings[3] = keyMappings4.GetKeyMappingArray();
	_internalRegs = console->GetInternalRegisters();
}

void Multitap::SetControllerState(uint8_t controllerNumber, ControlDeviceState state)
{
	SnesController controller(_emu, 0, KeyMappingSet());
	controller.SetRawState(state);
	UpdateControllerState(controllerNumber, controller);
}

uint8_t Multitap::ReadRam(uint16_t addr)
{
	uint8_t selectBit = 0x80 >> ((_port == 0) ? 1 : 0);
	uint8_t portSelect = (_internalRegs->GetIoPortOutput() & selectBit) ? 0 : 2;
	uint8_t output = 0;

	if(IsCurrentPort(addr)) {
		StrobeProcessRead();

		output = _stateBuffer[portSelect] & 0x01;
		output |= (_stateBuffer[portSelect + 1] & 0x01) << 1;  //P3 & P5 are reported in bit 1

		if(_strobe) {
			//Bit 1 is always set when strobe is high
			output |= 0x02;
		}

		_stateBuffer[portSelect] >>= 1;
		_stateBuffer[portSelect + 1] >>= 1;

		//"All subsequent reads will return D=1 on an authentic controller but may return D=0 on third party controllers."
		_stateBuffer[portSelect] |= 0x8000;
		_stateBuffer[portSelect + 1] |= 0x8000;
	}

	return output;
}

void Multitap::WriteRam(uint16_t addr, uint8_t value)
{
	if(addr == 0x4016) {
		StrobeProcessWrite(value);
	}
}
