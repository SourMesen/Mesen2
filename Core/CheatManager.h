#pragma once
#include "stdafx.h"

class Console;

struct CheatCode
{
	uint32_t Address;
	uint8_t Value;
};

class CheatManager
{
private:
	Console* _console;
	bool _hasCheats = false;
	bool _bankHasCheats[0x100] = {};
	vector<CheatCode> _cheats;

	void AddCheat(CheatCode code);

public:
	CheatManager(Console* console);

	void SetCheats(vector<CheatCode> codes);
	void SetCheats(uint32_t codes[], uint32_t length);
	void ClearCheats(bool showMessage = true);

	vector<CheatCode> GetCheats();

	__forceinline void ApplyCheat(uint32_t addr, uint8_t &value);
};

__forceinline void CheatManager::ApplyCheat(uint32_t addr, uint8_t &value)
{
	if(_hasCheats && _bankHasCheats[addr >> 16]) {
		for(CheatCode &cheat : _cheats) {
			if(cheat.Address == addr) {
				value = cheat.Value;
				break;
			}
		}
	}
}
