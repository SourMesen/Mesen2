#pragma once
#include "pch.h"

#include "NES/NesTypes.h"
#include "NES/NesHeader.h"
#include "Shared/RomInfo.h"

enum class RomHeaderVersion
{
	iNes = 0,
	Nes2_0 = 1,
	OldiNes = 2
};

struct NsfHeader
{
	char Header[5] = {};
	uint8_t Version = 0;
	uint8_t TotalSongs = 0;
	uint8_t StartingSong = 0;
	uint16_t LoadAddress = 0;
	uint16_t InitAddress = 0;
	uint16_t PlayAddress = 0;
	char SongName[256] = {};
	char ArtistName[256] = {};
	char CopyrightHolder[256] = {};
	uint16_t PlaySpeedNtsc = 0;
	uint8_t BankSetup[8] = {};
	uint16_t PlaySpeedPal = 0;
	uint8_t Flags = 0;
	uint8_t SoundChips = 0;
	uint8_t Padding[4] = {};

	//NSFe extensions
	char RipperName[256] = {};
	vector<string> TrackNames;
	int32_t TrackLength[256] = {};
	int32_t TrackFade[256] = {};
};

struct GameInfo
{
	uint32_t Crc = 0;
	string System;
	string Board;
	string Pcb;
	string Chip;
	uint16_t MapperID = 0;
	uint32_t PrgRomSize = 0;
	uint32_t ChrRomSize = 0;
	uint32_t ChrRamSize = 0;
	uint32_t WorkRamSize = 0;
	uint32_t SaveRamSize = 0;
	bool HasBattery = false;
	string Mirroring;
	GameInputType InputType = {};
	string BusConflicts;
	string SubmapperID;
	VsSystemType VsType = {};
	PpuModel VsPpuModel = {};
};

struct NesRomInfo
{
	string RomName;
	string Filename;
	RomFormat Format = {};

	bool IsNes20Header = false;
	bool IsInDatabase = false;
	bool IsHeaderlessRom = false;

	uint32_t FilePrgOffset = 0;

	uint16_t MapperID = 0;
	uint8_t SubMapperID = 0;
	
	GameSystem System = GameSystem::Unknown;
	VsSystemType VsType = VsSystemType::Default;
	GameInputType InputType = GameInputType::Unspecified;
	PpuModel VsPpuModel = PpuModel::Ppu2C02;

	bool HasChrRam = false;
	bool HasBattery = false;
	bool HasEpsm = false;
	bool HasTrainer = false;
	MirroringType Mirroring = MirroringType::Horizontal;
	BusConflictType BusConflicts = BusConflictType::Default;

	HashInfo Hash = {};

	NesHeader Header = {};
	NsfHeader NsfInfo = {};
	GameInfo DatabaseInfo = {};
};

struct PageInfo
{
	uint32_t LeadInOffset = 0;
	uint32_t AudioOffset = 0;
	vector<uint8_t> Data;
};

struct StudyBoxData
{
	string FileName;
	vector<uint8_t> AudioFile;
	vector<PageInfo> Pages;
};

struct RomData
{
	NesRomInfo Info = {};

	int32_t ChrRamSize = -1;
	int32_t SaveChrRamSize = -1;
	int32_t SaveRamSize = -1;
	int32_t WorkRamSize = -1;

	vector<uint8_t> PrgRom;
	vector<uint8_t> ChrRom;
	vector<uint8_t> TrainerData;
	vector<vector<uint8_t>> FdsDiskData;
	vector<vector<uint8_t>> FdsDiskHeaders;
	StudyBoxData StudyBox = {};

	vector<uint8_t> RawData;

	bool Error = false;
	bool BiosMissing = false;
};
