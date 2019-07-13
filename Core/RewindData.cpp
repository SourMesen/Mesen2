#include "stdafx.h"
#include "RewindData.h"
#include "Console.h"
#include "SaveStateManager.h"
#include "../Utilities/miniz.h"

void RewindData::GetStateData(stringstream &stateData)
{
	unsigned long length = OriginalSaveStateSize;
	uint8_t* buffer = new uint8_t[length];
	uncompress(buffer, &length, SaveStateData.data(), (unsigned long)SaveStateData.size());
	stateData.write((char*)buffer, length);
	delete[] buffer;
}

void RewindData::LoadState(shared_ptr<Console> &console)
{
	if(SaveStateData.size() > 0 && OriginalSaveStateSize > 0) {
		unsigned long length = OriginalSaveStateSize;
		uint8_t* buffer = new uint8_t[length];
		uncompress(buffer, &length, SaveStateData.data(), (unsigned long)SaveStateData.size());

		stringstream stream;
		stream.write((char*)buffer, length);
		stream.seekg(0, ios::beg);

		console->Deserialize(stream, SaveStateManager::FileFormatVersion);
		delete[] buffer;
	}
}

void RewindData::CompressState(string stateData, vector<uint8_t>& compressedState)
{
	unsigned long compressedSize = compressBound((unsigned long)stateData.size());
	uint8_t* compressedData = new uint8_t[compressedSize];
	compress2(compressedData, &compressedSize, (unsigned char*)stateData.c_str(), (unsigned long)stateData.size(), MZ_BEST_SPEED);
	compressedState = vector<uint8_t>(compressedData, compressedData + compressedSize);
	delete[] compressedData;
}

void RewindData::SaveState(shared_ptr<Console> &console)
{
	std::stringstream state;
	console->Serialize(state);

	string stateData = state.str();
	vector<uint8_t> compressedState;
	CompressState(stateData, compressedState);
	SaveStateData = compressedState;
	OriginalSaveStateSize = (uint32_t)stateData.size();
	FrameCount = 0;
}
