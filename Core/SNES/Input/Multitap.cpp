#include "pch.h"
#include "SNES/Input/Multitap.h"
#include "SNES/SnesConsole.h"
#include "SNES/InternalRegisters.h"
#include "Shared/Emulator.h"

Multitap::Multitap(SnesConsole* console, uint8_t port, ControllerConfig controllers[]) : ControllerHub(console->GetEmulator(), ControllerType::Multitap, port, controllers)
{
	_internalRegs = console->GetInternalRegisters();
}

uint8_t Multitap::ReadRam(uint16_t addr)
{
	uint8_t output = 0;

	if(IsCurrentPort(addr)) {
		StrobeProcessRead();

		uint8_t selectBit = 0x80 >> ((_port == 0) ? 1 : 0);
		uint8_t portSelect = (_internalRegs->GetIoPortOutput() & selectBit) ? 0 : 2;
		output = ReadPort(portSelect) & 0x01;
		output |= (ReadPort(portSelect + 1) & 0x01) << 1;  //P3 & P5 are reported in bit 1

		if(_strobe) {
			//Bit 1 is always set and bit 0 is always clear (when strobe is high)
			return 0x02;
		}
	}

	return output;
}
