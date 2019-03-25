#include "stdafx.h"
#include "BaseCartridge.h"
#include "RamHandler.h"
#include "RomHandler.h"
#include "MemoryManager.h"
#include "IMemoryHandler.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/sha1.h"

BaseCartridge::~BaseCartridge()
{
	SaveBattery();

	delete[] _prgRom;
	delete[] _saveRam;
}

shared_ptr<BaseCartridge> BaseCartridge::CreateCartridge(VirtualFile &romFile, VirtualFile &patchFile)
{
	if(romFile.IsValid()) {
		vector<uint8_t> romData;
		romFile.ReadFile(romData);

		shared_ptr<BaseCartridge> cart(new BaseCartridge());
		cart->_romPath = romFile;
		cart->_prgRomSize = (uint32_t)romData.size();
		cart->_prgRom = new uint8_t[cart->_prgRomSize];
		memcpy(cart->_prgRom, romData.data(), cart->_prgRomSize);
		cart->Init();

		return cart;
	} else {
		return nullptr;
	}
}

int32_t BaseCartridge::GetHeaderScore(uint32_t addr)
{
	//Try to figure out where the header is by using a scoring system
	if(_prgRomSize < addr + 0x7FFF) {
		return -1;
	}

	SnesCartInformation cartInfo;
	memcpy(&cartInfo, _prgRom + addr + 0x7FB0, sizeof(SnesCartInformation));
	
	uint32_t score = 0;
	uint8_t mode = (cartInfo.MapMode & ~0x10);
	if((mode == 0x20 || mode == 0x22) && addr < 0x8000) {
		score++;
	} else if((mode == 0x21 || mode == 0x25) && addr >= 0x8000) {
		score++;
	}

	if(cartInfo.RomType < 0x08) {
		score++;
	}
	if(cartInfo.RomSize < 0x10) {
		score++;
	}
	if(cartInfo.SramSize < 0x08) {
		score++;
	}

	uint16_t checksum = cartInfo.Checksum[0] | (cartInfo.Checksum[1] << 8);
	uint16_t complement = cartInfo.ChecksumComplement[0] | (cartInfo.ChecksumComplement[1] << 8);
	if(checksum + complement == 0xFFFF && checksum != 0 && complement != 0) {
		score += 8;
	}

	uint32_t resetVectorAddr = addr + 0x7FFC;
	uint32_t resetVector = _prgRom[resetVectorAddr] | (_prgRom[resetVectorAddr + 1] << 8);
	if(resetVector < 0x8000) {
		return -1;
	}
	
	uint8_t op = _prgRom[addr + (resetVector & 0x7FFF)];
	if(op == 0x18 || op == 0x78 || op == 0x4C || op == 0x5C || op == 0x20 || op == 0x22 || op == 0x9C) {
		//CLI, SEI, JMP, JML, JSR, JSl, STZ
		score += 8;
	} else if(op == 0xC2 || op == 0xE2 || op == 0xA9 || op == 0xA2 || op == 0xA0) {
		//REP, SEP, LDA, LDX, LDY
		score += 4;
	} else if(op == 0x00 || op == 0xFF || op == 0xCC) {
		//BRK, SBC, CPY
		score -= 8;
	}

	return std::max<int32_t>(0, score);
}

void BaseCartridge::Init()
{
	//Find the best potential header among lorom/hirom + headerless/headered combinations
	vector<uint32_t> baseAddresses = { 0, 0x200, 0x8000, 0x8200 };
	int32_t bestScore = -1;
	bool hasHeader = false;
	bool isLoRom = true;
	for(uint32_t baseAddress : baseAddresses) {
		int32_t score = GetHeaderScore(baseAddress);
		if(score > bestScore) {
			bestScore = score;
			isLoRom = (baseAddress & 0x8000) == 0;
			hasHeader = (baseAddress & 0x200) != 0;
		}
	}

	uint32_t headerOffset = 0;
	uint32_t flags = 0;
	if(isLoRom) {
		if(hasHeader) {
			flags |= CartFlags::CopierHeader;
		}
		flags |= CartFlags::LoRom;
		headerOffset = 0x7FB0;
	} else {
		if(hasHeader) {
			flags |= CartFlags::CopierHeader;
		}
		flags |= CartFlags::HiRom;
		headerOffset = 0xFFB0;
	}

	if(flags & CartFlags::CopierHeader) {
		//Remove the copier header
		memmove(_prgRom, _prgRom + 512, _prgRomSize - 512);
		_prgRomSize -= 512;
	}
	
	memcpy(&_cartInfo, _prgRom + headerOffset, sizeof(SnesCartInformation));

	if((flags & CartFlags::HiRom) && (_cartInfo.MapMode & 0x27) == 0x25) {
		flags |= CartFlags::ExHiRom;
	} else if((flags & CartFlags::LoRom) && (_cartInfo.MapMode & 0x27) == 0x22) {
		flags |= CartFlags::ExLoRom;
	}

	if(_cartInfo.MapMode & 0x10) {
		flags |= CartFlags::FastRom;
	}
	_flags = (CartFlags::CartFlags)flags;
	
	_saveRamSize = _cartInfo.SramSize > 0 ? 1024 * (1 << _cartInfo.SramSize) : 0;
	_saveRam = new uint8_t[_saveRamSize];

	LoadBattery();

	DisplayCartInfo();
}

RomInfo BaseCartridge::GetRomInfo()
{
	RomInfo info;
	info.Header = _cartInfo;
	info.RomFile = static_cast<VirtualFile>(_romPath);
	return info;
}

string BaseCartridge::GetSha1Hash()
{
	return SHA1::GetHash(_prgRom, _prgRomSize);
}

void BaseCartridge::LoadBattery()
{
	if(_saveRamSize > 0) {
		string saveFilePath = FolderUtilities::CombinePath(FolderUtilities::GetSaveFolder(), FolderUtilities::GetFilename(_romPath, false) + ".srm");
		VirtualFile saveFile(saveFilePath);
		if(saveFile.IsValid()) {
			vector<uint8_t> saveData;
			saveFile.ReadFile(saveData);

			if(saveData.size() == _saveRamSize) {
				memcpy(_saveRam, saveData.data(), _saveRamSize);
			}
		}
	}
}

void BaseCartridge::SaveBattery()
{
	if(_saveRamSize > 0) {
		string saveFilePath = FolderUtilities::CombinePath(FolderUtilities::GetSaveFolder(), FolderUtilities::GetFilename(_romPath, false) + ".srm");
		ofstream saveFile(saveFilePath, ios::binary);
		saveFile.write((char*)_saveRam, _saveRamSize);
	}
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
		_prgRomHandlers.push_back(unique_ptr<RomHandler>(new RomHandler(_prgRom, i, _prgRomSize, SnesMemoryType::PrgRom)));
	}

	for(uint32_t i = 0; i < _saveRamSize; i += 0x1000) {
		_saveRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(_saveRam, i, _saveRamSize, SnesMemoryType::SaveRam)));
	}

	if(_flags & CartFlags::LoRom) {
		MapBanks(mm, _prgRomHandlers, 0x00, 0x7D, 0x08, 0x0F, 0, true);
		MapBanks(mm, _prgRomHandlers, 0x80, 0xFF, 0x08, 0x0F, 0, true);
		if(_saveRamSize > 0) {
			MapBanks(mm, _saveRamHandlers, 0x70, 0x7D, 0x00, 0x07, 0, true);
			MapBanks(mm, _saveRamHandlers, 0xF0, 0xFF, 0x00, 0x07, 0, true);
		}
	} else {
		MapBanks(mm, _prgRomHandlers, 0x00, 0x3F, 0x08, 0x0F, 8, true);
		MapBanks(mm, _prgRomHandlers, 0x40, 0x7D, 0x00, 0x0F, 0, true);
		MapBanks(mm, _prgRomHandlers, 0x80, 0xBF, 0x08, 0x0F, 8, true);
		MapBanks(mm, _prgRomHandlers, 0xC0, 0xFF, 0x00, 0x0F, 0, true);

		MapBanks(mm, _saveRamHandlers, 0x20, 0x3F, 0x06, 0x07, 0, true);
		MapBanks(mm, _saveRamHandlers, 0xA0, 0xBF, 0x06, 0x07, 0, true);
	}
}

void BaseCartridge::Serialize(Serializer &s)
{
	s.StreamArray(_saveRam, _saveRamSize);
}

void BaseCartridge::DisplayCartInfo()
{
	int nameLength = 21;
	for(int i = 0; i < 21; i++) {
		if(_cartInfo.CartName[i] == 0) {
			nameLength = i;
			break;
		}
	}

	MessageManager::Log("-----------------------------");
	MessageManager::Log("Game: " + string(_cartInfo.CartName, nameLength));
	if(_flags & CartFlags::ExHiRom) {
		MessageManager::Log("Type: ExHiROM");
	} else if(_flags & CartFlags::ExLoRom) {
		MessageManager::Log("Type: ExLoROM");
	} else if(_flags & CartFlags::HiRom) {
		MessageManager::Log("Type: HiROM");
	} else if(_flags & CartFlags::LoRom) {
		MessageManager::Log("Type: LoROM");
	}

	if(_flags & CartFlags::FastRom) {
		MessageManager::Log("FastROM");
	}

	if(_flags & CartFlags::CopierHeader) {
		MessageManager::Log("Copier header found.");
	}

	MessageManager::Log("Map Mode: $" + HexUtilities::ToHex(_cartInfo.MapMode));
	MessageManager::Log("Rom Type: $" + HexUtilities::ToHex(_cartInfo.RomType));

	MessageManager::Log("File size: " + std::to_string(_prgRomSize / 1024) + " KB");
	MessageManager::Log("ROM size: " + std::to_string((0x400 << _cartInfo.RomSize) / 1024) + " KB");
	if(_saveRamSize > 0) {
		MessageManager::Log("SRAM size: " + std::to_string(_saveRamSize / 1024) + " KB");
	}
	MessageManager::Log("-----------------------------");
}