#pragma once
#include "stdafx.h"
#include "Console.h"
#include "NotificationManager.h"
#include "../Utilities/FolderUtilities.h"

struct MissingBiosMessage
{
	const char* Filename;
	CoprocessorType BiosType;
	uint32_t Size;
};

class BiosHelper
{
private:
	static bool AttemptLoadDspBios(string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &programRom, vector<uint8_t> &dataRom, uint32_t programSize, uint32_t dataSize)
	{
		VirtualFile combinedBios(FolderUtilities::CombinePath(FolderUtilities::GetBiosFolder(), combinedFilename));
		if(combinedBios.GetSize() == programSize + dataSize) {
			vector<uint8_t> biosData;
			combinedBios.ReadFile(biosData);
			programRom.insert(programRom.end(), biosData.begin(), biosData.begin() + programSize);
			dataRom.insert(dataRom.end(), biosData.begin() + programSize, biosData.end());
			return true;
		} else {
			VirtualFile splitBiosProg(FolderUtilities::CombinePath(FolderUtilities::GetBiosFolder(), splitFilenameProgram));
			VirtualFile splitBiosData(FolderUtilities::CombinePath(FolderUtilities::GetBiosFolder(), splitFilenameData));

			if(splitBiosProg.GetSize() == programSize && splitBiosData.GetSize() == dataSize) {
				splitBiosProg.ReadFile(programRom);
				splitBiosData.ReadFile(dataRom);
				return true;
			}
		}
		return false;
	}
public:
	static bool LoadDspBios(Console *console, CoprocessorType coprocessorType, string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &programRom, vector<uint8_t> &dataRom, uint32_t programSize = 0x1800, uint32_t dataSize = 0x800)
	{
		if(AttemptLoadDspBios(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MissingBiosMessage msg;
		msg.Filename = combinedFilename.c_str();
		msg.BiosType = coprocessorType;
		msg.Size = programSize + dataSize;
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingBios, &msg);

		//Try again in case the user selected a valid bios file
		if(AttemptLoadDspBios(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find BIOS file for DSP: " + combinedFilename);
		return false;
	}
};