#include "stdafx.h"
#include <algorithm>
#include <unordered_set>
#include "Utilities/FolderUtilities.h"
#include "Utilities/CRC32.h"
#include "Utilities/sha1.h"
#include "Utilities/ArchiveReader.h"
#include "Utilities/VirtualFile.h"

#include "NES/Loaders/RomLoader.h"
#include "NES/Loaders/iNesLoader.h"
#include "NES/Loaders/FdsLoader.h"
#include "NES/NesHeader.h"
#include "NES/GameDatabase.h"

/*//TODO NES
#include "NsfLoader.h"
#include "NsfeLoader.h"
#include "UnifLoader.h"
#include "StudyBoxLoader.h"*/

bool RomLoader::LoadFile(VirtualFile &romFile)
{
	if(!romFile.IsValid()) {
		return false;
	}

	vector<uint8_t>& fileData = _romData.RawData;
	romFile.ReadFile(fileData);
	if(fileData.size() < 15) {
		return false;
	}

	_filename = romFile.GetFileName();
	string romName = FolderUtilities::GetFilename(_filename, true);

	bool skipSha1Hash = false;
	uint32_t crc = CRC32::GetCRC(fileData.data(), fileData.size());
	_romData.Info.Hash.Crc32 = crc;

	Log("");
	Log("Loading rom: " + romName);
	stringstream crcHex;
	crcHex << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << crc;
	Log("File CRC32: 0x" + crcHex.str());

	if(memcmp(fileData.data(), "NES\x1a", 4) == 0) {
		iNesLoader loader(_checkOnly);
		loader.LoadRom(_romData, fileData, nullptr);
	} else if(memcmp(fileData.data(), "FDS\x1a", 4) == 0 || memcmp(fileData.data(), "\x1*NINTENDO-HVC*", 15) == 0) {
		FdsLoader loader(_checkOnly);
		loader.LoadRom(_romData, fileData);
	} else if(memcmp(fileData.data(), "NESM\x1a", 5) == 0) {
		//TODO NES
		//NsfLoader loader(_checkOnly);
		//loader.LoadRom(_romData, fileData);
	} else if(memcmp(fileData.data(), "NSFE", 4) == 0) {
		//TODO NES
		//NsfeLoader loader(_checkOnly);
		//loader.LoadRom(_romData, fileData);
	} else if(memcmp(fileData.data(), "UNIF", 4) == 0) {
		//TODO NES
		//UnifLoader loader(_checkOnly);
		//loader.LoadRom(_romData, fileData);
	} else if(memcmp(fileData.data(), "STBX", 4) == 0) {
		//TODO NES
		//StudyBoxLoader loader(_checkOnly);
		//loader.LoadRom(_romData, fileData, romFile.GetFilePath());
		//skipSha1Hash = true;
	} else {
		NesHeader header = {};
		if(GameDatabase::GetiNesHeader(crc, header)) {
			Log("[DB] Headerless ROM file found - using game database data.");
			iNesLoader loader;
			loader.LoadRom(_romData, fileData, &header);
			_romData.Info.IsHeaderlessRom = true;
		} else {
			Log("Invalid rom file.");
			_romData.Error = true;
		}
	}

	if(!skipSha1Hash) {
		_romData.Info.Hash.Sha1 = SHA1::GetHash(fileData);
	}

	_romData.Info.RomName = romName;
	_romData.Info.Filename = _filename;

	if(_romData.Info.System == GameSystem::Unknown) {
		//Use filename to detect PAL/VS system games
		string name = _romData.Info.Filename;
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		if(name.find("(e)") != string::npos || name.find("(australia)") != string::npos || name.find("(europe)") != string::npos ||
			name.find("(germany)") != string::npos || name.find("(spain)") != string::npos) {
			_romData.Info.System = GameSystem::NesPal;
		} else if(name.find("(vs)") != string::npos) {
			_romData.Info.System = GameSystem::VsSystem;
		} else {
			_romData.Info.System = GameSystem::NesNtsc;
		}
	}

	return !_romData.Error;
}

RomData RomLoader::GetRomData()
{
	return _romData;
}

string RomLoader::FindMatchingRomInFile(string filePath, HashInfo hashInfo, int &iterationCount)
{
	shared_ptr<ArchiveReader> reader = ArchiveReader::GetReader(filePath);
	if(reader) {
		for(string file : reader->GetFileList(VirtualFile::RomExtensions)) {
			RomLoader loader(true);
			vector<uint8_t> fileData;
			VirtualFile innerFile(filePath, file);
			if(loader.LoadFile(innerFile)) {
				if(hashInfo.Crc32 == loader._romData.Info.Hash.Crc32 || hashInfo.Sha1.compare(loader._romData.Info.Hash.Sha1) == 0) {
					return innerFile;
				}
				
				iterationCount++;
				if(iterationCount > RomLoader::MaxFilesToCheck) {
					break;
				}
			}
		}
	} else {
		RomLoader loader(true);
		VirtualFile file = filePath;
		if(loader.LoadFile(file)) {
			if(hashInfo.Crc32 == loader._romData.Info.Hash.Crc32 || hashInfo.Sha1.compare(loader._romData.Info.Hash.Sha1) == 0) {
				return filePath;
			}
		}
		iterationCount++;
	}
	return "";
}

string RomLoader::FindMatchingRom(vector<string> romFiles, string romFilename, HashInfo hashInfo, bool useFastSearch)
{
	int iterationCount = 0;
	if(useFastSearch) {
		string lcRomFile = romFilename;
		std::transform(lcRomFile.begin(), lcRomFile.end(), lcRomFile.begin(), ::tolower);

		for(string currentFile : romFiles) {
			//Quick search by filename
			string lcCurrentFile = currentFile;
			std::transform(lcCurrentFile.begin(), lcCurrentFile.end(), lcCurrentFile.begin(), ::tolower);
			if(lcCurrentFile.find(lcRomFile) != string::npos && FolderUtilities::GetFilename(lcRomFile, true) == FolderUtilities::GetFilename(lcCurrentFile, true)) {
				string match = RomLoader::FindMatchingRomInFile(currentFile, hashInfo, iterationCount);
				if(!match.empty()) {
					return match;
				}
			}
		}
	} else {
		for(string romFile : romFiles) {
			//Slower search by CRC value
			string match = RomLoader::FindMatchingRomInFile(romFile, hashInfo, iterationCount);
			if(!match.empty()) {
				return match;
			}
			iterationCount++;

			if(iterationCount > RomLoader::MaxFilesToCheck) {
				MessageManager::Log("[RomLoader] Could not find a file matching the specified name/hash after 100 tries, giving up...");
				break;
			}
		}
	}

	return "";
}