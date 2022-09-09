#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControllerHub.h"

class InternalRegisters;
class SnesConsole;

class Multitap : public ControllerHub<4>
{
private:
	InternalRegisters *_internalRegs = nullptr;

public:
	Multitap(SnesConsole* console, uint8_t port, ControllerConfig controllers[]);

	uint8_t ReadRam(uint16_t addr) override;
};