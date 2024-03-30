#pragma once
#include "pch.h"
#include <deque>
#include "Shared/BaseControlDevice.h"

class Emulator;

class RewindData
{
private:
	vector<uint8_t> _saveStateData;
	vector<uint8_t> _uncompressedData;

	template<typename T>
	void ProcessXorState(T& data, deque<RewindData>& prevStates, int32_t position);

public:
	std::deque<ControlDeviceState> InputLogs[BaseControlDevice::PortCount];
	int32_t FrameCount = 0;
	bool EndOfSegment = false;
	bool IsFullState = false;

	void GetStateData(stringstream& stateData, deque<RewindData>& prevStates, int32_t position);
	uint32_t GetStateSize() { return (uint32_t)_saveStateData.size(); }

	void LoadState(Emulator* emu, deque<RewindData>& prevStates, int32_t position = -1, bool sendNotification = true);
	void SaveState(Emulator* emu, deque<RewindData>& prevStates, int32_t position = -1);
};
