#pragma once
#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/HexUtilities.h"

class RomFinder
{
private:
	static string FindMatchingRom(Emulator* emu, string romName, uint32_t crc32)
	{
		if(emu->IsRunning() && emu->GetCrc32() == crc32) {
			//Current game matches
			return emu->GetRomInfo().RomFile;
		}

		string lcRomname = romName;
		std::transform(lcRomname.begin(), lcRomname.end(), lcRomname.begin(), ::tolower);

		vector<string> romFiles;
		vector<string> folders = FolderUtilities::GetKnownGameFolders();
		if(emu->IsRunning()) {
			//Look in the same folder as the current game first
			folders.insert(folders.begin(), emu->GetRomInfo().RomFile.GetFolderPath());
		}

		unordered_set<string> checkedFolders;
		for(string folder : folders) {
			if(!checkedFolders.emplace(folder).second) {
				//Already checked this folder
				continue;
			}

			for(string romFilename : FolderUtilities::GetFilesInFolder(folder, VirtualFile::RomExtensions, true)) {
				string lcRomFile = romFilename;
				std::transform(lcRomFile.begin(), lcRomFile.end(), lcRomFile.begin(), ::tolower);

				if(FolderUtilities::GetFilename(lcRomname, false) == FolderUtilities::GetFilename(lcRomFile, false) && VirtualFile(romFilename).GetCrc32() == crc32) {
					return romFilename;
				}
			}
		}

		MessageManager::Log("Could not find matching file: " + romName + "  CRC32: " + HexUtilities::ToHex(crc32, true));

		return "";
	}

public:
	static bool LoadMatchingRom(Emulator* emu, string romName, uint32_t crc32)
	{
		if(emu->IsRunning() && emu->GetCrc32() == crc32) {
			//Current game matches
			return true;
		}

		string match = FindMatchingRom(emu, romName, crc32);
		if(!match.empty()) {
			return emu->LoadRom(match, VirtualFile(""));
		}
		return false;
	}
};