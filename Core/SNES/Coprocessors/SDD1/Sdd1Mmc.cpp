#include "pch.h"
#include "SNES/Coprocessors/SDD1/Sdd1Mmc.h"
#include "SNES/Coprocessors/SDD1/Sdd1Types.h"
#include "SNES/BaseCartridge.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryType.h"

Sdd1Mmc::Sdd1Mmc(Sdd1State &state, BaseCartridge *cart) : IMemoryHandler(MemoryType::SnesRegister)
{
	_romHandlers = &cart->GetPrgRomHandlers();
	_handlerMask = (uint32_t)((*_romHandlers).size() - 1);
	_state = &state;
}

uint8_t Sdd1Mmc::ReadRom(uint32_t addr)
{
	return GetHandler(addr)->Read(addr);
}

uint16_t Sdd1Mmc::ProcessRomMirroring(uint32_t addr)
{
	if(((addr & 0x800000) && _state->SelectedBanks[3] & 0x80) || (!(addr & 0x800000) && (_state->SelectedBanks[1] & 0x80))) {
		//Force mirroring: $20-$3F mirrors $00-$1F, $A0-$BF mirrors $80-$9F
		return (((addr & 0x1F0000) >> 1) | (addr & 0x7000)) >> 12;
	} else {
		return (((addr & 0x3F0000) >> 1) | (addr & 0x7000)) >> 12;
	}
}

IMemoryHandler* Sdd1Mmc::GetHandler(uint32_t addr)
{
	uint16_t handlerIndex;
	if(!(addr & 0x400000)) {
		handlerIndex = ProcessRomMirroring(addr);
	} else {
		//Banks $C0+
		uint8_t bank = (addr >> 20) - 0x0C;
		handlerIndex = ((_state->SelectedBanks[bank] & 0x0F) << 8) | ((addr & 0xFF000) >> 12);
	}
	
	return (*_romHandlers)[handlerIndex & _handlerMask].get();
}

uint8_t Sdd1Mmc::Read(uint32_t addr)
{
	if(!(addr & 0x400000)) {
		return (*_romHandlers)[ProcessRomMirroring(addr) & _handlerMask]->Read(addr);
	}

	//Banks $C0+
	uint8_t activeChannels = _state->ProcessNextDma & _state->AllowDmaProcessing;
	if(activeChannels) {
		//Some DMA channels are being processed, need to check if the address being read matches one of the dma channels
		for(int i = 0; i < 8; i++) {
			if((activeChannels & (1 << i)) && addr == _state->DmaAddress[i]) {
				if(_state->NeedInit) {
					_decompressor.Init(this, addr);
					_state->NeedInit = false;
				}

				uint8_t data = _decompressor.GetDecompressedByte();

				_state->DmaLength[i]--;
				if(_state->DmaLength[i] == 0) {
					_state->NeedInit = true;
					_state->ProcessNextDma &= ~(1 << i);
				}

				return data;
			}
		}
	}

	return ReadRom(addr);
}

uint8_t Sdd1Mmc::Peek(uint32_t addr)
{
	return GetHandler(addr)->Peek(addr);
}

void Sdd1Mmc::PeekBlock(uint32_t addr, uint8_t *output)
{
	GetHandler(addr)->PeekBlock(addr, output);
}

void Sdd1Mmc::Write(uint32_t addr, uint8_t value)
{
	//ROM, read-only
}

AddressInfo Sdd1Mmc::GetAbsoluteAddress(uint32_t address)
{
	return GetHandler(address)->GetAbsoluteAddress(address);
}

void Sdd1Mmc::Serialize(Serializer &s)
{
	SV(_decompressor);
}
