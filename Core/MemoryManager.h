#pragma once
#include "stdafx.h"
#include "Console.h"
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

public:
	MemoryManager(shared_ptr<BaseCartridge> cart, shared_ptr<Console> console)
	{
		_console = console;
		_cart = cart;

		memset(_handlers, 0, sizeof(_handlers));
		_workRam = new uint8_t[128 * 1024];
		for(uint32_t i = 0; i < 128 * 1024; i += 0x1000) {
			_workRamHandlers.push_back(unique_ptr<WorkRamHandler>(new WorkRamHandler(_workRam + i)));
			RegisterHandler(0x7E0000 | i, 0x7E0000 | (i + 0xFFF), _workRamHandlers[_workRamHandlers.size() - 1].get());
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

	uint8_t Read(uint32_t addr, MemoryOperationType type)
	{
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
		_console->ProcessCpuWrite(addr, value, type);
		if(_handlers[addr >> 12]) {
			return _handlers[addr >> 12]->Write(addr, value);
		} else {
			//std::cout << "Write - missing handler: $" << HexUtilities::ToHex(addr) << " = " << HexUtilities::ToHex(value) << std::endl;
		}
	}
};