#pragma once
#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/NotificationManager.h"
#include "Utilities/FolderUtilities.h"

enum class FirmwareType
{
	DSP1,
	DSP1B,
	DSP2,
	DSP3,
	DSP4,
	ST010,
	ST011,
	ST018,
	Satellaview,
	SufamiTurbo,
	Gameboy,
	GameboyColor,
	GameboyAdvance,
	Sgb1GameboyCpu,
	Sgb2GameboyCpu,
	SGB1,
	SGB2,
	FDS,
	StudyBox,
	PceSuperCd,
	PceGamesExpress,
	ColecoVision,
	WonderSwan,
	WonderSwanColor,
	SwanCrystal,
	Ymf288AdpcmRom,
	SmsBootRom,
	GgBootRom
};

struct MissingFirmwareMessage
{
	const char* Filename = {};
	FirmwareType Firmware;
	uint32_t Size = 0;
	uint32_t AltSize = 0;

	MissingFirmwareMessage(const char* filename, FirmwareType type, uint32_t size, uint32_t altSize = 0)
	{
		Filename = filename;
		Firmware = type;
		Size = size;
		AltSize = altSize;
	}
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
		VirtualFile firmware(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), "BS-X.bin"));
		if(firmware.IsValid() && firmware.GetSize() >= 0x8000) {
			*prgRom = new uint8_t[firmware.GetSize()];
			prgSize = (uint32_t)firmware.GetSize();
			firmware.ReadFile(*prgRom, (uint32_t)firmware.GetSize());
			return true;
		}

		return false;
	}

	static bool AttemptLoadFirmware(uint8_t** out, string filename, uint32_t size, string altFilename = "")
	{
		string path = FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), filename);
		VirtualFile firmware(path);
		if((!firmware.IsValid() || firmware.GetSize() != size) && !altFilename.empty()) {
			string altPath = FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), altFilename);
			firmware = VirtualFile(altPath);
		}

		if(firmware.IsValid() && firmware.GetSize() == size) {
			*out = new uint8_t[firmware.GetSize()];
			firmware.ReadFile(*out, (uint32_t)firmware.GetSize());
			return true;
		}

		return false;
	}

	static bool AttemptLoadFirmware(vector<uint8_t>& out, string filename, uint32_t size, string altFilename = "")
	{
		string path = FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), filename);
		VirtualFile firmware(path);
		if((!firmware.IsValid() || firmware.GetSize() != size) && !altFilename.empty()) {
			string altPath = FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), altFilename);
			firmware = VirtualFile(altPath);
		}

		if(firmware.IsValid() && firmware.GetSize() == size) {
			firmware.ReadFile(out);
			return true;
		}

		return false;
	}

public:
	static bool LoadDspFirmware(Emulator* emu, FirmwareType type, string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &programRom, vector<uint8_t> &dataRom, vector<uint8_t> &embeddedFirmware, uint32_t programSize = 0x1800, uint32_t dataSize = 0x800)
	{
		if(embeddedFirmware.size() == programSize + dataSize) {
			programRom.insert(programRom.end(), embeddedFirmware.begin(), embeddedFirmware.begin() + programSize);
			dataRom.insert(dataRom.end(), embeddedFirmware.begin() + programSize, embeddedFirmware.end());
			return true;
		} else if(AttemptLoadDspFirmware(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MissingFirmwareMessage msg(combinedFilename.c_str(), type, programSize + dataSize);

		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		//Try again in case the user selected a valid firmware file
		if(AttemptLoadDspFirmware(combinedFilename, splitFilenameProgram, splitFilenameData, programRom, dataRom, programSize, dataSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for DSP: " + combinedFilename);
		return false;
	}

	static bool LoadSt018Firmware(Emulator* emu, vector<uint8_t>& out)
	{
		string filename = "st018.rom";
		uint32_t size = 0x28000;
		if(AttemptLoadFirmware(out, filename, size)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::ST018, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(out, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for ST018");
		return false;
	}

	static bool LoadBsxFirmware(Emulator* emu, uint8_t** prgRom, uint32_t& prgSize)
	{
		if(AttemptLoadBsxFirmware(prgRom, prgSize)) {
			return true;
		}

		MissingFirmwareMessage msg("BS-X.bin", FirmwareType::Satellaview, 1024*1024);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);
		
		if(AttemptLoadBsxFirmware(prgRom, prgSize)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for BS-X");
		return false;
	}

	static bool LoadSufamiTurboFirmware(Emulator* emu, vector<uint8_t>& data)
	{
		string filename = "SufamiTurbo.sfc";
		
		if(AttemptLoadFirmware(data, filename, 0x40000)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::SufamiTurbo, 0x40000);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(data, filename, 0x40000)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for Sufami Turbo");
		return false;
	}

	static bool LoadSgbFirmware(Emulator* emu, uint8_t** prgRom, uint32_t& prgSize, bool useSgb2, bool promptForFirmware)
	{
		string filename = useSgb2 ? "SGB2.sfc" : "SGB1.sfc";
		prgSize = useSgb2 ? 0x80000 : 0x40000;
		if(AttemptLoadFirmware(prgRom, filename, prgSize)) {
			return true;
		}

		if(promptForFirmware) {
			MissingFirmwareMessage msg(filename.c_str(), useSgb2 ? FirmwareType::SGB2 : FirmwareType::SGB1, prgSize);
			emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

			if(AttemptLoadFirmware(prgRom, filename, prgSize)) {
				return true;
			}

			MessageManager::DisplayMessage("Error", "Could not find firmware file for Super Game Boy");
		}
		return false;
	}

	static bool LoadGbBootRom(Emulator* emu, uint8_t** bootRom, FirmwareType type)
	{
		string filename;
		string altFilename;
		switch(type) {
			default:
			case FirmwareType::Gameboy: filename = "dmg_boot.bin"; altFilename = "gb_bios.bin"; break;
			case FirmwareType::GameboyColor: filename = "cgb_boot.bin"; altFilename = "gbc_bios.bin"; break;
			case FirmwareType::Sgb1GameboyCpu: filename = "sgb_boot.bin"; altFilename = "sgb_bios.bin"; break;
			case FirmwareType::Sgb2GameboyCpu: filename = "sgb2_boot.bin"; altFilename = "sgb_bios.bin"; break;
		}

		uint32_t size = type == FirmwareType::GameboyColor ? 2304 : 256;
		if(AttemptLoadFirmware(bootRom, filename, size, altFilename)) {
			return true;
		}

		/*MissingFirmwareMessage msg(filename.c_str(), type, size);
		console->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find boot rom: " + filename);*/
		return false;
	}

	static bool LoadGbaBootRom(Emulator* emu, uint8_t** bootRom)
	{
		string filename = "gba_bios.bin";

		uint32_t size = 0x4000;
		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::GameboyAdvance, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find BIOS rom: " + filename);
		return false;
	}

	static bool LoadFdsFirmware(Emulator* emu, uint8_t** biosRom)
	{
		string filename = "disksys.rom";
		uint32_t size = 0x2000;
		if(AttemptLoadFirmware(biosRom, filename, size, "FdsBios.bin")) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::FDS, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(biosRom, filename, size, "FdsBios.bin")) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for Famicom Disk System");
		return false;
	}

	static bool LoadStudyBoxFirmware(Emulator* emu, uint8_t** biosRom)
	{
		string filename = "StudyBox.bin";
		uint32_t size = 0x40000;
		if(AttemptLoadFirmware(biosRom, filename, size, "StudyBox.bin")) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::StudyBox, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(biosRom, filename, size, "StudyBox.bin")) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for Study Box");
		return false;
	}

	static bool LoadPceSuperCdFirmware(Emulator* emu, vector<uint8_t>& biosRom)
	{
		string filename = "[BIOS] Super CD-ROM System (Japan) (v3.0).pce";
		string altName = "syscard3.pce";
		uint32_t size = 0x40000;
		if(AttemptLoadFirmware(biosRom, filename, size, altName)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::PceSuperCd, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(biosRom, filename, size, altName)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for PC Engine CD-ROM");
		return false;
	}

	static bool LoadPceGamesExpressFirmware(Emulator* emu, vector<uint8_t>& biosRom)
	{
		string filename = "[BIOS] Games Express CD Card (Japan).pce";
		string altName = "gecard.pce";
		if(AttemptLoadFirmware(biosRom, filename, 0x8000, altName) || AttemptLoadFirmware(biosRom, filename, 0x4000, altName)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::PceGamesExpress, 0x8000, 0x4000);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(biosRom, filename, 0x8000, altName) || AttemptLoadFirmware(biosRom, filename, 0x4000, altName)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for the Games Express Card");
		return false;
	}

	static bool LoadSmsBios(Emulator* emu, vector<uint8_t>& biosRom, bool forGameGear)
	{
		string filename = forGameGear ? "bios.gg" : "bios.sms";
		string path = FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), filename);
		VirtualFile firmware(path);
		if(firmware.IsValid()) {
			firmware.ReadFile(biosRom);
			return true;
		}
		return false;
	}

	static bool LoadColecoVisionBios(Emulator* emu, vector<uint8_t>& biosRom)
	{
		string filename = "bios.col";
		uint32_t size = 0x2000;
		if(AttemptLoadFirmware(biosRom, filename, size)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::ColecoVision, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(biosRom, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find firmware file for the ColecoVision");
		return false;
	}

	static bool LoadWsBootRom(Emulator* emu, vector<uint8_t>& bootRom, WsModel model)
	{
		string filename;
		FirmwareType firmwareType;
		switch(model) {
			default:
			case WsModel::Monochrome: filename = "bootrom.ws"; firmwareType = FirmwareType::WonderSwan; break;
			case WsModel::Color: filename = "bootrom.wsc"; firmwareType = FirmwareType::WonderSwanColor; break;
			case WsModel::SwanCrystal: filename = "bootrom_sc.wsc"; firmwareType = FirmwareType::SwanCrystal; break;
		}
		uint32_t size = model == WsModel::Monochrome ? 0x1000 : 0x2000;
		string path = FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), filename);
		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), firmwareType, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(bootRom, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find boot rom for the WonderSwan, skipping boot screen.");
		return false;
	}

	static bool LoadYmf288AdpcmRom(Emulator* emu, vector<uint8_t>& romData)
	{
		string filename = "ymf288_adpcm_rom.bin";
		uint32_t size = 0x2000;
		if(AttemptLoadFirmware(romData, filename, size)) {
			return true;
		}

		MissingFirmwareMessage msg(filename.c_str(), FirmwareType::Ymf288AdpcmRom, size);
		emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::MissingFirmware, &msg);

		if(AttemptLoadFirmware(romData, filename, size)) {
			return true;
		}

		MessageManager::DisplayMessage("Error", "Could not find ADPCM ROM for YMF288 (EPSM) - sound emulation will be incorrect.");
		return false;
	}

};