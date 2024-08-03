#include "pch.h"
#include <regex>
#include "Shared/CheatManager.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/NotificationManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"

using std::regex;

CheatManager::CheatManager(Emulator* emu)
{
	_emu = emu;
}

optional<InternalCheatCode> CheatManager::TryConvertCode(CheatCode code)
{
	switch(code.Type) {
		case CheatType::NesGameGenie: return ConvertFromNesGameGenie(code.Code);
		case CheatType::NesProActionRocky: return ConvertFromNesProActionRocky(code.Code);
		case CheatType::NesCustom: return ConvertFromNesCustomCode(code.Code);
		case CheatType::SnesProActionReplay: return ConvertFromSnesProActionReplay(code.Code);
		case CheatType::SnesGameGenie: return ConvertFromSnesGameGenie(code.Code);
		case CheatType::GbGameGenie: return ConvertFromGbGameGenie(code.Code);
		case CheatType::GbGameShark: return ConvertFromGbGameShark(code.Code);
		case CheatType::PceRaw: return ConvertFromPceRaw(code.Code);
		case CheatType::PceAddress: return ConvertFromPceAddress(code.Code);
		case CheatType::SmsGameGenie: return ConvertFromSmsGameGenie(code.Code);
		case CheatType::SmsProActionReplay: return ConvertFromSmsProActionReplay(code.Code);

		default: throw std::runtime_error("unsupported cheat type");
	}
}

bool CheatManager::AddCheat(CheatCode code)
{
	optional<InternalCheatCode> convertedCode = TryConvertCode(code);

	if(!convertedCode.has_value()) {
		return false;
	}

	_cheats.push_back(code);

	int cpuIndex = (int)convertedCode->Cpu;
	if(convertedCode->IsRamCode) {
		_ramRefreshCheats[cpuIndex].push_back(convertedCode.value());
	} else {
		_cheatsByAddress[cpuIndex].emplace(convertedCode->Address, convertedCode.value());
		_hasCheats[cpuIndex] = true;
		_bankHasCheats[cpuIndex][convertedCode->Address >> GetBankShift(convertedCode->Cpu)] = true;
	}

	return true;
}

void CheatManager::SetCheats(vector<CheatCode>& codes)
{
	auto lock = _emu->AcquireLock();

	bool hasCheats = !_cheats.empty();
	ClearCheats(false);
	for(CheatCode &code : codes) {
		if(!AddCheat(code)) {
			MessageManager::DisplayMessage("Cheats", "Invalid cheat: " + string(code.Code));
		}
	}

	if(codes.size() > 1) {
		MessageManager::DisplayMessage("Cheats", "CheatsApplied", std::to_string(codes.size()));
	} else if(codes.size() == 1) {
		MessageManager::DisplayMessage("Cheats", "CheatApplied");
	} else if(hasCheats) {
		MessageManager::DisplayMessage("Cheats", "CheatsDisabled");
	}

	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::CheatsChanged);
}

void CheatManager::SetCheats(CheatCode codes[], uint32_t length)
{
	vector<CheatCode> cheats(codes, codes+length);
	SetCheats(cheats);
}

void CheatManager::InternalClearCheats()
{
	_cheats.clear();
	for(int i = 0; i < CpuTypeUtilities::GetCpuTypeCount(); i++) {
		_cheatsByAddress[i].clear();
		_ramRefreshCheats[i].clear();
	}
	memset(_hasCheats, 0, sizeof(_hasCheats));
	memset(_bankHasCheats, 0, sizeof(_bankHasCheats));
}

void CheatManager::ClearCheats(bool showMessage)
{
	auto lock = _emu->AcquireLock();

	bool hadCheats = !_cheats.empty();
	InternalClearCheats();

	if(showMessage && hadCheats) {
		MessageManager::DisplayMessage("Cheats", "CheatsDisabled");

		//Used by net play
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::CheatsChanged);
	}
}

optional<InternalCheatCode> CheatManager::ConvertFromNesGameGenie(string code)
{
	static regex _validator = regex("^([APZLGITYEOXUKSVN]{6})|([APZLGITYEOXUKSVN]{8})$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	static string ggLetters = "APZLGITYEOXUKSVN";

	auto decodeValue = [](int code, int bitIndexes[], int count) -> int {
		int result = 0;
		for(int i = 0; i < count; i++) {
			result <<= 1;
			result |= (code >> bitIndexes[i]) & 0x01;
		}
		return result;
	};

	int addressBits[15] = { 14, 13, 12, 19, 22, 21, 20, 7, 10, 9, 8, 15, 18, 17, 16 };
	int valueBits[8] = { 3, 6, 5, 4, 23, 2, 1, 0 };
	int rawCode = 0;
	for(int i = 0, len = (int)code.size(); i < len; i++) {
		rawCode |= ggLetters.find(std::toupper(code[i])) << (i * 4);
	}

	int compareValue = -1;
	if(code.size() == 8) {
		//Bit 5 of the value is stored in a different location for 8-character codes
		valueBits[4] = 31;

		int compareValueBits[8] = { 27, 30, 29, 28, 23, 26, 25, 24 };
		compareValue = decodeValue(rawCode, compareValueBits, 8);
	}

	int address = decodeValue(rawCode, addressBits, 15) + 0x8000;
	int value = decodeValue(rawCode, valueBits, 8);

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::NesGameGenie;
	cheat.Cpu = CpuType::Nes;
	cheat.Address = address;
	cheat.Value = value;
	cheat.Compare = compareValue;
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromNesProActionRocky(string code)
{
	static regex _validator = regex("^[a-f0-9]{8}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	int shiftValues[31] = {
		3, 13, 14, 1, 6, 9, 5, 0, 12, 7, 2, 8, 10, 11, 4,	//address
		19, 21, 23, 22, 20, 17, 16, 18,							//compare
		29, 31, 24, 26, 25, 30, 27, 28							//value
	};

	uint32_t key = 0x7E5EE93A;
	uint32_t xorValue = 0x5C184B91;

	uint32_t parCode = HexUtilities::FromHex(code);

	//Throw away bit 0, not used.
	parCode >>= 1;

	uint32_t result = 0;
	for(int i = 30; i >= 0; i--) {
		if((((key ^ parCode) >> 30) & 0x01) != 0) {
			result |= (uint32_t)(0x01 << shiftValues[i]);
			key ^= xorValue;
		}
		parCode <<= 1;
		key <<= 1;
	}

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::NesCustom;
	cheat.Cpu = CpuType::Nes;
	cheat.Address = (result & 0x7FFF) + 0x8000;
	cheat.Value = (result >> 24) & 0xFF;
	cheat.Compare = (result >> 16) & 0xFF;
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromNesCustomCode(string code)
{
	static regex _validator = regex("^[a-f0-9]{4}:[a-f0-9]{2}(:[a-f0-9]{2}){0,1}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	vector<string> parts = StringUtilities::Split(code, ':');
	uint32_t address = HexUtilities::FromHex(parts[0]);
	uint32_t value = HexUtilities::FromHex(parts[1]);
	int32_t compare = parts.size() == 3 ? HexUtilities::FromHex(parts[2]) : -1;

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::NesCustom;
	cheat.Cpu = CpuType::Nes;
	cheat.Address = address;
	cheat.Value = value;
	cheat.Compare = compare;
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromGbGameGenie(string code)
{
	static regex _validator = regex("^[a-f0-9]{3}-[a-f0-9]{3}(-[a-f0-9]{3}){0,1}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	uint8_t value = (uint8_t)HexUtilities::FromHex(code.substr(0, 2));

	int16_t compare = -1;
	if(code.length() > 7) {
		compare = (uint8_t)HexUtilities::FromHex(code.substr(8, 1) + code[10]);
		compare = (uint8_t)(((compare >> 2) | ((compare & 0x03) << 6)) ^ 0xBA);
	}

	uint16_t address = (uint16_t)(HexUtilities::FromHex(code.substr(6, 1) + code[2] + code[4] + code[5]) ^ 0xF000);

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::GbGameGenie;
	cheat.Cpu = CpuType::Gameboy;
	cheat.Address = address;
	cheat.Value = value;
	cheat.Compare = compare;
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromGbGameShark(string code)
{
	static regex _validator = regex("^[a-f0-9]{8}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	uint32_t codeValue = HexUtilities::FromHex(code);

	uint8_t codeType = codeValue >> 24;
	uint8_t value = (uint8_t)HexUtilities::FromHex(code.substr(2, 2));
	uint16_t address = (uint16_t)HexUtilities::FromHex(code.substr(6, 2) + code.substr(4, 2));

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::GbGameShark;
	cheat.Cpu = CpuType::Gameboy;
	cheat.Value = value;
	cheat.IsRamCode = true;

	if(codeType >= 0x80) {
		uint32_t bank = (codeType & 0x0F);
		uint32_t absAddress;

		if(address >= 0xA000 && address < 0xC000) {
			cheat.MemType = MemoryType::GbCartRam;
			absAddress = (bank * 0x2000) + address - 0xA000;
		} else if(address >= 0xD000 && address < 0xE000) {
			cheat.MemType = MemoryType::GbWorkRam;
			absAddress = (bank * 0x1000) + address - 0xD000;
		} else if(address >= 0xC000 && address < 0xD000) {
			cheat.MemType = MemoryType::GbWorkRam;
			bank = 0;
			absAddress = address - 0xC000;
		} else {
			//Invalid code?
			return std::nullopt;
		}

		cheat.IsAbsoluteAddress = true;
		cheat.Address = absAddress;
	} else if(codeType == 0x01) {
		cheat.Address = address;
	} else {
		//Invalid code?
		return std::nullopt;
	}

	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromSnesProActionReplay(string code)
{
	static regex _validator = regex("^[a-f0-9]{8}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	uint32_t codeValue = HexUtilities::FromHex(code);

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::SnesProActionReplay;
	cheat.Cpu = CpuType::Snes;
	cheat.Address = (codeValue >> 8) & 0xFFFFFF;
	cheat.Value = (uint8_t)(codeValue & 0xFF);

	if(cheat.Address >= 0x7E0000 && cheat.Address <= 0x7FFFFF) {
		cheat.IsRamCode = true;
		cheat.IsAbsoluteAddress = true;
		cheat.MemType = MemoryType::SnesWorkRam;
		cheat.Address -= 0x7E0000;
	}

	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromSnesGameGenie(string code)
{
	static regex _validator = regex("^[a-f0-9]{4}-[a-f0-9]{4}$", std::regex_constants::icase);
	static string _convertTable = "DF4709156BC8A23E";
	
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	uint32_t rawValue = 0;
	for(int i = 0; i < (int)code.size(); i++) {
		if(code[i] != '-') {
			rawValue <<= 4;
			rawValue |= (uint32_t)_convertTable.find_first_of(std::toupper(code[i]));
		}
	}

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::SnesGameGenie;
	cheat.Cpu = CpuType::Snes;
	cheat.Address = (
		((rawValue & 0x3C00) << 10) |
		((rawValue & 0x3C) << 14) |
		((rawValue & 0xF00000) >> 8) |
		((rawValue & 0x03) << 10) |
		((rawValue & 0xC000) >> 6) |
		((rawValue & 0xF0000) >> 12) |
		((rawValue & 0x3C0) >> 6)
	);

	cheat.Value = rawValue >> 24;

	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromPceRaw(string code)
{
	static regex _validator = regex("^[a-f0-9]{6}:[a-f0-9]{2}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::PceRaw;
	cheat.Cpu = CpuType::Pce;
	cheat.Value = (uint8_t)HexUtilities::FromHex(code.substr(7, 2));
	cheat.Address = HexUtilities::FromHex(code.substr(0, 6));
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromPceAddress(string code)
{
	static regex _validator = regex("^[a-f0-9]{6}:[a-f0-9]{2}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::PceAddress;
	cheat.Cpu = CpuType::Pce;
	cheat.Value = (uint8_t)HexUtilities::FromHex(code.substr(7, 2));
	uint32_t address = HexUtilities::FromHex(code.substr(0, 6));
	cheat.Address = ((address & 0xFF0000) >> 3) | (address & 0x1FFF);
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromSmsGameGenie(string code)
{
	static regex _validator = regex("^[a-f0-9]{3}-[a-f0-9]{3}(-[a-f0-9]{3}){0,1}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	uint8_t value = (uint8_t)HexUtilities::FromHex(code.substr(0, 2));

	int16_t compare = -1;
	if(code.length() > 7) {
		compare = (uint8_t)HexUtilities::FromHex(code.substr(8, 1) + code[10]);
		compare = (uint8_t)(((compare >> 2) | ((compare & 0x03) << 6)) ^ 0xBA);
	}

	uint16_t address = (uint16_t)(HexUtilities::FromHex(code.substr(6, 1) + code[2] + code[4] + code[5]) ^ 0xF000);

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::SmsGameGenie;
	cheat.Cpu = CpuType::Sms;
	cheat.Address = address;
	cheat.Value = value;
	cheat.Compare = compare;
	return cheat;
}

optional<InternalCheatCode> CheatManager::ConvertFromSmsProActionReplay(string code)
{
	static regex _validator = regex("^[a-f0-9]{8}$", std::regex_constants::icase);
	if(!std::regex_match(code, _validator)) {
		return std::nullopt;
	}

	uint32_t codeValue = HexUtilities::FromHex(code);

	InternalCheatCode cheat = {};
	cheat.Type = CheatType::SmsProActionReplay;
	cheat.Cpu = CpuType::Sms;
	cheat.Address = (codeValue >> 8) & 0xFFFF;
	cheat.Value = (uint8_t)(codeValue & 0xFF);
	cheat.IsRamCode = true;
	cheat.IsAbsoluteAddress = false;
	return cheat;
}

vector<CheatCode> CheatManager::GetCheats()
{
	return _cheats;
}

vector<InternalCheatCode>& CheatManager::GetRamRefreshCheats(CpuType cpuType)
{
	return _ramRefreshCheats[(int)cpuType];
}

bool CheatManager::GetConvertedCheat(CheatCode input, InternalCheatCode& output)
{
	optional<InternalCheatCode> convertedCode = TryConvertCode(input);
	if(convertedCode.has_value()) {
		output = convertedCode.value();
		return true;
	}

	return false;
}

void CheatManager::RefreshRamCheats(CpuType cpuType)
{
	for(InternalCheatCode& code : _ramRefreshCheats[(int)cpuType]) {
		if(code.IsAbsoluteAddress) {
			ConsoleMemoryInfo mem = _emu->GetMemory(code.MemType);
			if(code.Address < mem.Size) {
				((uint8_t*)mem.Memory)[code.Address] = code.Value;
			}
		}
	}
}

template<CpuType cpuType>
void CheatManager::ApplyCheat(uint32_t addr, uint8_t& value)
{
	if(_bankHasCheats[(int)cpuType][addr >> GetBankShift(cpuType)]) {
		auto result = _cheatsByAddress[(int)cpuType].find(addr);
		if(result != _cheatsByAddress[(int)cpuType].end()) {
			if(result->second.Compare == -1 || result->second.Compare == value) {
				value = result->second.Value;
				_emu->GetConsoleUnsafe()->ProcessCheatCode(result->second, addr, value);
			}
		}
	}
}

template void CheatManager::ApplyCheat<CpuType::Nes>(uint32_t addr, uint8_t& value);
template void CheatManager::ApplyCheat<CpuType::Snes>(uint32_t addr, uint8_t& value);
template void CheatManager::ApplyCheat<CpuType::Pce>(uint32_t addr, uint8_t& value);
template void CheatManager::ApplyCheat<CpuType::Gameboy>(uint32_t addr, uint8_t& value);
template void CheatManager::ApplyCheat<CpuType::Sms>(uint32_t addr, uint8_t& value);
