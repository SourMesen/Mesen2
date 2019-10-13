#include "stdafx.h"
#include "CheatManager.h"
#include "MessageManager.h"
#include "Console.h"

CheatManager::CheatManager(Console* console)
{
	_console = console;
}

void CheatManager::AddCheat(uint32_t addr, uint8_t value)
{
	_cheats.push_back({ addr, value });
	_hasCheats = true;
	_bankHasCheats[addr >> 16] = true;
}

void CheatManager::SetCheats(uint32_t codes[], uint32_t length)
{
	auto lock = _console->AcquireLock();

	bool hasCheats = !_cheats.empty();
	ClearCheats(false);
	for(uint32_t i = 0; i < length; i++) {
		AddCheat(codes[i] >> 8, codes[i] & 0xFF);
	}

	if(length > 1) {
		MessageManager::DisplayMessage("Cheats", "CheatsApplied", std::to_string(length));
	} else if(length == 1) {
		MessageManager::DisplayMessage("Cheats", "CheatApplied");
	} else if(hasCheats) {
		MessageManager::DisplayMessage("Cheats", "CheatsDisabled");
	}
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
