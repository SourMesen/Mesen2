#pragma once
#include "stdafx.h"
#include "Console.h"
#include "Ppu.h"
#include "DmaController.h"
#include "BaseCartridge.h"
#include "IMemoryHandler.h"
#include "../Utilities/HexUtilities.h"

class CpuRegisterHandler : public IMemoryHandler
{
private:
	shared_ptr<Ppu> _ppu;
	shared_ptr<MemoryManager> _memoryManager;
	unique_ptr<DmaController> _dmaController;

public:
	CpuRegisterHandler(shared_ptr<Console> console)
	{
		_ppu = console->GetPpu();
		_memoryManager = console->GetMemoryManager();
		_dmaController.reset(new DmaController(_memoryManager));
	}

	uint8_t Read(uint32_t addr) override
	{
		return _ppu->Read(addr & 0xFFFF);
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_ppu->Write(addr & 0xFFFF, value);
		_dmaController->Write(addr & 0xFFFF, value);
	}
};

class WorkRamHandler : public IMemoryHandler
{
private:
	uint8_t * _workRam;

public:
	WorkRamHandler(uint8_t *workRam)
	{
		_workRam = workRam;
	}

	uint8_t Read(uint32_t addr) override
	{
		return _workRam[addr & 0xFFF];
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_workRam[addr & 0xFFF] = value;
	}
};

class MemoryManager
{
public:
	constexpr static uint32_t WorkRamSize = 0x20000;

private:
	shared_ptr<Console> _console;

	uint8_t * _workRam;
	IMemoryHandler* _handlers[0x100 * 0x10];
	vector<unique_ptr<WorkRamHandler>> _workRamHandlers;
	shared_ptr<BaseCartridge> _cart;
	shared_ptr<CpuRegisterHandler> _cpuRegisterHandler;
	shared_ptr<Ppu> _ppu;
	
	unique_ptr<DmaController> _dmaController;

	uint32_t _wramPosition;

	uint64_t _masterClock;
	uint64_t _lastMasterClock;

public:
	void Initialize(shared_ptr<BaseCartridge> cart, shared_ptr<Console> console)
	{
		_lastMasterClock = 0;
		_masterClock = 0;
		_console = console;
		_cart = cart;
		_ppu = console->GetPpu();

		_cpuRegisterHandler.reset(new CpuRegisterHandler(console));

		memset(_handlers, 0, sizeof(_handlers));
		_workRam = new uint8_t[MemoryManager::WorkRamSize];
		//memset(_workRam, 0, 128 * 1024);

		for(uint32_t i = 0; i < 128 * 1024; i += 0x1000) {
			_workRamHandlers.push_back(unique_ptr<WorkRamHandler>(new WorkRamHandler(_workRam + i)));
			RegisterHandler(0x7E0000 | i, 0x7E0000 | (i + 0xFFF), _workRamHandlers[_workRamHandlers.size() - 1].get());
		}

		for(int i = 0; i <= 0x3F; i++) {
			RegisterHandler((i << 16) | 0x2000, (i << 16) | 0x2FFF, _cpuRegisterHandler.get());
			RegisterHandler(((i | 0x80) << 16) | 0x2000, ((i | 0x80) << 16) | 0x2FFF, _cpuRegisterHandler.get());

			RegisterHandler((i << 16) | 0x4000, (i << 16) | 0x4FFF, _cpuRegisterHandler.get());
			RegisterHandler(((i | 0x80) << 16) | 0x4000, ((i | 0x80) << 16) | 0x4FFF, _cpuRegisterHandler.get());
		}

		RegisterHandler(0x0000, 0x0FFF, _workRamHandlers[0].get());
		RegisterHandler(0x1000, 0x1FFF, _workRamHandlers[1].get());

		for(int bank = 0; bank < 0x20; bank++) {
			RegisterHandler((bank << 16) | 0x8000, (bank << 16) | 0xFFFF, _cart.get());
			RegisterHandler(((0x80 | bank) << 16) | 0x8000, ((0x80 | bank) << 16) | 0xFFFF, _cart.get());
		}
	}

	~MemoryManager()
	{
	}

	void RegisterHandler(uint32_t startAddr, uint32_t endAddr, IMemoryHandler* handler)
	{
		if((startAddr & 0xFFF) != 0 || (endAddr & 0xFFF) != 0xFFF) {
			throw new std::runtime_error("invalid start/end address");
		}

		for(uint32_t addr = startAddr; addr < endAddr; addr += 0x1000) {
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
			//std::cout << "Read - missing handler: $" << HexUtilities::ToHex(addr) << std::endl;
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
		} else {
			//std::cout << "Read - missing handler: $" << HexUtilities::ToHex(addr) << std::endl;
		}
		return value;
	}

	void Write(uint32_t addr, uint8_t value, MemoryOperationType type)
	{
		IncrementMasterClock(addr);

		switch(addr & 0xFFFF) {
			case 0x2180:
				_workRam[_wramPosition] = value;
				_wramPosition++;
				break;

			case 0x2181:
				_wramPosition = (_wramPosition & 0x1FF00) | value;
				break;

			case 0x2182:
				_wramPosition = (_wramPosition & 0x100FF) | (value << 8);
				break;

			case 0x2183:
				_wramPosition = (_wramPosition & 0xFFFF) | ((value & 0x01) << 16);
				break;
		}

		_console->ProcessCpuWrite(addr, value, type);
		if(_handlers[addr >> 12]) {
			return _handlers[addr >> 12]->Write(addr, value);
		} else {
			//std::cout << "Write - missing handler: $" << HexUtilities::ToHex(addr) << " = " << HexUtilities::ToHex(value) << std::endl;
		}
	}

	uint8_t* DebugGetWorkRam() { return _workRam; }
};