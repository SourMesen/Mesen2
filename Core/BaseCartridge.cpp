#include "stdafx.h"
#include "BaseCartridge.h"
#include "RamHandler.h"
#include "RomHandler.h"
#include "MemoryManager.h"
#include "IMemoryHandler.h"
#include "../Utilities/VirtualFile.h"

BaseCartridge::~BaseCartridge()
{
	delete[] _prgRom;
	delete[] _saveRam;
}

shared_ptr<BaseCartridge> BaseCartridge::CreateCartridge(VirtualFile &romFile, VirtualFile &patchFile)
{
	if(romFile.IsValid()) {
		vector<uint8_t> romData;
		romFile.ReadFile(romData);

		shared_ptr<BaseCartridge> cart(new BaseCartridge());
		cart->_prgRomSize = (uint32_t)romData.size();
		cart->_prgRom = new uint8_t[cart->_prgRomSize];
		memcpy(cart->_prgRom, romData.data(), cart->_prgRomSize);
		cart->Init();

		return cart;
	} else {
		return nullptr;
	}
}

void BaseCartridge::Init()
{
	uint32_t headerOffset = 0;
	if(((0x400 << _prgRom[0x7FD7]) == _prgRomSize && (_prgRom[0x7FD5] & 0x20)) || _prgRomSize < 0xFFFF) {
		//LoROM
		headerOffset = 0x7FB0;
	} else if((0x400 << _prgRom[0xFFD7]) == _prgRomSize && (_prgRom[0xFFD5] & 0x20)) {
		//HiROM
		headerOffset = 0xFFB0;
	} else if(_prgRom[0x7FD5] & 0x20) {
		//LoROM
		headerOffset = 0x7FB0;
	} else if(_prgRom[0xFFD5] & 0x20) {
		//HiROM
		headerOffset = 0xFFB0;
	} else {
		throw new std::runtime_error("invalid rom (?)");
	}

	_cartInfo = *(SnesCartInformation*)(&_prgRom[headerOffset]);

	_saveRamSize = _cartInfo.SramSize > 0 ? 1024 * (1 << _cartInfo.SramSize) : 0;
	_saveRam = new uint8_t[_saveRamSize];
}

CartFlags::CartFlags BaseCartridge::GetCartFlags()
{
	uint32_t flags = 0;
	if(_cartInfo.MapMode  & 0x04) {
		flags |= CartFlags::ExHiRom;
	} else if(_cartInfo.MapMode & 0x02) {
		flags |= CartFlags::ExLoRom;
	} else if(_cartInfo.MapMode & 0x01) {
		flags |= CartFlags::HiRom;
	} else {
		flags |= CartFlags::LoRom;
	}

	if(_cartInfo.MapMode & 0x10) {
		flags |= CartFlags::FastRom;
	}

	return (CartFlags::CartFlags)flags;
}

void BaseCartridge::MapBanks(MemoryManager &mm, vector<unique_ptr<IMemoryHandler>> &handlers, uint8_t startBank, uint8_t endBank, uint16_t startPage, uint16_t endPage, uint16_t pageIncrement, bool mirror)
{
	if(handlers.empty()) {
		return;
	}

	uint32_t pageNumber = 0;
	for(uint32_t i = startBank; i <= endBank; i++) {
		uint32_t baseAddress = i << 16;
		pageNumber += pageIncrement;
		for(uint32_t j = startPage; j <= endPage; j++) {
			mm.RegisterHandler(baseAddress + (j * 0x1000), baseAddress + (j * 0x1000) | 0xFFF, handlers[pageNumber].get());
			//MessageManager::Log("Map [$" + HexUtilities::ToHex(i) + ":" + HexUtilities::ToHex(j)[1] + "xxx] to page number " + HexUtilities::ToHex(pageNumber));
			pageNumber++;
			if(pageNumber >= handlers.size()) {
				if(mirror) {
					pageNumber = 0;
				} else {
					return;
				}
			}
		}
	}
}

void BaseCartridge::RegisterHandlers(MemoryManager &mm)
{
	for(uint32_t i = 0; i < _prgRomSize; i += 0x1000) {
		_prgRomHandlers.push_back(unique_ptr<RomHandler>(new RomHandler(_prgRom + i)));
	}

	for(uint32_t i = 0; i < _saveRamSize; i += 0x1000) {
		_saveRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(_saveRam + i)));
	}

	if(GetCartFlags() & CartFlags::LoRom) {
		MapBanks(mm, _prgRomHandlers, 0x00, 0x6F, 0x08, 0x0F, 0, true);
		MapBanks(mm, _saveRamHandlers, 0x70, 0x7D, 0x00, 0x0F, 0, true);

		MapBanks(mm, _prgRomHandlers, 0x80, 0xEF, 0x08, 0x0F, 0, true);
		MapBanks(mm, _saveRamHandlers, 0xF0, 0xFF, 0x00, 0x0F, 0, true);
	} else {
		MapBanks(mm, _prgRomHandlers, 0x40, 0x7D, 0x00, 0x0F, 0, true);
		MapBanks(mm, _prgRomHandlers, 0xC0, 0xFF, 0x00, 0x0F, 0, true);
		MapBanks(mm, _prgRomHandlers, 0x00, 0x3F, 0x08, 0x0F, 8, true);
		MapBanks(mm, _prgRomHandlers, 0x80, 0xBF, 0x08, 0x0F, 8, true);

		MapBanks(mm, _saveRamHandlers, 0x20, 0x3F, 0x06, 0x07, 0, true);
	}

}