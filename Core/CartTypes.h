#pragma once
#include "stdafx.h"
#include "../Utilities/VirtualFile.h"

struct SnesCartInformation
{
	uint8_t MakerCode[2];
	uint8_t GameCode[4];
	uint8_t Reserved[7];
	uint8_t ExpansionRamSize;
	uint8_t SpecialVersion;
	uint8_t CartridgeType;

	char CartName[21];
	uint8_t MapMode;
	uint8_t RomType;
	uint8_t RomSize;
	uint8_t SramSize;

	uint8_t DestinationCode;
	uint8_t Reserved2;
	uint8_t Version;

	uint8_t ChecksumComplement[2];
	uint8_t Checksum[2];
};

struct RomInfo
{
	SnesCartInformation Header;
	VirtualFile RomFile;
	VirtualFile PatchFile;
};