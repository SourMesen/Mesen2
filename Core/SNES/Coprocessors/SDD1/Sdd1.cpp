#include "pch.h"
#include "SNES/Coprocessors/SDD1/Sdd1.h"
#include "SNES/Coprocessors/SDD1/Sdd1Mmc.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/MemoryMappings.h"
#include "Utilities/Serializer.h"

Sdd1::Sdd1(SnesConsole* console)
{
	//This handler is used to dynamically map the ROM based on the banking registers
	_sdd1Mmc.reset(new Sdd1Mmc(_state, console->GetCartridge()));
	
	MemoryMappings *cpuMappings = console->GetMemoryManager()->GetMemoryMappings();
	vector<unique_ptr<IMemoryHandler>> &prgRomHandlers = console->GetCartridge()->GetPrgRomHandlers();
	vector<unique_ptr<IMemoryHandler>> &saveRamHandlers = console->GetCartridge()->GetSaveRamHandlers();

	//Regular A Bus register handler, keep a reference to it, it'll be overwritten below
	_cpuRegisterHandler = cpuMappings->GetHandler(0x4000);

	//Based on forum thread info: https://forums.nesdev.com/viewtopic.php?f=12&t=14156
	cpuMappings->RegisterHandler(0x00, 0x3F, 0x6000, 0x7FFF, saveRamHandlers);
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x6000, 0x7FFF, saveRamHandlers);
	cpuMappings->RegisterHandler(0x70, 0x73, 0x0000, 0xFFFF, saveRamHandlers);

	//S-DD1 registers (0x4800-0x4807)
	cpuMappings->RegisterHandler(0x00, 0x3F, 0x4000, 0x4FFF, this);
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x4000, 0x4FFF, this);

	cpuMappings->RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, prgRomHandlers);
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, prgRomHandlers);

	//Override these segments to implement the miroring that $4805/$4807 cause
	cpuMappings->RegisterHandler(0x20, 0x3F, 0x8000, 0xFFFF, _sdd1Mmc.get());
	cpuMappings->RegisterHandler(0xA0, 0xBF, 0x8000, 0xFFFF, _sdd1Mmc.get());

	//Regular bank switched mappings
	cpuMappings->RegisterHandler(0xC0, 0xFF, 0x0000, 0xFFFF, _sdd1Mmc.get());

	Reset();
}

void Sdd1::Reset()
{
	_state = {};
	_state.NeedInit = true;
	_state.SelectedBanks[0] = 0;
	_state.SelectedBanks[1] = 1;
	_state.SelectedBanks[2] = 2;
	_state.SelectedBanks[3] = 3;
}

uint8_t Sdd1::Read(uint32_t addr)
{
	if((uint16_t)addr >= 0x4800 && (uint16_t)addr <= 0x4807) {
		switch(addr & 0x07) {
			case 0: return _state.AllowDmaProcessing;
			case 1: return _state.ProcessNextDma;

			case 4: case 5: case 6: case 7:
				return _state.SelectedBanks[addr & 0x03];
		}
	}
	
	return _cpuRegisterHandler->Read(addr);
}

void Sdd1::Write(uint32_t addr, uint8_t value)
{
	if((uint16_t)addr >= 0x4800 && (uint16_t)addr <= 0x4807) {
		//S-DD1 registers
		switch(addr & 0x07) {
			case 0: _state.AllowDmaProcessing = value; break;
			case 1: _state.ProcessNextDma = value; break;

			case 4: case 5: case 6: case 7:
				_state.SelectedBanks[addr & 0x03] = value;
				break;
		}
	} else {
		if((uint16_t)addr >= 0x4300 && (uint16_t)addr <= 0x437A) {
			//Keep track of writes to the DMA controller to know which address/length the DMAs will use
			uint8_t ch = (addr >> 4) & 0x07;
			switch(addr & 0x0F) {
				case 0x02: _state.DmaAddress[ch] = (_state.DmaAddress[ch] & 0xFFFF00) | value; break;
				case 0x03: _state.DmaAddress[ch] = (_state.DmaAddress[ch] & 0xFF00FF) | (value << 8); break;
				case 0x04: _state.DmaAddress[ch] = (_state.DmaAddress[ch] & 0x00FFFF) | (value << 16); break;
				case 0x05: _state.DmaLength[ch] = (_state.DmaLength[ch] & 0xFF00) | value; break;
				case 0x06: _state.DmaLength[ch] = (_state.DmaLength[ch] & 0x00FF) | (value << 8); break;
			}
		}

		//Forward everything else to the regular handler 
		_cpuRegisterHandler->Write(addr, value);
	}
}

void Sdd1::Serialize(Serializer &s)
{
	SV(_state.AllowDmaProcessing); SV(_state.ProcessNextDma); SV(_state.NeedInit);
	SVArray(_state.DmaAddress, 8);
	SVArray(_state.DmaLength, 8);
	SVArray(_state.SelectedBanks, 4);
	SV(_sdd1Mmc);
}

uint8_t Sdd1::Peek(uint32_t addr)
{
	return 0;
}

void Sdd1::PeekBlock(uint32_t addr, uint8_t* output)
{
	memset(output, 0, 0x1000);
}

AddressInfo Sdd1::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}
