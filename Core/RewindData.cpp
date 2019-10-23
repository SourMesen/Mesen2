#include "stdafx.h"
#include "RewindData.h"
#include "Console.h"
#include "SaveStateManager.h"
#include "../Utilities/miniz.h"

void RewindData::GetStateData(stringstream &stateData)
{
	stateData.write((char*)SaveStateData.data(), SaveStateData.size());
}

void RewindData::LoadState(shared_ptr<Console> &console)
{
	if(SaveStateData.size() > 0) {
		stringstream stream;
		stream.write((char*)SaveStateData.data(), SaveStateData.size());
		stream.seekg(0, ios::beg);

		console->Deserialize(stream, SaveStateManager::FileFormatVersion);
	}
}

void RewindData::SaveState(shared_ptr<Console> &console)
{
	std::stringstream state;
	console->Serialize(state);

	string data = state.str();
	SaveStateData = vector<uint8_t>(data.c_str(), data.c_str()+data.size());
	FrameCount = 0;
}
