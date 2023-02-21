#pragma once
#include "pch.h"

class Emulator;
struct RenderedFrame;

class SaveStateManager
{
private:
	static constexpr uint32_t MaxIndex = 10;

	atomic<uint32_t> _lastIndex;
	Emulator* _emu;

	string GetStateFilepath(int stateIndex);
	void SaveVideoData(ostream& stream);
	bool GetVideoData(vector<uint8_t>& out, RenderedFrame& frame, istream& stream);

	void WriteValue(ostream& stream, uint32_t value);
	uint32_t ReadValue(istream& stream);

public:
	static constexpr uint32_t FileFormatVersion = 4;
	static constexpr uint32_t MinimumSupportedVersion = 3;
	static constexpr uint32_t AutoSaveStateIndex = 11;

	SaveStateManager(Emulator* emu);

	void SaveState();
	bool LoadState();

	void GetSaveStateHeader(ostream & stream);

	void SaveState(ostream &stream);
	bool SaveState(string filepath, bool showSuccessMessage = true);
	void SaveState(int stateIndex, bool displayMessage = true);
	bool LoadState(istream &stream);
	bool LoadState(string filepath, bool showSuccessMessage = true);
	bool LoadState(int stateIndex);

	void SaveRecentGame(string romName, string romPath, string patchPath);
	void LoadRecentGame(string filename, bool resetGame);

	int32_t GetSaveStatePreview(string saveStatePath, uint8_t* pngData);

	void SelectSaveSlot(int slotIndex);
	void MoveToNextSlot();
	void MoveToPreviousSlot();
};