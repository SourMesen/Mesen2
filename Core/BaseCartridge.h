#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "../Utilities/VirtualFile.h"

class BaseCartridge : public IMemoryHandler
{
private:
	uint8_t* _prgRom = nullptr;
	uint8_t* _saveRam = nullptr;
	
	uint32_t _prgRomSize = 0;
	uint32_t _saveRamSize = 0;

public:
	~BaseCartridge()
	{
		delete[] _prgRom;
		delete[] _saveRam;
	}

	static shared_ptr<BaseCartridge> CreateCartridge(VirtualFile romFile, VirtualFile patchFile)
	{
		if(romFile.IsValid()) {
			vector<uint8_t> romData;
			romFile.ReadFile(romData);

			shared_ptr<BaseCartridge> cart(new BaseCartridge());
			cart->_prgRomSize = (uint32_t)romData.size();
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

	uint8_t* DebugGetPrgRom() { return _prgRom; }
	uint8_t* DebugGetSaveRam() { return _saveRam; }
	uint32_t DebugGetPrgRomSize() { return _prgRomSize; }
	uint32_t DebugGetSaveRamSize() { return _saveRamSize; }
};
