#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "CartTypes.h"
#include "../Utilities/ISerializable.h"

class MemoryManager;
class VirtualFile;

namespace CartFlags
{
	enum CartFlags
	{
		LoRom = 1,
		HiRom = 2,
		FastRom = 4,
		ExLoRom = 8,
		ExHiRom = 16,
		CopierHeader = 32
	};
}

class BaseCartridge : public ISerializable
{
private:
	vector<unique_ptr<IMemoryHandler>> _prgRomHandlers;
	vector<unique_ptr<IMemoryHandler>> _saveRamHandlers;
	SnesCartInformation _cartInfo;

	CartFlags::CartFlags _flags;
	string _romPath;

	uint8_t* _prgRom = nullptr;
	uint8_t* _saveRam = nullptr;
	
	uint32_t _prgRomSize = 0;
	uint32_t _saveRamSize = 0;

	void MapBanks(MemoryManager &mm, vector<unique_ptr<IMemoryHandler>> &handlers, uint8_t startBank, uint8_t endBank, uint16_t startPage = 0, uint16_t endPage = 0x0F, uint16_t pageIncrement = 0, bool mirror = false);
	
	void LoadBattery();
	void SaveBattery();

	int32_t GetHeaderScore(uint32_t addr);
	void DisplayCartInfo();

public:
	~BaseCartridge();

	static shared_ptr<BaseCartridge> CreateCartridge(VirtualFile &romFile, VirtualFile &patchFile);

	void Init();

	RomInfo GetRomInfo();
	string GetSha1Hash();

	void RegisterHandlers(MemoryManager &mm);

	uint8_t* DebugGetPrgRom() { return _prgRom; }
	uint8_t* DebugGetSaveRam() { return _saveRam; }
	uint32_t DebugGetPrgRomSize() { return _prgRomSize; }
	uint32_t DebugGetSaveRamSize() { return _saveRamSize; }

	void Serialize(Serializer &s) override;
};
