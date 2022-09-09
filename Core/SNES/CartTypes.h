#pragma once
#include "pch.h"
#include "Utilities/VirtualFile.h"

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
	uint8_t DeveloperId;
	uint8_t Version;

	uint8_t ChecksumComplement[2];
	uint8_t Checksum[2];
	uint8_t CpuVectors[0x20];
};

enum class CoprocessorType
{
	None,
	DSP1,
	DSP1B,
	DSP2,
	DSP3,
	DSP4,
	GSU,
	OBC1,
	SA1,
	SDD1,
	RTC,
	Satellaview,
	SPC7110,
	ST010,
	ST011,
	ST018,
	CX4,
	SGB
};

namespace CartFlags
{
	enum CartFlags
	{
		None = 0,
		LoRom = 1,
		HiRom = 2,
		FastRom = 4,
		ExLoRom = 8,
		ExHiRom = 16,
		CopierHeader = 32
	};
}
