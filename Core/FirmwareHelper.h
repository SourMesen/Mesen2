#pragma once
#include "stdafx.h"
#include "Console.h"
#include "NotificationManager.h"
#include "../Utilities/FolderUtilities.h"

struct MissingFirmwareMessage
{
	const char* Filename;
	CoprocessorType FirmwareType;
	uint32_t Size;
};

class FirmwareHelper
{
private:
	static bool AttemptLoadDspFirmware(string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &programRom, vector<uint8_t> &dataRom, uint32_t programSize, uint32_t dataSize)
	{
		VirtualFile combinedFirmware(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), combinedFilename));
		if(combinedFirmware.GetSize() == programSize + dataSize) {
			vector<uint8_t> firmwareData;
			combinedFirmware.ReadFile(firmwareData);
			programRom.insert(programRom.end(), firmwareData.begin(), firmwareData.begin() + programSize);
			dataRom.insert(dataRom.end(), firmwareData.begin() + programSize, firmwareData.end());
			return true;
		} else {
			VirtualFile splitFirmwareProg(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), splitFilenameProgram));
			VirtualFile splitFirmwareData(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), splitFilenameData));

			if(splitFirmwareProg.GetSize() == programSize && splitFirmwareData.GetSize() == dataSize) {
				splitFirmwareProg.ReadFile(programRom);
				splitFirmwareData.ReadFile(dataRom);
				return true;
			}
		}
		return false;
	}
public:
	static bool LoadDspFirmware(Console *console, CoprocessorType coprocessorType, string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &programRom, vector<uint8_t> &dataRom, uint32_t programSize = 0x1800, uint32_t dataSize = 0x800)
	{
		if(AttemptLoadDspFirmware(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MissingFirmwareMessage msg;
		msg.Filename = combinedFilename.c_str();
		msg.FirmwareType = coprocessorType;
		msg.Size = programSize + dataSize;
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		//Try again in case the user selected a valid firmware file
		if(AttemptLoadDspFirmware(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for DSP: " + combinedFilename);
		return false;
	}
};