#pragma once
#include "pch.h"
#include "Utilities/StringUtilities.h"

uint32_t ReadBigEndian(uint8_t value[4])
{
	return value[3] | (value[2] << 8) | (value[1] << 16) | (value[0] << 24);
}

struct GbxCartridgeData
{
	char MapperId[4];
	uint8_t HasBattery;
	uint8_t HasRumble;
	uint8_t HasRtc;
	uint8_t Unused;

	uint8_t RomSize[4];
	uint8_t RamSize[4];
	uint8_t MapperVariables[8][4];
};

struct GbxMetadata
{
	uint8_t FooterSize[4];
	uint8_t MajorVersion[4];
	uint8_t MinorVersion[4];
	char Signature[4];

	bool IsValid()
	{
		return string(Signature, 4) == "GBX!" && ReadBigEndian(MajorVersion) == 1 && ReadBigEndian(FooterSize) == 0x40;
	}
};

struct GbxFooter
{
private:
	GbxCartridgeData _cartridge;
	GbxMetadata _metadata;

public:
	void Init(vector<uint8_t>& romData)
	{
		GbxMetadata meta = {};
		memcpy(&meta, romData.data() + romData.size() - sizeof(GbxMetadata), sizeof(GbxMetadata));
		if(meta.IsValid()) {
			memcpy(this, romData.data() + romData.size() - sizeof(GbxFooter), sizeof(GbxFooter));

			//Strip GBX footer from rom data
			romData.resize(romData.size() - ReadBigEndian(_metadata.FooterSize));
		}
	}

	bool IsValid()
	{
		return _metadata.IsValid();
	}

	string GetMapperId()
	{
		return StringUtilities::GetString(_cartridge.MapperId, 4);
	}

	uint32_t GetRomSize()
	{
		return ReadBigEndian(_cartridge.RomSize);
	}
	
	uint32_t GetRamSize()
	{
		return ReadBigEndian(_cartridge.RamSize);
	}

	bool HasBattery()
	{
		return (bool)_cartridge.HasBattery;
	}

	bool HasRtc()
	{
		return (bool)_cartridge.HasRtc;
	}

	bool HasRumble()
	{
		return (bool)_cartridge.HasRumble;
	}
};