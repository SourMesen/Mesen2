#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "DmaController.h"
#include "InternalRegisters.h"
#include "ControlManager.h"

class RegisterHandlerA : public IMemoryHandler
{
private:
	DmaController *_dmaController;
	InternalRegisters *_regs;
	ControlManager *_controlManager;

public:
	RegisterHandlerA(DmaController *dmaController, InternalRegisters *regs, ControlManager *controlManager)
	{
		_regs = regs;
		_dmaController = dmaController;
		_controlManager = controlManager;
	}

	uint8_t Read(uint32_t addr) override
	{
		addr &= 0xFFFF;
		if(addr == 0x4016 || addr == 0x4017) {
			return _controlManager->Read(addr);
		} else if(addr >= 0x4300) {
			return _dmaController->Read(addr);
		} else {
			return _regs->Read(addr);
		}
	}

	uint8_t Peek(uint32_t addr) override
	{
		//Avoid side effects for now
		return 0;
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		addr &= 0xFFFF;
		if(addr == 0x4016) {
			return _controlManager->Write(addr, value);
		} else if(addr == 0x420B || addr == 0x420C || addr >= 0x4300) {
			_dmaController->Write(addr, value);
		} else {
			_regs->Write(addr, value);
		}
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		return { -1, SnesMemoryType::CpuMemory };
	}
};
