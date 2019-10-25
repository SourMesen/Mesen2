#pragma once
#include "stdafx.h"
#include "NetMessage.h"
#include "Console.h"
#include "EmuSettings.h"
#include "CheatManager.h"
#include "SaveStateManager.h"

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
	
	SaveStateMessage(shared_ptr<Console> console) : NetMessage(MessageType::SaveState)
	{
		//Used when sending state to clients
		console->Lock();
		_activeCheats = console->GetCheatManager()->GetCheats();
		stringstream state;
		console->Serialize(state);

		EmulationConfig emuCfg = console->GetSettings()->GetEmulationConfig();
		_region = emuCfg.Region;
		_ppuExtraScanlinesAfterNmi = emuCfg.PpuExtraScanlinesAfterNmi;
		_ppuExtraScanlinesBeforeNmi = emuCfg.PpuExtraScanlinesBeforeNmi;
		_gsuClockSpeed = emuCfg.GsuClockSpeed;

		InputConfig inputCfg = console->GetSettings()->GetInputConfig();
		for(int i = 0; i < 5; i++) {
			_controllerTypes[i] = inputCfg.Controllers[i].Type;
		}

		console->Unlock();

		uint32_t dataSize = (uint32_t)state.tellp();
		_stateData.resize(dataSize);
		state.read((char*)_stateData.data(), dataSize);
	}
	
	void LoadState(shared_ptr<Console> console)
	{
		std::stringstream ss;
		ss.write((char*)_stateData.data(), _stateData.size());
		console->Deserialize(ss, SaveStateManager::FileFormatVersion);

		console->GetCheatManager()->SetCheats(_activeCheats);

		EmulationConfig emuCfg = console->GetSettings()->GetEmulationConfig();
		emuCfg.Region = _region;
		emuCfg.PpuExtraScanlinesAfterNmi = _ppuExtraScanlinesAfterNmi;
		emuCfg.PpuExtraScanlinesBeforeNmi = _ppuExtraScanlinesBeforeNmi;
		emuCfg.GsuClockSpeed = _gsuClockSpeed;

		InputConfig inputCfg = console->GetSettings()->GetInputConfig();
		for(int i = 0; i < 5; i++) {
			inputCfg.Controllers[i].Type = _controllerTypes[i];
		}

		console->GetSettings()->SetEmulationConfig(emuCfg);
		console->GetSettings()->SetInputConfig(inputCfg);
	}
};