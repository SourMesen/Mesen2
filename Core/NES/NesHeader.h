#pragma once
#include "NesTypes.h"

enum class RomHeaderVersion;

struct NesHeader
{
	/*
	Thing 	Archaic			 	iNES 									NES 2.0
	Byte 6	Mapper low nibble, Mirroring, Battery/Trainer flags
	Byte 7 	Unused 				Mapper high nibble, Vs. 		Mapper high nibble, NES 2.0 signature, PlayChoice, Vs.
	Byte 8 	Unused 				Total PRG RAM size (linear) 	Mapper highest nibble, mapper variant
	Byte 9 	Unused 				TV system 							Upper bits of ROM size
	Byte 10 	Unused 				Unused 								PRG RAM size (logarithmic; battery and non-battery)
	Byte 11 	Unused 				Unused 								VRAM size (logarithmic; battery and non-battery)
	Byte 12 	Unused 				Unused 								TV system
	Byte 13 	Unused 				Unused 								Vs. PPU variant
	*/
	char NES[4];
	uint8_t PrgCount;
	uint8_t ChrCount;
	uint8_t Byte6;
	uint8_t Byte7;
	uint8_t Byte8;
	uint8_t Byte9;
	uint8_t Byte10;
	uint8_t Byte11;
	uint8_t Byte12;
	uint8_t Byte13;
	uint8_t Byte14;
	uint8_t Byte15;

	uint16_t GetMapperID();
	bool HasBattery();
	bool HasTrainer();
	GameSystem GetNesGameSystem();
	GameSystem GetGameSystem();
	bool HasEpsm();
	RomHeaderVersion GetRomHeaderVersion();
	uint32_t GetSizeValue(uint32_t exponent, uint32_t multiplier);
	uint32_t GetPrgSize();
	uint32_t GetChrSize();
	int32_t GetWorkRamSize();
	int32_t GetSaveRamSize();
	int32_t GetChrRamSize();
	int32_t GetSaveChrRamSize();
	uint8_t GetSubMapper();
	MirroringType GetMirroringType();
	GameInputType GetInputType();
	VsSystemType GetVsSystemType();
	PpuModel GetVsSystemPpuModel();
};
