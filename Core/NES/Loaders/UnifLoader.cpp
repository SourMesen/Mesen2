#include "pch.h"
#include "NES/Loaders/UnifLoader.h"
#include "NES/Loaders/UnifBoards.h"
#include "NES/RomData.h"
#include "NES/GameDatabase.h"
#include "Shared/MessageManager.h"
#include "Utilities/CRC32.h"
#include "Utilities/md5.h"
#include "Utilities/HexUtilities.h"

void UnifLoader::Read(uint8_t*& data, uint8_t& dest)
{
	dest = data[0];
	data++;
}

void UnifLoader::Read(uint8_t*& data, uint32_t& dest)
{
	dest = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	data += 4;
}

void UnifLoader::Read(uint8_t*& data, uint8_t* dest, size_t len)
{
	memcpy(dest, data, len);
	data += len;
}

string UnifLoader::ReadString(uint8_t*& data, uint8_t* chunkEnd)
{
	stringstream ss;
	while(data < chunkEnd) {
		if(data[0] == 0) {
			//end of string
			data = chunkEnd;
			break;
		} else {
			if(data[0] != ' ') {
				//Ignore spaces
				ss << (char)data[0];
			}
		}
		data++;
	}

	return ss.str();
}

string UnifLoader::ReadFourCC(uint8_t*& data)
{
	stringstream ss;
	for(int i = 0; i < 4; i++) {
		ss << (char)data[i];
	}
	data += 4;
	return ss.str();
}

bool UnifLoader::ReadChunk(uint8_t*& data, uint8_t* dataEnd, RomData& romData)
{
	if(data + 8 > dataEnd) {
		return false;
	}

	string fourCC = ReadFourCC(data);

	uint32_t length;
	Read(data, length);

	uint8_t* chunkEnd = data + length;
	if(chunkEnd > dataEnd) {
		return false;
	}

	if(fourCC.compare("MAPR") == 0) {
		_mapperName = ReadString(data, chunkEnd);
		if(_mapperName.size() > 0) {
			romData.Info.MapperID = GetMapperID(_mapperName);
			if(romData.Info.MapperID == UnifBoards::UnknownBoard) {
				Log("[UNIF] Error: Unknown board");
			}
		} else {
			romData.Error = true;
			return false;
		}
	} else if(fourCC.substr(0, 3).compare("PRG") == 0) {
		uint32_t chunkNumber;
		std::stringstream ss;
		ss << std::hex << fourCC[3];
		ss >> chunkNumber;

		_prgChunks[chunkNumber].resize(length);
		Read(data, _prgChunks[chunkNumber].data(), length);
	} else if(fourCC.substr(0, 3).compare("CHR") == 0) {
		uint32_t chunkNumber;
		std::stringstream ss;
		ss << std::hex << fourCC[3];
		ss >> chunkNumber;

		_chrChunks[chunkNumber].resize(length);
		Read(data, _chrChunks[chunkNumber].data(), length);
	} else if(fourCC.compare("TVCI") == 0) {
		uint8_t value;
		Read(data, value);
		romData.Info.System = value == 1 ? GameSystem::NesPal : GameSystem::NesNtsc;
	} else if(fourCC.compare("CTRL") == 0) {
		//not supported
	} else if(fourCC.compare("BATR") == 0) {
		uint8_t value;
		Read(data, value);
		romData.Info.HasBattery = value > 0;
	} else if(fourCC.compare("MIRR") == 0) {
		uint8_t value;
		Read(data, value);

		switch(value) {
			default:
			case 0: romData.Info.Mirroring = MirroringType::Horizontal; break;
			case 1: romData.Info.Mirroring = MirroringType::Vertical; break;
			case 2: romData.Info.Mirroring = MirroringType::ScreenAOnly; break;
			case 3: romData.Info.Mirroring = MirroringType::ScreenBOnly; break;
			case 4: romData.Info.Mirroring = MirroringType::FourScreens; break;
		}
	} else {
		//Unsupported/unused FourCCs: PCKn, CCKn, NAME, WRTR, READ, DINF, VROR
	}

	data = chunkEnd;

	return true;
}

int32_t UnifLoader::GetMapperID(string mapperName)
{
	string prefix = mapperName.substr(0, 4);
	if(prefix.compare("NES-") == 0 || prefix.compare("UNL-") == 0 || prefix.compare("HVC-") == 0 || prefix.compare("BTL-") == 0 || prefix.compare("BMC-") == 0) {
		mapperName = mapperName.substr(4);
	}

	auto result = _boardMappings.find(mapperName);
	if(result != _boardMappings.end()) {
		return result->second;
	}

	return UnifBoards::UnknownBoard;
}

void UnifLoader::LoadRom(RomData& romData, vector<uint8_t>& romFile, bool databaseEnabled)
{
	//Skip header, version & null bytes, start reading at first chunk
	uint8_t* data = romFile.data() + 32;
	uint8_t* endOfFile = romFile.data() + romFile.size();

	while(ReadChunk(data, endOfFile, romData)) {
		//Read all chunks
	}

	for(int i = 0; i < 16; i++) {
		romData.PrgRom.insert(romData.PrgRom.end(), _prgChunks[i].begin(), _prgChunks[i].end());
		romData.ChrRom.insert(romData.ChrRom.end(), _chrChunks[i].begin(), _chrChunks[i].end());
	}

	if(romData.PrgRom.size() == 0 || _mapperName.empty()) {
		romData.Error = true;
	} else {
		vector<uint8_t> fullRom;
		fullRom.insert(fullRom.end(), romData.PrgRom.begin(), romData.PrgRom.end());
		fullRom.insert(fullRom.end(), romData.ChrRom.begin(), romData.ChrRom.end());

		romData.Info.Format = RomFormat::Unif;
		romData.Info.Hash.PrgCrc32 = CRC32::GetCRC(romData.PrgRom.data(), romData.PrgRom.size());
		romData.Info.Hash.PrgChrCrc32 = CRC32::GetCRC(fullRom.data(), fullRom.size());

		Log("PRG+CHR CRC32: 0x" + HexUtilities::ToHex(romData.Info.Hash.PrgChrCrc32));
		Log("[UNIF] Board Name: " + _mapperName);
		Log("[UNIF] PRG ROM: " + std::to_string(romData.PrgRom.size() / 1024) + " KB");
		Log("[UNIF] CHR ROM: " + std::to_string(romData.ChrRom.size() / 1024) + " KB");
		if(romData.ChrRom.size() == 0) {
			Log("[UNIF] CHR RAM: 8 KB");
		}

		string mirroringType;
		switch(romData.Info.Mirroring) {
			case MirroringType::Horizontal: mirroringType = "Horizontal"; break;
			case MirroringType::Vertical: mirroringType = "Vertical"; break;
			case MirroringType::ScreenAOnly: mirroringType = "1-Screen (A)"; break;
			case MirroringType::ScreenBOnly: mirroringType = "1-Screen (B)"; break;
			case MirroringType::FourScreens: mirroringType = "Four Screens"; break;
		}

		Log("[UNIF] Mirroring: " + mirroringType);
		Log("[UNIF] Battery: " + string(romData.Info.HasBattery ? "Yes" : "No"));

		GameDatabase::SetGameInfo(romData.Info.Hash.PrgChrCrc32, romData, databaseEnabled, false);

		if(romData.Info.MapperID == UnifBoards::UnknownBoard) {
			MessageManager::DisplayMessage("Error", "UnsupportedMapper", "UNIF: " + _mapperName);
			romData.Error = true;
		}
	}
}

std::unordered_map<string, int> UnifLoader::_boardMappings = std::unordered_map<string, int> {
	{ "11160", 299 },
	{ "12-IN-1", 331 },
	{ "13in1JY110", UnifBoards::UnknownBoard },
	{ "190in1", 300 },
	{ "22211", 132 },
	{ "255in1", UnifBoards::Unl255in1 }, //Doesn't actually exist as a UNIF file (used to assign a mapper to the 255-in-1 rom)
	{ "3D-BLOCK", UnifBoards::UnknownBoard },
	{ "411120-C", 287 },
	{ "42in1ResetSwitch", 226 },
	{ "43272", 227 },
	{ "603-5052", 238 },
	{ "64in1NoRepeat", 314 },
	{ "70in1", 236 },
	{ "70in1B", 236 },
	{ "810544-C-A1", 261 },
	{ "830425C-4391T", 320 },
	{ "8157", 301 },
	{ "8237", 215 },
	{ "8237A", UnifBoards::Unl8237A },
	{ "830118C", 348 },
	{ "A65AS", 285 },
	{ "AC08", UnifBoards::Ac08 },
	{ "ANROM", 7 },
	{ "AX5705", 530 },
	{ "BB", 108 },
	{ "BS-5", 286 },
	{ "CC-21", UnifBoards::Cc21 },
	{ "CITYFIGHT", 266 },
	{ "COOLBOY", 268 },
	{ "10-24-C-A1", UnifBoards::UnknownBoard },
	{ "CNROM", 3 },
	{ "CPROM", 13 },
	{ "D1038", 59 },
	{ "DANCE", UnifBoards::UnknownBoard },
	{ "DANCE2000", 518 },
	{ "DREAMTECH01", 521 },
	{ "EDU2000", 329 },
	{ "EKROM", 5 },
	{ "ELROM", 5 },
	{ "ETROM", 5 },
	{ "EWROM", 5 },
	{ "FARID_SLROM_8-IN-1", 323 },
	{ "FARID_UNROM_8-IN-1", 324 },
	{ "FK23C", 176 },
	{ "FK23CA", 176 },
	{ "FS304", 162 },
	{ "G-146", 349 },
	{ "GK-192", 58 },
	{ "GS-2004", 283 },
	{ "GS-2013", UnifBoards::Gs2013 },
	{ "Ghostbusters63in1", UnifBoards::Ghostbusters63in1 },
	{ "H2288", 123 },
	{ "HKROM", 4 },
	{ "KOF97", 263 },
	{ "KONAMI-QTAI", 190 },
	{ "K-3046", 336 },
	{ "KS7010", UnifBoards::UnknownBoard },
	{ "KS7012", 346 },
	{ "KS7013B", 312 },
	{ "KS7016", 306 },
	{ "KS7017", 303 },
	{ "KS7030", UnifBoards::UnknownBoard },
	{ "KS7031", 305 },
	{ "KS7032", 142 },
	{ "KS7037", 307 },
	{ "KS7057", 302 },
	{ "LE05", UnifBoards::UnknownBoard },
	{ "LH10", 522 },
	{ "LH32", 125 },
	{ "LH51", 309 },
	{ "LH53", UnifBoards::UnknownBoard },
	{ "MALISB", 325 },
	{ "MARIO1-MALEE2", UnifBoards::Malee },
	{ "MHROM", 66 },
	{ "N625092", 221 },
	{ "NROM", 0 },
	{ "NROM-128", 0 },
	{ "NROM-256", 0 },
	{ "NTBROM", 68 },
	{ "NTD-03", 290 },
	{ "NovelDiamond9999999in1", 201 },
	{ "OneBus", UnifBoards::UnknownBoard },
	{ "PEC-586", UnifBoards::UnknownBoard },
	{ "PUZZLE", UnifBoards::UnlPuzzle }, //Doesn't actually exist as a UNIF file (used to reassign a new mapper number to the Puzzle beta)
	{ "RESET-TXROM", 313 },
	{ "RET-CUFROM", 29 },
	{ "RROM", 0 },
	{ "RROM-128", 0 },
	{ "SA-002", 136 },
	{ "SA-0036", 149 },
	{ "SA-0037", 148 },
	{ "SA-009", 160 },
	{ "SA-016-1M", 146 },
	{ "SA-72007", 145 },
	{ "SA-72008", 133 },
	{ "SA-9602B", 513 },
	{ "SA-NROM", 143 },
	{ "SAROM", 1 },
	{ "SBROM", 1 },
	{ "SC-127", 35 },
	{ "SCROM", 1 },
	{ "SEROM", 1 },
	{ "SGROM", 1 },
	{ "SHERO", 262 },
	{ "SKROM", 1 },
	{ "SL12", 116 },
	{ "SL1632", 14 },
	{ "SL1ROM", 1 },
	{ "SLROM", 1 },
	{ "SMB2J", 304 },
	{ "SNROM", 1 },
	{ "SOROM", 1 },
	{ "SSS-NROM-256", UnifBoards::SssNrom256 },
	{ "SUNSOFT_UNROM", 93 },
	{ "Sachen-74LS374N", 150 },
	{ "Sachen-74LS374NA", 243 },
	{ "Sachen-8259A", 141 },
	{ "Sachen-8259B", 138 },
	{ "Sachen-8259C", 139 },
	{ "Sachen-8259D", 137 },
	{ "Super24in1SC03", 176 },
	{ "SuperHIK8in1", 45 },
	{ "Supervision16in1", 53 },
	{ "T-227-1", UnifBoards::UnknownBoard },
	{ "T-230", 529 },
	{ "T-262", 265 },
	{ "TBROM", 4 },
	{ "TC-U01-1.5M", 147 },
	{ "TEK90", 90 },
	{ "TEROM", 4 },
	{ "TF1201", 298 },
	{ "TFROM", 4 },
	{ "TGROM", 4 },
	{ "TKROM", 4 },
	{ "TKSROM", 4 },
	{ "TLROM", 4 },
	{ "TLSROM", 4 },
	{ "TQROM", 4 },
	{ "TR1ROM", 4 },
	{ "TSROM", 4 },
	{ "TVROM", 4 },
	{ "Transformer", UnifBoards::UnknownBoard },
	{ "UNROM", 2 },
	{ "UNROM-512-8", 30 },
	{ "UNROM-512-16", 30 },
	{ "UNROM-512-32", 30 },
	{ "UOROM", 2 },
	{ "VRC7", 85 },
	{ "YOKO", 264 },
	{ "SB-2000", UnifBoards::UnknownBoard },
	{ "158B", 258 },
	{ "DRAGONFIGHTER", 292 },
	{ "EH8813A", 519 },
	{ "HP898F", 319 },
	{ "F-15", 259 },
	{ "RT-01", 328 },
	{ "81-01-31-C", UnifBoards::UnknownBoard },
	{ "8-IN-1", 333 },
	{ "WS", 332 },
	{ "80013-B", 274 },
	{ "WAIXING-FW01", 227 },
	{ "WAIXING-FS005", UnifBoards::UnknownBoard },
	{ "HPxx", 260 },
	{ "HP2018A", 260 },
	{ "DRIPGAME", 284 },
	{ "60311C", 289 },
	{ "CHINA_ER_SAN2", 19 }, //Appears to be a mapper 19 hack specific for VirtuaNES (which adds chinese text on top of the PPU's output), unknown if a board actually exists
};
