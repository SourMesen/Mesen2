#include "pch.h"
#include "NES/RomData.h"
#include "NES/GameDatabase.h"
#include "NES/Loaders/UnifLoader.h"
#include "NES/Loaders/UnifBoards.h"
#include "Shared/MessageManager.h"
#include "Utilities/CRC32.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/HexUtilities.h"

std::unordered_map<uint32_t, GameInfo> GameDatabase::_gameDatabase;
bool GameDatabase::_enabled = true;
bool GameDatabase::_initialized = false;
SimpleLock GameDatabase::_loadLock;

template<typename T> 
T GameDatabase::ToInt(string value)
{
	if(value.empty()) {
		return 0;
	}
	return std::stoi(value);
}

void GameDatabase::LoadGameDb(vector<string> data)
{
	for(string &row : data) {
		vector<string> values = StringUtilities::Split(row, ',');
		if(values.size() >= 16) {
			GameInfo gameInfo;
			gameInfo.Crc = (uint32_t)std::stoll(values[0], nullptr, 16);
			gameInfo.System = values[1];
			gameInfo.Board = values[2];
			gameInfo.Pcb = values[3];
			gameInfo.Chip = values[4];
			gameInfo.MapperID = (uint16_t)ToInt<uint32_t>(values[5]);
			gameInfo.PrgRomSize = ToInt<uint32_t>(values[6]) * 1024;
			gameInfo.ChrRomSize = ToInt<uint32_t>(values[7]) * 1024;
			gameInfo.ChrRamSize = ToInt<uint32_t>(values[8]) * 1024;
			gameInfo.WorkRamSize = ToInt<uint32_t>(values[9]) * 1024;
			gameInfo.SaveRamSize = ToInt<uint32_t>(values[10]) * 1024;
			gameInfo.HasBattery = ToInt<uint32_t>(values[11]) == 0 ? false : true;
			gameInfo.Mirroring = values[12];
			gameInfo.InputType = (GameInputType)ToInt<uint32_t>(values[13]);
			gameInfo.BusConflicts = values[14];
			gameInfo.SubmapperID = values[15];
			gameInfo.VsType = (VsSystemType)ToInt<uint32_t>(values[16]);
			gameInfo.VsPpuModel = (PpuModel)ToInt<uint32_t>(values[17]);

			if(gameInfo.MapperID == 65000) {
				gameInfo.MapperID = UnifLoader::GetMapperID(gameInfo.Board);
			}

			_gameDatabase[gameInfo.Crc] = gameInfo;
		}
	}

	MessageManager::Log();
	MessageManager::Log("[DB] Initialized - " + std::to_string(_gameDatabase.size()) + " games in DB");
}

void GameDatabase::LoadGameDb(std::istream &db)
{
	vector<string> dbData;
	while(db.good()) {
		string lineContent;
		std::getline(db, lineContent);
		if(lineContent[lineContent.size() - 1] == '\r') {
			lineContent = lineContent.substr(0, lineContent.size() - 1);
		}
		if(lineContent.empty() || lineContent[0] == '#') {
			continue;
		}
		dbData.push_back(lineContent);
	}
	LoadGameDb(dbData);
}

void GameDatabase::InitDatabase()
{
	if(!_initialized) {
		auto lock = _loadLock.AcquireSafe();
		if(!_initialized) {
			string dbPath = FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "MesenNesDB.txt");
			ifstream db(dbPath, ios::in | ios::binary);
			LoadGameDb(db);
			_initialized = true;
		}
	}
}

BusConflictType GameDatabase::GetBusConflictType(string busConflictSetting)
{
	if(busConflictSetting.compare("Y") == 0) {
		return BusConflictType::Yes;
	} else if(busConflictSetting.compare("N") == 0) {
		return BusConflictType::No;
	}
	return BusConflictType::Default;
}

GameSystem GameDatabase::GetGameSystem(string system)
{
	if(system.compare("NesNtsc") == 0) {
		return GameSystem::NesNtsc;
	} else if(system.compare("NesPal") == 0) {
		return GameSystem::NesPal;
	} else if(system.compare("Famicom") == 0) {
		return GameSystem::Famicom;
	} else if(system.compare("VsSystem") == 0) {
		return GameSystem::VsSystem;
	} else if(system.compare("Dendy") == 0) {
		return GameSystem::Dendy;
	} else if(system.compare("Playchoice") == 0) {
		return GameSystem::Playchoice;
	}
	
	return GameSystem::NesNtsc;
}

uint8_t GameDatabase::GetSubMapper(GameInfo &info)
{
	if(!info.SubmapperID.empty()) {
		return ToInt<uint8_t>(info.SubmapperID);
	}
	return 0;
}

bool GameDatabase::GetDbRomSize(uint32_t romCrc, uint32_t &prgSize, uint32_t &chrSize)
{
	InitDatabase();
	auto result = _gameDatabase.find(romCrc);
	if(result != _gameDatabase.end()) {
		prgSize = result->second.PrgRomSize;
		chrSize = result->second.ChrRomSize;
		return true;
	}
	return false;
}

bool GameDatabase::GetiNesHeader(uint32_t romCrc, NesHeader &nesHeader)
{
	GameInfo info = {};
	InitDatabase();
	auto result = _gameDatabase.find(romCrc);
	if(result != _gameDatabase.end()) {
		info = result->second;

		nesHeader.Byte9 = 0;
		if(info.PrgRomSize > 4096*1024) {
			uint16_t prgSize = info.PrgRomSize / 0x4000;
			nesHeader.PrgCount = prgSize & 0xFF;
			nesHeader.Byte9 |= (prgSize & 0xF00) >> 8;
		} else {
			nesHeader.PrgCount = info.PrgRomSize / 0x4000;
		}

		if(info.ChrRomSize > 2048*1024) {
			uint16_t chrSize = info.ChrRomSize / 0x2000;
			nesHeader.ChrCount = chrSize & 0xFF;
			nesHeader.Byte9 |= (chrSize & 0xF00) >> 4;
		} else {
			nesHeader.ChrCount = info.ChrRomSize / 0x2000;
		}
		
		nesHeader.Byte6 = (info.MapperID & 0x0F) << 4;
		if(info.HasBattery) {
			nesHeader.Byte6 |= 0x02;
		}
		if(info.Mirroring.compare("v") == 0) {
			nesHeader.Byte6 |= 0x01;
		}

		nesHeader.Byte7 = (info.MapperID & 0xF0);
		GameSystem system = GetGameSystem(info.System);
		if(system == GameSystem::Playchoice) {
			nesHeader.Byte7 |= 0x02;
		} else if(system == GameSystem::VsSystem) {
			nesHeader.Byte7 |= 0x01;
		}
		
		//Don't set this, otherwise the header will be used over the game DB data
		//nesHeader.Byte7 |= 0x08; //NES 2.0 marker

		nesHeader.Byte8 = ((GetSubMapper(info) & 0x0F) << 4) | ((info.MapperID & 0xF00) >> 8);

		nesHeader.Byte10 = 0;
		if(info.SaveRamSize > 0) {
			nesHeader.Byte10 |= ((int)log2(info.SaveRamSize) - 6) << 4;
		}
		if(info.WorkRamSize > 0) {
			nesHeader.Byte10 |= ((int)log2(info.WorkRamSize) - 6);
		}

		nesHeader.Byte11 = 0;
		if(info.ChrRamSize > 0) {
			nesHeader.Byte11 |= ((int)log2(info.ChrRamSize) - 6);
		}
		
		nesHeader.Byte12 = system == GameSystem::NesPal ? 0x01 : 0;
		nesHeader.Byte13 = 0; //VS PPU variant

		return true;
	}

	return false;
}

void GameDatabase::SetGameInfo(uint32_t romCrc, RomData &romData, bool updateRomData, bool forHeaderlessRom)
{	
	GameInfo info = {};

	InitDatabase();

	auto result = _gameDatabase.find(romCrc);
	bool foundInDatabase = result != _gameDatabase.end();
	if(foundInDatabase) {
		info = result->second;
		MessageManager::Log("[DB] Game found in database");

		MessageManager::Log("[DB] Mapper: " + std::to_string(info.MapperID) + "  Sub: " + std::to_string(GetSubMapper(info)));
		if(info.MapperID < UnifBoards::UnknownBoard) {
			MessageManager::Log("[DB] Mapper: " + std::to_string(info.MapperID) + "  Sub: " + std::to_string(GetSubMapper(info)));
		} else if(info.MapperID == UnifBoards::UnknownBoard) {
			MessageManager::DisplayMessage("Error", "UnsupportedMapper", "UNIF: " + info.Board);
		}

		MessageManager::Log("[DB] System : " + info.System);

		if(GetGameSystem(info.System) == GameSystem::VsSystem) {
			string type = "VS-UniSystem";
			switch(info.VsType) {
				case VsSystemType::Default: break;
				case VsSystemType::IceClimberProtection: type = "VS-UniSystem (Ice Climbers)"; break;
				case VsSystemType::RaidOnBungelingBayProtection: type = "VS-DualSystem (Raid on Bungeling Bay)"; break;
				case VsSystemType::RbiBaseballProtection: type = "VS-UniSystem (RBI Baseball)"; break;
				case VsSystemType::SuperXeviousProtection: type = "VS-UniSystem (Super Xevious)"; break;
				case VsSystemType::TkoBoxingProtection: type = "VS-UniSystem (TKO Boxing)"; break;
				case VsSystemType::VsDualSystem: type = "VS-DualSystem"; break;
			}
			MessageManager::Log("[DB] VS System Type: " + type);
		}
		if(!info.Board.empty()) {
			MessageManager::Log("[DB] Board: " + info.Board);
		}
		if(!info.Chip.empty()) {
			MessageManager::Log("[DB] Chip: " + info.Chip);
		}

		switch(GetBusConflictType(info.BusConflicts)) {
			case BusConflictType::Default: break;
			case BusConflictType::Yes: MessageManager::Log("[DB] Bus conflicts: Yes"); break;
			case BusConflictType::No: MessageManager::Log("[DB] Bus conflicts: No"); break;
		}

		if(!info.Mirroring.empty()) {
			string msg = "[DB] Mirroring: ";
			switch(info.Mirroring[0]) {
				case 'h': msg += "Horizontal"; break;
				case 'v': msg += "Vertical"; break;
				case '4': msg += "4 Screens"; break;
				case '0': msg += "Screen A only"; break;
				case '1': msg += "Screen B only"; break;
			}
			MessageManager::Log(msg);
		}
		MessageManager::Log("[DB] PRG ROM: " + std::to_string(info.PrgRomSize / 1024) + " KB");
		MessageManager::Log("[DB] CHR ROM: " + std::to_string(info.ChrRomSize / 1024) + " KB");
		if(info.ChrRamSize > 0) {
			MessageManager::Log("[DB] CHR RAM: " + std::to_string(info.ChrRamSize / 1024) + " KB");
		}
		if(info.WorkRamSize > 0) {
			MessageManager::Log("[DB] Work RAM: " + std::to_string(info.WorkRamSize / 1024) + " KB");
		}
		if(info.SaveRamSize > 0) {
			MessageManager::Log("[DB] Save RAM: " + std::to_string(info.SaveRamSize / 1024) + " KB");
		}
		MessageManager::Log("[DB] Battery: " + string(info.HasBattery ? "Yes" : "No"));

		if(updateRomData) {
			MessageManager::Log("[DB] Database info will be used instead of file header.");
			romData.Info.IsInDatabase = true;
			UpdateRomData(info, romData);
		}

#ifdef _DEBUG
		MessageManager::DisplayMessage("DB", "Mapper: " + std::to_string(romData.Info.MapperID) + "  Sub: " + std::to_string(romData.Info.SubMapperID) + "  System: " + info.System);
#endif
	} else {
		MessageManager::Log("[DB] Game not found in database");
	}

	romData.Info.DatabaseInfo = info;
}

void GameDatabase::UpdateRomData(GameInfo &info, RomData &romData)
{
	romData.Info.MapperID = info.MapperID;
	romData.Info.System = GetGameSystem(info.System);
	if(romData.Info.System == GameSystem::VsSystem) {
		romData.Info.VsType = info.VsType;
		romData.Info.VsPpuModel = info.VsPpuModel;
	}
	romData.Info.InputType = info.InputType;
	romData.Info.SubMapperID = GetSubMapper(info);
	romData.Info.BusConflicts = GetBusConflictType(info.BusConflicts);

	bool isValidatedEntry = !info.SubmapperID.empty();
	if(isValidatedEntry || info.ChrRamSize > 0) {
		romData.ChrRamSize = info.ChrRamSize;
	}
	if(isValidatedEntry || info.WorkRamSize > 0) {
		romData.WorkRamSize = info.WorkRamSize;
	}
	if(isValidatedEntry || info.SaveRamSize > 0) {
		romData.SaveRamSize = info.SaveRamSize;
	}

	if(isValidatedEntry) {
		romData.Info.HasBattery = info.HasBattery;
	} else {
		romData.Info.HasBattery |= info.HasBattery;
	}

	if(!info.Mirroring.empty()) {
		switch(info.Mirroring[0]) {
			case 'h': romData.Info.Mirroring = MirroringType::Horizontal; break;
			case 'v': romData.Info.Mirroring = MirroringType::Vertical; break;
			case '4': romData.Info.Mirroring = MirroringType::FourScreens; break;
			case '0': romData.Info.Mirroring = MirroringType::ScreenAOnly; break;
			case '1': romData.Info.Mirroring = MirroringType::ScreenBOnly; break;
		}
	}
}
