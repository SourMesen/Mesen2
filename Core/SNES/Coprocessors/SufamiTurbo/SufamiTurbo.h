#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/RomHandler.h"
#include "SNES/MemoryMappings.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Shared/FirmwareHelper.h"
#include "Utilities/VirtualFile.h"

struct SufamiTurboFilePromptMessage
{
	char Filename[5000];
};

class SufamiTurbo
{
private:
	Emulator* _emu = nullptr;
	string _nameSlotA;

	uint8_t* _firmware = nullptr;
	uint32_t _firmwareSize = 0;

	string _cartName;
	uint8_t* _cartRom = nullptr;
	uint32_t _cartRomSize = 0;

	uint8_t* _cartRam = nullptr;
	uint32_t _cartRamSize = 0;

	vector<unique_ptr<IMemoryHandler>> _firmwareHandlers;
	vector<unique_ptr<IMemoryHandler>> _cartRomHandlers;
	vector<unique_ptr<IMemoryHandler>> _cartRamHandlers;

	SufamiTurbo() {}

public:
	static SufamiTurbo* Init(Emulator* emu, VirtualFile& slotA)
	{
		vector<uint8_t> firmware;
		if(!FirmwareHelper::LoadSufamiTurboFirmware(emu, firmware)) {
			return nullptr;
		}

		SufamiTurbo* st = new SufamiTurbo();
		st->_emu = emu;
		st->_nameSlotA = FolderUtilities::GetFilename(slotA.GetFileName(), false);

		st->_firmwareSize = (uint32_t)firmware.size();
		st->_firmware = new uint8_t[st->_firmwareSize];
		memcpy(st->_firmware, firmware.data(), firmware.size());
		BaseCartridge::EnsureValidPrgRomSize(st->_firmwareSize, st->_firmware);
		emu->RegisterMemory(MemoryType::SufamiTurboFirmware, st->_firmware, st->_firmwareSize);
		for(uint32_t i = 0; i < st->_firmwareSize; i += 0x1000) {
			st->_firmwareHandlers.push_back(unique_ptr<RomHandler>(new RomHandler(st->_firmware, i, st->_firmwareSize, MemoryType::SufamiTurboFirmware)));
		}

		SufamiTurboFilePromptMessage msg = {};
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::SufamiTurboFilePrompt, &msg);

		string slot2File = string(msg.Filename, strlen(msg.Filename));
		if(slot2File.size()) {
			VirtualFile file = slot2File;
			if(file.IsValid()) {
				vector<uint8_t> cart;
				file.ReadFile(cart);

				st->_cartName = FolderUtilities::GetFilename(file.GetFileName(), false);
				if(st->_nameSlotA == st->_cartName) {
					st->_cartName += "_SlotB";
				}

				st->_cartRomSize = (uint32_t)cart.size();
				st->_cartRom = new uint8_t[st->_cartRomSize];
				memcpy(st->_cartRom, cart.data(), cart.size());
				BaseCartridge::EnsureValidPrgRomSize(st->_cartRomSize, st->_cartRom);

				emu->RegisterMemory(MemoryType::SufamiTurboSecondCart, st->_cartRom, st->_cartRomSize);

				for(uint32_t i = 0; i < st->_cartRomSize; i += 0x1000) {
					st->_cartRomHandlers.push_back(unique_ptr<RomHandler>(new RomHandler(st->_cartRom, i, st->_cartRomSize, MemoryType::SufamiTurboSecondCart)));
				}

				st->_cartRamSize = GetSaveRamSize(cart);
				st->_cartRam = new uint8_t[st->_cartRamSize];
				emu->RegisterMemory(MemoryType::SufamiTurboSecondCartRam, st->_cartRam, st->_cartRamSize);
				memset(st->_cartRam, 0, st->_cartRamSize);

				emu->GetBatteryManager()->LoadBattery(st->_cartName + ".srm", st->_cartRam, st->_cartRamSize);

				for(uint32_t i = 0; i < st->_cartRamSize; i += 0x1000) {
					st->_cartRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(st->_cartRam, i, st->_cartRamSize, MemoryType::SufamiTurboSecondCartRam)));
				}
			}
		}

		return st;
	}

	static uint32_t GetSaveRamSize(vector<uint8_t>& cart)
	{
		auto checkMarker = [&](string marker) {
			return std::search((char*)cart.data(), (char*)cart.data()+cart.size(), marker.c_str(), marker.c_str()+marker.size()) != (char*)cart.data() + cart.size();
		};

		if(checkMarker("POIPOI.Ver") || checkMarker("SDBATTLE ")) {
			return 0x800;
		} else if(checkMarker("SD \xB6\xDE\xDD\xC0\xDE\xD1 GN")) {
			//SD ｶﾞﾝﾀﾞﾑ GN
			return 0x2000;
		}

		return 0;
	}

	void InitializeMappings(MemoryMappings& mm, vector<unique_ptr<IMemoryHandler>>& prgRomHandlers, vector<unique_ptr<IMemoryHandler>>& saveRamHandlers)
	{
		mm.RegisterHandler(0x20, 0x3F, 0x8000, 0xFFFF, prgRomHandlers);
		mm.RegisterHandler(0xA0, 0xBF, 0x8000, 0xFFFF, prgRomHandlers);

		mm.RegisterHandler(0x60, 0x63, 0x8000, 0xFFFF, saveRamHandlers);
		mm.RegisterHandler(0xE0, 0xE3, 0x8000, 0xFFFF, saveRamHandlers);

		mm.RegisterHandler(0x00, 0x1F, 0x8000, 0xFFFF, _firmwareHandlers);
		mm.RegisterHandler(0x80, 0x9F, 0x8000, 0xFFFF, _firmwareHandlers);

		mm.RegisterHandler(0x40, 0x5F, 0x8000, 0xFFFF, _cartRomHandlers);
		mm.RegisterHandler(0xC0, 0xDF, 0x8000, 0xFFFF, _cartRomHandlers);

		mm.RegisterHandler(0x70, 0x73, 0x8000, 0xFFFF, _cartRamHandlers);
		mm.RegisterHandler(0xF0, 0xF3, 0x8000, 0xFFFF, _cartRamHandlers);
	}

	void SaveBattery()
	{
		if(_cartRam) {
			_emu->GetBatteryManager()->SaveBattery(_cartName + ".srm", _cartRam, _cartRamSize);
		}
	}
	
	~SufamiTurbo()
	{
		delete[] _firmware;
		delete[] _cartRom;
		delete[] _cartRam;
	}
};