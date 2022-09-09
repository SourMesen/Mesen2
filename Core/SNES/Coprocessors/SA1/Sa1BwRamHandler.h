#pragma once
#include "pch.h"
#include "SNES/Coprocessors/SA1/Sa1Cpu.h"
#include "SNES/Coprocessors/SA1/Sa1Types.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/IMemoryHandler.h"

//Manages BWRAM access from the SA-1 CPU, for regions that can enable bitmap mode. e.g:
//00-3F:6000-7FFF + 80-BF:6000-7FFF (optional bitmap mode + bank select)
//60-6F:0000-FFFF (always bitmap mode)
class Sa1BwRamHandler : public IMemoryHandler
{
private:
	uint8_t * _ram;
	uint32_t _mask;
	Sa1State* _state;

	uint32_t GetBwRamAddress(uint32_t addr)
	{
		return ((_state->Sa1BwBank * 0x2000) | (addr & 0x1FFF));
	}

	__forceinline uint8_t InternalRead(uint32_t addr)
	{
		if((addr & 0x600000) == 0x600000) {
			return ReadBitmapMode(addr - 0x600000);
		} else {
			addr = GetBwRamAddress(addr);
			if(_state->Sa1BwMode) {
				//Bitmap mode is enabled
				return ReadBitmapMode(addr);
			} else {
				//Return regular memory content
				return _ram[addr & _mask];
			}
		}
	}

public:
	Sa1BwRamHandler(uint8_t* bwRam, uint32_t bwRamSize, Sa1State* state) : IMemoryHandler(MemoryType::SnesSaveRam)
	{
		_ram = bwRam;
		_mask = bwRamSize - 1;
		_state = state;
	}

	uint8_t Read(uint32_t addr) override
	{
		return InternalRead(addr);
	}

	uint8_t Peek(uint32_t addr) override
	{
		return InternalRead(addr);
	}

	void PeekBlock(uint32_t addr, uint8_t *output) override
	{
		for(int i = 0; i < 0x1000; i++) {
			output[i] = InternalRead(addr + i);
		}
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		if((addr & 0x600000) == 0x600000) {
			WriteBitmapMode(addr - 0x600000, value);
		} else {
			addr = GetBwRamAddress(addr);
			if(_state->Sa1BwMode) {
				WriteBitmapMode(addr, value);
			} else {
				_ram[addr & _mask] = value;
			}
		}
	}

	uint8_t ReadBitmapMode(uint32_t addr)
	{
		if(_state->BwRam2BppMode) {
			return (_ram[(addr >> 2) & _mask] >> ((addr & 0x03) * 2)) & 0x03;
		} else {
			return (_ram[(addr >> 1) & _mask] >> ((addr & 0x01) * 4)) & 0x0F;
		}
	}

	void WriteBitmapMode(uint32_t addr, uint8_t value)
	{
		if(_state->BwRam2BppMode) {
			uint8_t shift = (addr & 0x03) * 2;
			addr = (addr >> 2) & _mask;
			_ram[addr] = (_ram[addr] & ~(0x03 << shift)) | ((value & 0x03) << shift);
		} else {
			uint8_t shift = (addr & 0x01) * 4;
			addr = (addr >> 1) & _mask;
			_ram[addr] = (_ram[addr] & ~(0x0F << shift)) | ((value & 0x0F) << shift);
		}
	}

	AddressInfo GetAbsoluteAddress(uint32_t addr) override
	{
		AddressInfo info;
		if((addr & 0x600000) == 0x600000) {
			info.Address = ((addr - 0x600000) >> (_state->BwRam2BppMode ? 2 : 1)) & _mask;
		} else {
			info.Address = GetBwRamAddress(addr) & _mask;
		}
		info.Type = MemoryType::SnesSaveRam;
		return info;
	}
};