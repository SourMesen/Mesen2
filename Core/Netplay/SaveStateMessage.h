#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/CheatManager.h"
#include "Shared/SaveStateManager.h"

class SaveStateMessage : public NetMessage
{
private:
	vector<CheatCode> _activeCheats;
	vector<uint8_t> _stateData;

protected:
	void Serialize(Serializer &s) override
	{
		SVVector(_stateData);
		SVVector(_activeCheats);
	}

public:
	SaveStateMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }
	
	SaveStateMessage(Emulator* emu) : NetMessage(MessageType::SaveState)
	{
		//Used when sending state to clients
		stringstream state;
		{
			auto lock = emu->AcquireLock();
			_activeCheats = emu->GetCheatManager()->GetCheats();
			emu->Serialize(state, true);
		}

		uint32_t dataSize = (uint32_t)state.tellp();
		_stateData.resize(dataSize);
		state.read((char*)_stateData.data(), dataSize);
	}
	
	void LoadState(Emulator* emu)
	{
		std::stringstream ss;
		ss.write((char*)_stateData.data(), _stateData.size());
		emu->Deserialize(ss, SaveStateManager::FileFormatVersion, true);

		emu->GetCheatManager()->SetCheats(_activeCheats);
	}
};