#pragma once
#include "stdafx.h"
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
	
	void AddCheat(CheatCode code);

	optional<InternalCheatCode> TryConvertCode(CheatCode code);
	
	optional<InternalCheatCode> ConvertFromSnesGameGenie(string code);
	optional<InternalCheatCode> ConvertFromSnesProActionReplay(string code);
	
	optional<InternalCheatCode> ConvertFromGbGameGenie(string code);
	optional<InternalCheatCode> ConvertFromGbGameShark(string code);
	
	optional<InternalCheatCode> ConvertFromNesGameGenie(string code);
	optional<InternalCheatCode> ConvertFromNesProActionRocky(string code);
	optional<InternalCheatCode> ConvertFromNesCustomCode(string code);

	__forceinline constexpr int GetBankShift(CpuType cpuType)
	{
		switch(cpuType) {
			case CpuType::Snes: return 16;
			case CpuType::Gameboy: return 8;
			case CpuType::Nes: return 8;
			default: throw std::runtime_error("unsupported cpu type");
		}
	}

public:
	CheatManager(Emulator* emu);

	void SetCheats(vector<CheatCode>& codes);
	void SetCheats(CheatCode codes[], uint32_t length);
	void ClearCheats(bool showMessage = true);

	vector<CheatCode> GetCheats();

	bool GetConvertedCheat(CheatCode input, InternalCheatCode& output);
	
	vector<InternalCheatCode>& GetRamRefreshCheats(CpuType cpuType);
	void RefreshRamCheats(CpuType cpuType);

	template<CpuType cpuType>
	__forceinline void ApplyCheat(uint32_t addr, uint8_t &value)
	{
		if(_hasCheats[(int)cpuType] && _bankHasCheats[(int)cpuType][addr >> GetBankShift(cpuType)]) {
			auto result = _cheatsByAddress[(int)cpuType].find(addr);
			if(result != _cheatsByAddress[(int)cpuType].end()) {
				if(result->second.Compare == -1 || result->second.Compare == value) {
					value = result->second.Value;
					_emu->GetConsole()->ProcessCheatCode(result->second, addr, value);
				}
			}
		}
	}
};