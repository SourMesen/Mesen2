#include "pch.h"
#include "SNES/MemoryMappings.h"
#include "SNES/IMemoryHandler.h"
#include "Shared/MemoryType.h"

void MemoryMappings::RegisterHandler(uint8_t startBank, uint8_t endBank, uint16_t startPage, uint16_t endPage, vector<unique_ptr<IMemoryHandler>> &handlers, uint16_t pageIncrement, uint16_t startPageNumber)
{
	if(handlers.empty()) {
		return;
	}

	startPageNumber %= handlers.size();

	uint32_t pageNumber = startPageNumber;
	for(uint32_t i = startBank; i <= endBank; i++) {
		pageNumber += pageIncrement;
		for(uint32_t j = startPage; j <= endPage; j += 0x1000) {
			_handlers[(i << 4) | (j >> 12)] = handlers[pageNumber].get();
			//MessageManager::Log("Map [$" + HexUtilities::ToHex(i) + ":" + HexUtilities::ToHex(j)[1] + "xxx] to page number " + HexUtilities::ToHex(pageNumber));
			pageNumber++;
			if(pageNumber >= handlers.size()) {
				pageNumber = 0;
			}
		}
	}
}

void MemoryMappings::RegisterHandler(uint8_t startBank, uint8_t endBank, uint16_t startAddr, uint16_t endAddr, IMemoryHandler* handler)
{
	if((startAddr & 0xFFF) != 0 || (endAddr & 0xFFF) != 0xFFF || startBank > endBank || startAddr > endAddr) {
		throw std::runtime_error("invalid start/end address");
	}

	for(uint32_t bank = startBank; bank <= endBank; bank++) {
		for(uint32_t addr = startAddr; addr < endAddr; addr += 0x1000) {
			/*if(_handlers[addr >> 12]) {
			throw std::runtime_error("handler already set");
			}*/

			_handlers[(bank << 4) | (addr >> 12)] = handler;
		}
	}
}

IMemoryHandler* MemoryMappings::GetHandler(uint32_t addr)
{
	return _handlers[addr >> 12];
}

AddressInfo MemoryMappings::GetAbsoluteAddress(uint32_t addr)
{
	IMemoryHandler* handler = GetHandler(addr);
	if(handler) {
		return handler->GetAbsoluteAddress(addr);
	} else {
		return { -1, MemoryType::SnesMemory };
	}
}

int MemoryMappings::GetRelativeAddress(AddressInfo& absAddress, uint8_t startBank)
{
	uint16_t startPosition = startBank << 4;

	for(int i = startPosition; i <= 0xFFF; i++) {
		IMemoryHandler* handler = GetHandler(i << 12);
		if(handler) {
			AddressInfo addrInfo = handler->GetAbsoluteAddress((i << 12) | (absAddress.Address & 0xFFF));
			if(addrInfo.Type == absAddress.Type && addrInfo.Address == absAddress.Address) {
				return (i << 12) | (absAddress.Address & 0xFFF);
			}
		}
	}
	for(int i = 0; i < startPosition; i++) {
		IMemoryHandler* handler = GetHandler(i << 12);
		if(handler) {
			AddressInfo addrInfo = handler->GetAbsoluteAddress((i << 12) | (absAddress.Address & 0xFFF));
			if(addrInfo.Type == absAddress.Type && addrInfo.Address == absAddress.Address) {
				return (i << 12) | (absAddress.Address & 0xFFF);
			}
		}
	}
	return -1;
}

uint8_t MemoryMappings::Peek(uint32_t addr)
{
	//Read, without triggering side-effects
	uint8_t value = 0;
	IMemoryHandler* handler = GetHandler(addr);
	if(handler) {
		value = handler->Peek(addr);
	}
	return value;
}

uint16_t MemoryMappings::PeekWord(uint32_t addr)
{
	uint8_t lsb = Peek(addr);
	uint8_t msb = Peek((addr + 1) & 0xFFFFFF);
	return (msb << 8) | lsb;
}

void MemoryMappings::PeekBlock(uint32_t addr, uint8_t *dest)
{
	IMemoryHandler* handler = GetHandler(addr);
	if(handler) {
		handler->PeekBlock(addr & ~0xFFF, dest);
	} else {
		memset(dest, 0, 0x1000);
	}
}

void MemoryMappings::DebugWrite(uint32_t addr, uint8_t value)
{
	IMemoryHandler* handler = GetHandler(addr);
	if(handler) {
		handler->Write(addr, value);
	}
}
