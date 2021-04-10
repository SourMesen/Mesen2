#pragma once
#include "stdafx.h"
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
	
	ControllerType _controllerTypes[5];
	ConsoleRegion _region;
	uint32_t _ppuExtraScanlinesAfterNmi;
	uint32_t _ppuExtraScanlinesBeforeNmi;
	uint32_t _gsuClockSpeed;

protected:
	void Serialize(Serializer &s) override
	{
		s.StreamVector(_stateData);
		s.Stream(_region, _ppuExtraScanlinesAfterNmi, _ppuExtraScanlinesBeforeNmi, _gsuClockSpeed);
		s.StreamArray(_controllerTypes, 5);
		s.StreamVector(_activeCheats);
	}

public:
	SaveStateMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }
	
	SaveStateMessage(shared_ptr<Emulator> emu) : NetMessage(MessageType::SaveState)
	{
		//Used when sending state to clients
		emu->Lock();
		_activeCheats = emu->GetCheatManager()->GetCheats();
		stringstream state;
		emu->Serialize(state);

		EmulationConfig emuCfg = emu->GetSettings()->GetEmulationConfig();
		_region = emuCfg.Region;
		_ppuExtraScanlinesAfterNmi = emuCfg.PpuExtraScanlinesAfterNmi;
		_ppuExtraScanlinesBeforeNmi = emuCfg.PpuExtraScanlinesBeforeNmi;
		_gsuClockSpeed = emuCfg.GsuClockSpeed;

		InputConfig inputCfg = emu->GetSettings()->GetInputConfig();
		for(int i = 0; i < 5; i++) {
			_controllerTypes[i] = inputCfg.Controllers[i].Type;
		}

		emu->Unlock();

		uint32_t dataSize = (uint32_t)state.tellp();
		_stateData.resize(dataSize);
		state.read((char*)_stateData.data(), dataSize);
	}
	
	void LoadState(shared_ptr<Emulator> emu)
	{
		std::stringstream ss;
		ss.write((char*)_stateData.data(), _stateData.size());
		emu->Deserialize(ss, SaveStateManager::FileFormatVersion);

		emu->GetCheatManager()->SetCheats(_activeCheats);

		EmulationConfig emuCfg = emu->GetSettings()->GetEmulationConfig();
		emuCfg.Region = _region;
		emuCfg.PpuExtraScanlinesAfterNmi = _ppuExtraScanlinesAfterNmi;
		emuCfg.PpuExtraScanlinesBeforeNmi = _ppuExtraScanlinesBeforeNmi;
		emuCfg.GsuClockSpeed = _gsuClockSpeed;

		InputConfig inputCfg = emu->GetSettings()->GetInputConfig();
		for(int i = 0; i < 5; i++) {
			inputCfg.Controllers[i].Type = _controllerTypes[i];
		}

		emu->GetSettings()->SetEmulationConfig(emuCfg);
		emu->GetSettings()->SetInputConfig(inputCfg);
	}
};