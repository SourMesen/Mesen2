#pragma once
#include "stdafx.h"
#include "Console.h"
#include "Ppu.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/VirtualFile.h"

class IMemoryHandler
{
public:
	virtual uint8_t Read(uint32_t addr) = 0;
	virtual void Write(uint32_t addr, uint8_t value) = 0;

	//virtual void GetMemoryRanges(MemoryRanges &ranges) = 0;
	//virtual uint8_t PeekRAM(uint16_t addr) { return 0; }

	virtual ~IMemoryHandler() {}
};

class BaseCartridge : public IMemoryHandler
{
private:
	size_t _prgRomSize;
	uint8_t* _prgRom;

public:
	static shared_ptr<BaseCartridge> CreateCartridge(VirtualFile romFile, VirtualFile patchFile)
	{
		if(romFile.IsValid()) {
			vector<uint8_t> romData;
			romFile.ReadFile(romData);

			shared_ptr<BaseCartridge> cart(new BaseCartridge());
			cart->_prgRomSize = romData.size();
			cart->_prgRom = new uint8_t[cart->_prgRomSize];
			memcpy(cart->_prgRom, romData.data(), cart->_prgRomSize);

			return cart;
		} else {
			return nullptr;
		}
	}

	uint8_t Read(uint32_t addr) override
	{
		uint8_t bank = (addr >> 16) & 0x7F;
		return _prgRom[((bank * 0x8000) | (addr & 0x7FFF)) & (_prgRomSize - 1)];
	}

	void Write(uint32_t addr, uint8_t value) override
	{
	}
};

class CpuRegisterHandler : public IMemoryHandler
{
private:
	shared_ptr<Ppu> _ppu;

public:
	CpuRegisterHandler(shared_ptr<Ppu> ppu)
	{
		_ppu = ppu;
	}

	uint8_t Read(uint32_t addr) override
	{
		return _ppu->Read(addr);
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_ppu->Write(addr, value);
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
private:
	shared_ptr<Console> _console;

	uint8_t * _workRam;
	IMemoryHandler* _handlers[0x100 * 0x10];
	vector<unique_ptr<WorkRamHandler>> _workRamHandlers;
	shared_ptr<BaseCartridge> _cart;
	shared_ptr<CpuRegisterHandler> _cpuRegisterHandler;
	shared_ptr<Ppu> _ppu;

	uint64_t _masterClock;
	uint64_t _lastMasterClock;

public:
	MemoryManager(shared_ptr<BaseCartridge> cart, shared_ptr<Console> console)
	{
		_lastMasterClock = 0;
		_masterClock = 0;
		_console = console;
		_cart = cart;
		_ppu = console->GetPpu();

		_cpuRegisterHandler.reset(new CpuRegisterHandler(_ppu));

		memset(_handlers, 0, sizeof(_handlers));
		_workRam = new uint8_t[128 * 1024];
		memset(_workRam, 0, 128 * 1024);

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

		_console->ProcessCpuWrite(addr, value, type);
		if(_handlers[addr >> 12]) {
			return _handlers[addr >> 12]->Write(addr, value);
		} else {
			//std::cout << "Write - missing handler: $" << HexUtilities::ToHex(addr) << " = " << HexUtilities::ToHex(value) << std::endl;
		}
	}
};