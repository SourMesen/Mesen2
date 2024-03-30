#include "pch.h"
#include "Shared/RewindData.h"
#include "Shared/Emulator.h"
#include "Shared/SaveStateManager.h"
#include "Utilities/CompressionHelper.h"

void RewindData::GetStateData(stringstream &stateData, deque<RewindData>& prevStates, int32_t position)
{
	vector<uint8_t> data;
	CompressionHelper::Decompress(_saveStateData, data);

	if(!IsFullState) {
		position = (position > 0 ? position : (int32_t)prevStates.size()) - 1;
		ProcessXorState(data, prevStates, position);
	}

	stateData.write((char*)data.data(), data.size());
}

template<typename T>
void RewindData::ProcessXorState(T& data, deque<RewindData>& prevStates, int32_t position)
{
	//Find last full state and XOR with it
	while(position >= 0 && position < prevStates.size()) {
		RewindData& prevState = prevStates[position];
		if(prevState.IsFullState) {
			//XOR with previous state to restore state data to its initial state
			if(!prevState._uncompressedData.empty()) {
				for(size_t i = 0, len = std::min(prevState._uncompressedData.size(), data.size()); i < len; i++) {
					data[i] ^= prevState._uncompressedData[i];
				}
			} else {
				vector<uint8_t> prevStateData;
				CompressionHelper::Decompress(prevState._saveStateData, prevStateData);
				for(size_t i = 0, len = std::min(prevStateData.size(), data.size()); i < len; i++) {
					data[i] ^= prevStateData[i];
				}
			}
			break;
		}
		position--;
	}
}

void RewindData::LoadState(Emulator* emu, deque<RewindData>& prevStates, int32_t position, bool sendNotification)
{
	if(_saveStateData.size() == 0) {
		return;
	}
		
	vector<uint8_t> data;
	CompressionHelper::Decompress(_saveStateData, data);

	if(!IsFullState) {
		position = (position > 0 ? position : (int32_t)prevStates.size()) - 1;
		ProcessXorState(data, prevStates, position);
	}

	stringstream stream;
	stream.write((char*)data.data(), data.size());
	stream.seekg(0, ios::beg);

	emu->Deserialize(stream, SaveStateManager::FileFormatVersion, true, std::nullopt, sendNotification);
}

void RewindData::SaveState(Emulator* emu, deque<RewindData>& prevStates, int32_t position)
{
	std::stringstream state;
	emu->Serialize(state, true, 0);

	string data = state.str();

	position = position > 0 ? position : (int32_t)prevStates.size();

	if(position > 0 && (position % 30) != 0) {
		position--;
		ProcessXorState(data, prevStates, position);
	} else {
		IsFullState = true;
		while(position > 0) {
			position--;
			RewindData& prevState = prevStates[position];
			if(prevState.IsFullState) {
				//Get rid of previous full state's uncompressed data once the next full state is added
				prevState._uncompressedData = {};
				break;
			}
		}

		//Keep uncompressed data for the next 30 states - this avoids having to decompress the state 30 times
		_uncompressedData = vector<uint8_t>(data.begin(), data.end());
	}

	CompressionHelper::Compress(data, 1, _saveStateData);
	FrameCount = 0;
}
