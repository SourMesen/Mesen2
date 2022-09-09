#include "pch.h"
#include "RewindData.h"
#include "Emulator.h"
#include "SaveStateManager.h"
#include "Utilities/miniz.h"

void RewindData::GetStateData(stringstream &stateData)
{
	stateData.write((char*)SaveStateData.data(), SaveStateData.size());
}

void RewindData::LoadState(Emulator* emu)
{
	if(SaveStateData.size() > 0) {
		stringstream stream;
		stream.write((char*)SaveStateData.data(), SaveStateData.size());
		stream.seekg(0, ios::beg);

		emu->Deserialize(stream, SaveStateManager::FileFormatVersion, true);
	}
}

void RewindData::SaveState(Emulator* emu)
{
	std::stringstream state;
	emu->Serialize(state, true);

	string data = state.str();
	SaveStateData = vector<uint8_t>(data.c_str(), data.c_str()+data.size());
	FrameCount = 0;
}
