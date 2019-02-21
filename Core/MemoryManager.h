#pragma once
#include "stdafx.h"
#include "Console.h"
#include "Ppu.h"
#include "Spc.h"
#include "RamHandler.h"
#include "DmaController.h"
#include "BaseCartridge.h"
#include "ControlManager.h"
#include "InternalRegisters.h"
#include "IMemoryHandler.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"

class CpuRegisterHandler : public IMemoryHandler
{
private:
	Ppu *_ppu;
	Spc *_spc;
	DmaController *_dmaController;
	InternalRegisters *_regs;
	ControlManager *_controlManager;
	uint8_t *_workRam;
	uint32_t _wramPosition;

public:
	CpuRegisterHandler(Ppu *ppu, Spc *spc, DmaController *dmaController, InternalRegisters *regs, ControlManager *controlManager, uint8_t *workRam)
	{
		_ppu = ppu;
		_spc = spc;
		_regs = regs;
		_dmaController = dmaController;
		_controlManager = controlManager;
		
		_workRam = workRam;
		_wramPosition = 0;
	}

	uint8_t Read(uint32_t addr) override
	{
		addr &= 0xFFFF;
		if(addr >= 0x2140 && addr <= 0x217F) {
			return _spc->Read(addr & 0x03);
		} else if(addr == 0x2180) {
			return _workRam[_wramPosition++];
		} else if(addr == 0x4016 || addr == 0x4017) {
			return _controlManager->Read(addr);
		} else if(addr < 0x4200) {
			return _ppu->Read(addr);
		} else {
			return _regs->Read(addr);
		}
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		addr &= 0xFFFF;
		if(addr >= 0x2140 && addr <= 0x217F) {
			return _spc->Write(addr & 0x03, value);
		} if(addr >= 0x2180 && addr <= 0x2183) {
			switch(addr & 0xFFFF) {
				case 0x2180:
					_workRam[_wramPosition] = value;
					_wramPosition = (_wramPosition + 1) & (0x1FFFF);
					break;

				case 0x2181: _wramPosition = (_wramPosition & 0x1FF00) | value; break;
				case 0x2182: _wramPosition = (_wramPosition & 0x100FF) | (value << 8); break;
				case 0x2183: _wramPosition = (_wramPosition & 0xFFFF) | ((value & 0x01) << 16); break;
			}
		} else if(addr == 0x4016) {
			return _controlManager->Write(addr, value);
		} else if(addr == 0x420B || addr == 0x420C || addr >= 0x4300) {
			_dmaController->Write(addr, value);
		} else if(addr < 0x4200) {
			_ppu->Write(addr, value);
		} else {
			_regs->Write(addr, value);
		}
	}
};

class MemoryManager
{
public:
	constexpr static uint32_t WorkRamSize = 0x20000;

private:
	shared_ptr<Console> _console;

	shared_ptr<BaseCartridge> _cart;
	shared_ptr<CpuRegisterHandler> _cpuRegisterHandler;
	shared_ptr<Ppu> _ppu;

	IMemoryHandler* _handlers[0x100 * 0x10];
	vector<unique_ptr<RamHandler>> _workRamHandlers;

	uint8_t * _workRam;

	uint64_t _masterClock;
	uint64_t _lastMasterClock;

public:
	void Initialize(shared_ptr<Console> console)
	{
		_lastMasterClock = 0;
		_masterClock = 0;
		_console = console;
		_cart = console->GetCartridge();
		_ppu = console->GetPpu();

		_workRam = new uint8_t[MemoryManager::WorkRamSize];

		_cpuRegisterHandler.reset(new CpuRegisterHandler(
			_ppu.get(),
			console->GetSpc().get(),
			console->GetDmaController().get(),
			console->GetInternalRegisters().get(),
			console->GetControlManager().get(),
			_workRam
		));

		memset(_handlers, 0, sizeof(_handlers));
		//memset(_workRam, 0, 128 * 1024);

		for(uint32_t i = 0; i < 128 * 1024; i += 0x1000) {
			_workRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(_workRam + i)));
			RegisterHandler(0x7E0000 | i, 0x7E0000 | (i + 0xFFF), _workRamHandlers[_workRamHandlers.size() - 1].get());
		}

		for(int i = 0; i <= 0x3F; i++) {
			RegisterHandler((i << 16) | 0x2000, (i << 16) | 0x2FFF, _cpuRegisterHandler.get());
			RegisterHandler(((i | 0x80) << 16) | 0x2000, ((i | 0x80) << 16) | 0x2FFF, _cpuRegisterHandler.get());

			RegisterHandler((i << 16) | 0x4000, (i << 16) | 0x4FFF, _cpuRegisterHandler.get());
			RegisterHandler(((i | 0x80) << 16) | 0x4000, ((i | 0x80) << 16) | 0x4FFF, _cpuRegisterHandler.get());
		}

		for(int i = 0; i < 0x3F; i++) {
			RegisterHandler((i << 16) | 0x0000, (i << 16) | 0x0FFF, _workRamHandlers[0].get());
			RegisterHandler((i << 16) | 0x1000, (i << 16) | 0x1FFF, _workRamHandlers[1].get());
		}

		for(int i = 0x80; i < 0xBF; i++) {
			RegisterHandler((i << 16) | 0x0000, (i << 16) | 0x0FFF, _workRamHandlers[0].get());
			RegisterHandler((i << 16) | 0x1000, (i << 16) | 0x1FFF, _workRamHandlers[1].get());
		}

		_cart->RegisterHandlers(*this);
	}

	~MemoryManager()
	{
		delete[] _workRam;
	}

	void RegisterHandler(uint32_t startAddr, uint32_t endAddr, IMemoryHandler* handler)
	{
		if((startAddr & 0xFFF) != 0 || (endAddr & 0xFFF) != 0xFFF) {
			throw new std::runtime_error("invalid start/end address");
		}

		for(uint32_t addr = startAddr; addr < endAddr; addr += 0x1000) {
			if(_handlers[addr >> 12]) {
				throw new std::runtime_error("handler already set");
			}

			_handlers[addr >> 12] = handler;
		}
	}

	void IncrementMasterClock(uint32_t addr)
	{
		//This is incredibly inaccurate
		uint8_t bank = (addr & 0xFF0000) >> 8;
		if(bank >= 0x40 && bank <= 0x7F) {
			//Slow
			_masterClock += 8;
		} else if(bank >= 0xCF) {
			//Slow or fast (depending on register)
			//Use slow
			_masterClock += 8;
		} else {
			uint8_t page = (addr & 0xFF00) >> 8;
			if(page <= 0x1F) {
				//Slow
				_masterClock += 6;
			} else if(page >= 0x20 && page <= 0x3F) {
				//Fast
				_masterClock += 6;
			} else if(page == 0x40 || page == 0x41) {
				//extra slow
				_masterClock += 12;
			} else if(page >= 0x42 && page <= 0x5F) {
				//Fast
				_masterClock += 6;
			} else if(page >= 0x60 && page <= 0x7F) {
				//Slow
				_masterClock += 8;
			} else {
				//Slow or fast (depending on register)
				//Use slow
				_masterClock += 8;
			}
		}

		while(_lastMasterClock < _masterClock - 3) {
			_ppu->Exec();
			_lastMasterClock += 4;
		}
	}

	uint8_t Read(uint32_t addr, MemoryOperationType type)
	{
		IncrementMasterClock(addr);

		uint8_t value = 0;
		if(_handlers[addr >> 12]) {
			value = _handlers[addr >> 12]->Read(addr);
		} else {
			//open bus
			value = (addr>> 12);

			MessageManager::DisplayMessage("Debug", "Read - missing handler: $" + HexUtilities::ToHex(addr));
		}
		_console->ProcessCpuRead(addr, value, type);
		return value;
	}

	uint8_t Peek(uint32_t addr)
	{
		//Read, without triggering side-effects
		uint8_t value = 0;
		if(_handlers[addr >> 12]) {
			value = _handlers[addr >> 12]->Read(addr);
		}
		return value;
	}

	void Write(uint32_t addr, uint8_t value, MemoryOperationType type)
	{
		IncrementMasterClock(addr);

		_console->ProcessCpuWrite(addr, value, type);
		if(_handlers[addr >> 12]) {
			return _handlers[addr >> 12]->Write(addr, value);
		} else {
			MessageManager::DisplayMessage("Debug", "Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
		}
	}

	uint64_t GetMasterClock() { return _masterClock; }
	uint8_t* DebugGetWorkRam() { return _workRam; }
};