#pragma once
#include "pch.h"

enum class CgbCompat : uint8_t
{
	Gameboy = 0x00,
	GameboyColorSupport = 0x80,
	GameboyColorExclusive = 0xC0,
};

struct GameboyHeader
{
	//Starts at 0x134
	char Title[11];
	char ManufacturerCode[4];
	CgbCompat CgbFlag;
	char LicenseeCode[2];
	uint8_t SgbFlag;
	uint8_t CartType;
	uint8_t PrgRomSize;
	uint8_t CartRamSize;
	uint8_t DestCode;
	uint8_t OldLicenseeCode;
	uint8_t MaskRomVersion;
	uint8_t HeaderChecksum;
	uint8_t GlobalChecksum[2];

	uint32_t GetCartRamSize()
	{
		if(CartType == 5 || CartType == 6) {
			//MBC2 has 512x4bits of cart ram
			return 0x200;
		}

		if(CartType == 0x22) {
			//MBC7 has a 256-byte eeprom
			return 0x100;
		}

		switch(CartRamSize) {
			case 0: return 0;
			case 1: return 0x800;
			case 2: return 0x2000;
			case 3: return 0x8000;
			case 4: return 0x20000;
			case 5: return 0x10000;
		}
		return 0;
	}

	bool HasBattery()
	{
		switch(CartType) {
			case 0x03: case 0x06: case 0x09: case 0x0D:
			case 0x0F: case 0x10: case 0x13: case 0x1B:
			case 0x1E: case 0x22: case 0xFF:
				return true;
		}

		return false;
	}

	string GetCartName()
	{
		int nameLength = 11;
		for(int i = 0; i < 11; i++) {
			if(Title[i] == 0) {
				nameLength = i;
				break;
			}
		}
		string name = string(Title, nameLength);

		size_t lastNonSpace = name.find_last_not_of(' ');
		if(lastNonSpace != string::npos) {
			return name.substr(0, lastNonSpace + 1);
		} else {
			return name;
		}
	}
};
