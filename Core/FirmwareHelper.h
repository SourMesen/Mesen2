#pragma once
#include "stdafx.h"
#include "Console.h"
#include "NotificationManager.h"
#include "../Utilities/FolderUtilities.h"

struct MissingFirmwareMessage
{
	const char* Filename;
	FirmwareType Firmware;
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
	
	static bool AttemptLoadBsxFirmware(uint8_t** prgRom, uint32_t& prgSize)
	{
		VirtualFile firmware(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), "BsxBios.sfc"));
		if(firmware.IsValid() && firmware.GetSize() >= 0x8000) {
			*prgRom = new uint8_t[firmware.GetSize()];
			prgSize = (uint32_t)firmware.GetSize();
			firmware.ReadFile(*prgRom, (uint32_t)firmware.GetSize());
			return true;
		}

		return false;
	}

	static bool AttemptLoadFirmware(uint8_t** out, string filename, uint32_t size)
	{
		VirtualFile firmware(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), filename));
		if(firmware.IsValid() && firmware.GetSize() == size) {
			*out = new uint8_t[firmware.GetSize()];
			firmware.ReadFile(*out, (uint32_t)firmware.GetSize());
			return true;
		}

		return false;
	}

public:
	static bool LoadDspFirmware(Console *console, FirmwareType type, string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &programRom, vector<uint8_t> &dataRom, vector<uint8_t> &embeddedFirware, uint32_t programSize = 0x1800, uint32_t dataSize = 0x800)
	{
		if(embeddedFirware.size() == programSize + dataSize) {
			programRom.insert(programRom.end(), embeddedFirware.begin(), embeddedFirware.begin() + programSize);
			dataRom.insert(dataRom.end(), embeddedFirware.begin() + programSize, embeddedFirware.end());
			return true;
		} else if(AttemptLoadDspFirmware(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MissingFirmwareMessage msg;
		msg.Filename = combinedFilename.c_str();
		msg.Firmware = type;
		msg.Size = programSize + dataSize;
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		//Try again in case the user selected a valid firmware file
		if(AttemptLoadDspFirmware(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for DSP: " + combinedFilename);
		return false;
	}

	static bool LoadBsxFirmware(Console* console, uint8_t** prgRom, uint32_t& prgSize)
	{
		if(AttemptLoadBsxFirmware(prgRom, prgSize)) {
			return true;
		}

		MissingFirmwareMessage msg;
		msg.Filename = "BsxBios.sfc";
		msg.Firmware = FirmwareType::Satellaview;
		msg.Size = 1024*1024;
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);
		
		if(AttemptLoadBsxFirmware(prgRom, prgSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for BS-X");
		return false;
	}

	static bool LoadSgbFirmware(Console* console, uint8_t** prgRom, uint32_t& prgSize, bool useSgb2)
	{
		string filename = useSgb2 ? "SGB2.sfc" : "SGB1.sfc";
		prgSize = useSgb2 ? 0x80000 : 0x40000;
		if(AttemptLoadFirmware(prgRom, filename, prgSize)) {
			return true;
		}

		MissingFirmwareMessage msg;
		msg.Filename = filename.c_str();
		msg.Firmware = useSgb2 ? FirmwareType::SGB2 : FirmwareType::SGB1;
		msg.Size = prgSize;
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(prgRom, filename, prgSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for Super Game Boy");
		return false;
	}

	static bool LoadGbBootRom(Console* console, uint8_t** bootRom, FirmwareType type)
	{
		string filename;
		switch(type) {
			default:
			case FirmwareType::Gameboy: filename = "dmg_boot.bin"; break;
			case FirmwareType::GameboyColor: filename = "cgb_boot.bin"; break;
			case FirmwareType::Sgb1GameboyCpu: filename = "sgb_boot.bin"; break;
			case FirmwareType::Sgb2GameboyCpu: filename = "sgb2_boot.bin"; break;
		}

		uint32_t size = type == FirmwareType::GameboyColor ? 2304 : 256;
		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		/*MissingFirmwareMessage msg;
		msg.Filename = filename.c_str();
		msg.Firmware = type;
		msg.Size = size;
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find boot rom: " + filename);*/
		return false;
	}
};