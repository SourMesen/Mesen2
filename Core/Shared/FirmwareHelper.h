#pragma once
#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/NotificationManager.h"
#include "Utilities/FolderUtilities.h"

enum class FirmwareType
{
	CX4,
	DSP1,
	DSP1B,
	DSP2,
	DSP3,
	DSP4,
	ST010,
	ST011,
	ST018,
	Satellaview,
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
	Ymf288AdpcmRom
};

struct MissingFirmwareMessage
{
	const char* Filename = {};
	FirmwareType Firmware;
	uint32_t Size = 0;
	uint32_t AltSize = 0;
	const char* FileHashes[5] = {};

	MissingFirmwareMessage(const char* filename, FirmwareType type, uint32_t size, uint32_t altSize = 0)
	{
		Filename = filename;
		Firmware = type;
		Size = size;
		AltSize = altSize;

		switch(type) {
			case FirmwareType::CX4: FileHashes[0] = "AE8D4D1961B93421FF00B3CAA1D0F0CE7783E749772A3369C36B3DBF0D37EF18"; break;
			case FirmwareType::DSP1: FileHashes[0] = "91E87D11E1C30D172556BED2211CCE2EFA94BA595F58C5D264809EF4D363A97B"; break;
			case FirmwareType::DSP1B: FileHashes[0] = "D789CB3C36B05C0B23B6C6F23BE7AA37C6E78B6EE9CEAC8D2D2AA9D8C4D35FA9"; break;
			case FirmwareType::DSP2: FileHashes[0] = "03EF4EF26C9F701346708CB5D07847B5203CF1B0818BF2930ACD34510FFDD717"; break;
			case FirmwareType::DSP3: FileHashes[0] = "0971B08F396C32E61989D1067DDDF8E4B14649D548B2188F7C541B03D7C69E4E"; break;
			case FirmwareType::DSP4: FileHashes[0] = "752D03B2D74441E430B7F713001FA241F8BBCFC1A0D890ED4143F174DBE031DA"; break;
			case FirmwareType::ST010: FileHashes[0] = "FA9BCED838FEDEA11C6F6ACE33D1878024BDD0D02CC9485899D0BDD4015EC24C"; break;
			case FirmwareType::ST011: FileHashes[0] = "8B2B3F3F3E6E29F4D21D8BC736B400BC988B7D2214EBEE15643F01C1FEE2F364"; break;
			case FirmwareType::ST018: FileHashes[0] = "6DF209AB5D2524D1839C038BE400AE5EB20DAFC14A3771A3239CD9E8ACD53806"; break;

			case FirmwareType::Satellaview:
				FileHashes[0] = "27CFDB99F7E4252BF3740D420147B63C4C88616883BC5E7FE43F2F30BF8C8CBB"; //Japan, no DRM
				FileHashes[1] = "A49827B45FF9AC9CF5B4658190E1428E59251BC82D8A63D8E9E0F71E439F008F"; //English, no DRM
				FileHashes[2] = "3CE321496EDC5D77038DE2034EB3FB354D7724AFD0BC7FD0319F3EB5D57B984D"; //Japan, original
				FileHashes[3] = "77D94D64D745014BF8B51280A4204056CDEB9D41EA30EAE80DBC006675BEBEF8"; //English, DRM
				break;

			case FirmwareType::Gameboy:
				FileHashes[0] = "CF053ECCB4CCAFFF9E67339D4E78E98DCE7D1ED59BE819D2A1BA2232C6FCE1C7";
				FileHashes[1] = "26E71CF01E301E5DC40E987CD2ECBF6D0276245890AC829DB2A25323DA86818E";
				break;

			case FirmwareType::GameboyColor:
				FileHashes[0] = "B4F2E416A35EEF52CBA161B159C7C8523A92594FACB924B3EDE0D722867C50C7";
				FileHashes[1] = "3A307A41689BEE99A9A32EA021BF45136906C86B2E4F06C806738398E4F92E45";
				break;

			case FirmwareType::GameboyAdvance:
				FileHashes[0] = "FD2547724B505F487E6DCB29EC2ECFF3AF35A841A77AB2E85FD87350ABD36570";
				break;

			case FirmwareType::Sgb1GameboyCpu: FileHashes[0] = "0E4DDFF32FC9D1EEAAE812A157DD246459B00C9E14F2F61751F661F32361E360"; break;
			case FirmwareType::Sgb2GameboyCpu: FileHashes[0] = "FD243C4FB27008986316CE3DF29E9CFBCDC0CD52704970555A8BB76EDBEC3988"; break;

			case FirmwareType::SGB1:
				FileHashes[0] = "BBA9C269273BEDB9B38BD5EB23BFAA6E509B8DECC7CB80BB5513905AF04F4CEB"; //Rev 0 (Japan)
				FileHashes[1] = "C6C4DAAB5C899B69900C460787DE6089EDABE94B760F96D9F583D30CC0A5BB30"; //Rev 1 (Japan+USA)
				FileHashes[2] = "A75160F7B89B1F0E20FD2F6441BB86285C7378DB5035EF6885485EAFF6059376"; //Rev 2 (World)
				break;

			case FirmwareType::SGB2:
				FileHashes[0] = "C172498A23D1176672931BAB33B629C7D28F914A43DCA9E540B8AF1B37CCF2C6"; //SGB2 (Japan)
				break;

			case FirmwareType::FDS:
				FileHashes[0] = "99C18490ED9002D9C6D999B9D8D15BE5C051BDFA7CC7E73318053C9A994B0178";
				FileHashes[1] = "A0A9D57CBACE21BF9C85C2B85E86656317F0768D7772ACC90C7411AB1DBFF2BF";
				break;

			case FirmwareType::StudyBox:
				FileHashes[0] = "365F84C86F7F7C3AAA2042D78494D41448E998EC5A89AC1B5FECB452951D514C";
				break;

			case FirmwareType::PceSuperCd:
				FileHashes[0] = "E11527B3B96CE112A037138988CA72FD117A6B0779C2480D9E03EAEBECE3D9CE";
				break;

			case FirmwareType::PceGamesExpress:
				FileHashes[0] = "4B86BB96A48A4CA8375FC0109631D0B1D64F255A03B01DE70594D40788BA6C3D";
				FileHashes[1] = "DA173B20694C2B52087B099B8C44E471D3EE08A666C90D4AFD997F8E1382ADD8";
				break;

			case FirmwareType::ColecoVision:
				FileHashes[0] = "990BF1956F10207D8781B619EB74F89B00D921C8D45C95C334C16C8CCECA09AD";
				FileHashes[1] = "FB4A898EB93B19B36773D87A6C70EB28F981B2686BEBDD2D431B05DCDF9CFFD4";
				break;

			case FirmwareType::WonderSwan:
				FileHashes[0] = "BF4480DBEA1C47C8B54CE7BE9382BC1006148F431FBE4277E136351FA74F635E";
				break;

			case FirmwareType::WonderSwanColor:
				FileHashes[0] = "F5A5C044D84CE1681F94E9EF74287CB989784497BE5BD5108DF17908DFA55DB2";
				break;
			
			case FirmwareType::SwanCrystal:
				FileHashes[0] = "82E96ADDF5AB1CE09A84B6EEDAA904E4CA432756851F7E0CC0649006C183834D";
				break;

			case FirmwareType::Ymf288AdpcmRom:
				FileHashes[0] = "53AFD0FA9C62EDA3E2BE939E23F3ADF48A2AF8AD37BB1640261726C5D5ADEBA8";
				break;

			default:
				throw std::runtime_error("Unexpected firmware type");
		}
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