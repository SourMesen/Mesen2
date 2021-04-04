#pragma once
#include "../stdafx.h"

#include "NesTypes.h"
#include "NesHeader.h"

enum class RomHeaderVersion
{
	iNes = 0,
	Nes2_0 = 1,
	OldiNes = 2
};

struct NsfHeader
{
	char Header[5];
	uint8_t Version;
	uint8_t TotalSongs;
	uint8_t StartingSong;
	uint16_t LoadAddress;
	uint16_t InitAddress;
	uint16_t PlayAddress;
	char SongName[256];
	char ArtistName[256];
	char CopyrightHolder[256];
	uint16_t PlaySpeedNtsc;
	uint8_t BankSetup[8];
	uint16_t PlaySpeedPal;
	uint8_t Flags;
	uint8_t SoundChips;
	uint8_t Padding[4];

	//NSFe extensions
	char RipperName[256];
	char TrackName[20000];
	int32_t TrackLength[256];
	int32_t TrackFade[256];
};

struct GameInfo
{
	uint32_t Crc;
	string System;
	string Board;
	string Pcb;
	string Chip;
	uint16_t MapperID;
	uint32_t PrgRomSize;
	uint32_t ChrRomSize;
	uint32_t ChrRamSize;
	uint32_t WorkRamSize;
	uint32_t SaveRamSize;
	bool HasBattery;
	string Mirroring;
	GameInputType InputType;
	string BusConflicts;
	string SubmapperID;
	VsSystemType VsType;
	PpuModel VsPpuModel;
};

struct NesRomInfo
{
	string RomName;
	string Filename;
	RomFormat Format;

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
	bool HasTrainer = false;
	MirroringType Mirroring = MirroringType::Horizontal;
	BusConflictType BusConflicts = BusConflictType::Default;

	HashInfo Hash;

	NesHeader NesHeader;
	NsfHeader NsfInfo;
	GameInfo DatabaseInfo;
};

struct PageInfo
{
	uint32_t LeadInOffset;
	uint32_t AudioOffset;
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
	NesRomInfo Info;

	int32_t ChrRamSize = -1;
	int32_t SaveChrRamSize = -1;
	int32_t SaveRamSize = -1;
	int32_t WorkRamSize = -1;
	
	vector<uint8_t> PrgRom;
	vector<uint8_t> ChrRom;
	vector<uint8_t> TrainerData;
	vector<vector<uint8_t>> FdsDiskData;
	vector<vector<uint8_t>> FdsDiskHeaders;
	StudyBoxData StudyBox;

	vector<uint8_t> RawData;

	bool Error = false;
	bool BiosMissing = false;
};
