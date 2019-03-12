#pragma once
#include "stdafx.h"

class Console;

class SaveStateManager
{
private:
	static constexpr uint32_t MaxIndex = 10;

	atomic<uint32_t> _lastIndex;
	shared_ptr<Console> _console;

	string GetStateFilepath(int stateIndex);	

public:
	static constexpr uint32_t FileFormatVersion = 1;

	SaveStateManager(shared_ptr<Console> console);

	uint64_t GetStateInfo(int stateIndex);

	void SaveState();
	bool LoadState();

	void GetSaveStateHeader(ostream & stream);

	void SaveState(ostream &stream);
	bool SaveState(string filepath);
	void SaveState(int stateIndex, bool displayMessage = true);
	bool LoadState(istream &stream, bool hashCheckRequired = true);
	bool LoadState(string filepath, bool hashCheckRequired = true);
	bool LoadState(int stateIndex);

	//void SaveRecentGame(string romName, string romPath, string patchPath);
	//void LoadRecentGame(string filename, bool resetGame);

	void MoveToNextSlot();
	void MoveToPreviousSlot();
};