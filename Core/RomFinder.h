#pragma once
#include "stdafx.h"
#include "Console.h"
#include "BaseCartridge.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/FolderUtilities.h"

class RomFinder
{
public:
	static bool LoadMatchingRom(Console* console, string romName, string sha1Hash)
	{
		if(console->IsRunning() && console->GetCartridge()->GetSha1Hash() == sha1Hash) {
			//Current game matches
			return true;
		}

		string match = FindMatchingRom(console, romName, sha1Hash);
		if(!match.empty()) {
			return console->LoadRom(match, VirtualFile(""));
		}
		return false;
	}

	static string FindMatchingRom(Console* console, string romName, string sha1Hash)
	{
		if(console->IsRunning() && console->GetCartridge()->GetSha1Hash() == sha1Hash) {
			//Current game matches
			return console->GetRomInfo().RomFile;
		}

		string lcRomname = romName;
		std::transform(lcRomname.begin(), lcRomname.end(), lcRomname.begin(), ::tolower);
		std::transform(romName.begin(), romName.end(), romName.begin(), ::tolower);

		vector<string> romFiles;
		for(string folder : FolderUtilities::GetKnownGameFolders()) {
			for(string romFilename : FolderUtilities::GetFilesInFolder(folder, VirtualFile::RomExtensions, true)) {
				string lcRomFile = romFilename;
				std::transform(lcRomFile.begin(), lcRomFile.end(), lcRomFile.begin(), ::tolower);
				if(FolderUtilities::GetFilename(romName, false) == FolderUtilities::GetFilename(lcRomFile, false) && VirtualFile(romFilename).GetSha1Hash() == sha1Hash) {
					return romFilename;
				}
			}
		}

		return "";
	}
};