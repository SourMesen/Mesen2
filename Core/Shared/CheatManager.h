#pragma once
#include "pch.h"
#include <optional>
#include "Shared/CpuType.h"

using std::optional;

class Emulator;
enum class MemoryType;

enum class CheatType : uint8_t
{
	NesGameGenie = 0,
	NesProActionRocky,
	NesCustom,
	GbGameGenie,
	GbGameShark,
	SnesGameGenie,
	SnesProActionReplay,
	PceRaw,
	PceAddress,
	SmsProActionReplay,
	SmsGameGenie
};

struct InternalCheatCode
{
	MemoryType MemType = {};
	uint32_t Address = 0;
	int16_t Compare = -1;
	uint8_t Value = 0;
	CheatType Type = {};
	CpuType Cpu = {};
	bool IsRamCode = false;
	bool IsAbsoluteAddress = false;
};

struct CheatCode
{
	CheatType Type;
	char Code[16];
};

class CheatManager
{
private:
	Emulator* _emu;
	bool _hasCheats[CpuTypeUtilities::GetCpuTypeCount()] = {};
	bool _bankHasCheats[CpuTypeUtilities::GetCpuTypeCount()][0x100] = {};
	
	vector<CheatCode> _cheats;

	vector<InternalCheatCode> _ramRefreshCheats[CpuTypeUtilities::GetCpuTypeCount()];
	unordered_map<uint32_t, InternalCheatCode> _cheatsByAddress[CpuTypeUtilities::GetCpuTypeCount()];
	
	optional<InternalCheatCode> TryConvertCode(CheatCode code);
	
	optional<InternalCheatCode> ConvertFromSnesGameGenie(string code);
	optional<InternalCheatCode> ConvertFromSnesProActionReplay(string code);
	
	optional<InternalCheatCode> ConvertFromGbGameGenie(string code);
	optional<InternalCheatCode> ConvertFromGbGameShark(string code);

	optional<InternalCheatCode> ConvertFromPceRaw(string code);
	optional<InternalCheatCode> ConvertFromPceAddress(string code);
	
	optional<InternalCheatCode> ConvertFromNesGameGenie(string code);
	optional<InternalCheatCode> ConvertFromNesProActionRocky(string code);
	optional<InternalCheatCode> ConvertFromNesCustomCode(string code);

	optional<InternalCheatCode> ConvertFromSmsGameGenie(string code);
	optional<InternalCheatCode> ConvertFromSmsProActionReplay(string code);

	__forceinline constexpr int GetBankShift(CpuType cpuType)
	{
		switch(cpuType) {
			case CpuType::Snes: return 16;
			case CpuType::Gameboy: return 8;
			case CpuType::Nes: return 8;
			case CpuType::Pce: return 13;
			case CpuType::Sms: return 8;
			default: throw std::runtime_error("unsupported cpu type");
		}
	}

public:
	CheatManager(Emulator* emu);

	bool AddCheat(CheatCode code);
	void InternalClearCheats();

	void SetCheats(vector<CheatCode>& codes);
	void SetCheats(CheatCode codes[], uint32_t length);
	void ClearCheats(bool showMessage = true);

	vector<CheatCode> GetCheats();

	bool GetConvertedCheat(CheatCode input, InternalCheatCode& output);
	
	vector<InternalCheatCode>& GetRamRefreshCheats(CpuType cpuType);
	void RefreshRamCheats(CpuType cpuType);

	template<CpuType cpuType>
	__forceinline bool HasCheats()
	{
		return _hasCheats[(int)cpuType];
	}

	template<CpuType cpuType>
	__noinline void ApplyCheat(uint32_t addr, uint8_t& value);
};