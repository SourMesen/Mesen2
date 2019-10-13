#include "stdafx.h"
#include "CheatManager.h"
#include "MessageManager.h"
#include "Console.h"

CheatManager::CheatManager(Console* console)
{
	_console = console;
}

void CheatManager::AddCheat(CheatCode code)
{
	_cheats.push_back(code);
	_hasCheats = true;
	_bankHasCheats[code.Address >> 16] = true;
}

void CheatManager::SetCheats(vector<CheatCode> codes)
{
	auto lock = _console->AcquireLock();

	bool hasCheats = !_cheats.empty();
	ClearCheats(false);
	for(CheatCode &code : codes) {
		AddCheat(code);
	}

	if(codes.size() > 1) {
		MessageManager::DisplayMessage("Cheats", "CheatsApplied", std::to_string(codes.size()));
	} else if(codes.size() == 1) {
		MessageManager::DisplayMessage("Cheats", "CheatApplied");
	} else if(hasCheats) {
		MessageManager::DisplayMessage("Cheats", "CheatsDisabled");
	}
}

void CheatManager::SetCheats(uint32_t codes[], uint32_t length)
{
	vector<CheatCode> cheats;
	cheats.reserve(length);
	for(uint32_t i = 0; i < length; i++) {
		cheats.push_back({ codes[i] >> 8, codes[i] & 0xFF });
	}
	SetCheats(cheats);
}

void CheatManager::ClearCheats(bool showMessage)
{
	auto lock = _console->AcquireLock();

	if(showMessage && !_cheats.empty()) {
		MessageManager::DisplayMessage("Cheats", "CheatsDisabled");
	}

	_cheats.clear();
	_hasCheats = false;
	memset(_bankHasCheats, 0, sizeof(_bankHasCheats));
}

vector<CheatCode> CheatManager::GetCheats()
{
	return _cheats;
}