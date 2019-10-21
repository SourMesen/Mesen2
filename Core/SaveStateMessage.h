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

	uint8_t* _stateData = nullptr;
	uint32_t _dataSize = 0;
	
	ControllerType _controllerTypes[5];
	ConsoleRegion _region;
	uint32_t _ppuExtraScanlinesAfterNmi;
	uint32_t _ppuExtraScanlinesBeforeNmi;
	uint32_t _gsuClockSpeed;

	CheatCode* _cheats = nullptr;
	uint32_t _cheatArraySize = 0;

protected:
	virtual void ProtectedStreamState()
	{
		StreamArray((void**)&_stateData, _dataSize);
		
		Stream(_region);
		Stream(_ppuExtraScanlinesAfterNmi);
		Stream(_ppuExtraScanlinesBeforeNmi);
		Stream(_gsuClockSpeed);
		StreamArray(_controllerTypes, sizeof(ControllerType) * 5);

		if(_sending) {
			_cheats = _activeCheats.size() > 0 ? &_activeCheats[0] : nullptr;
			_cheatArraySize = (uint32_t)_activeCheats.size() * sizeof(CheatCode);
			StreamArray((void**)&_cheats, _cheatArraySize);
			delete[] _stateData;
		} else {
			StreamArray((void**)&_cheats, _cheatArraySize);
		}
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

		_dataSize = (uint32_t)state.tellp();
		_stateData = new uint8_t[_dataSize];
		state.read((char*)_stateData, _dataSize);
	}
	
	void LoadState(shared_ptr<Console> console)
	{
		std::stringstream ss;
		ss.write((char*)_stateData, _dataSize);
		console->Deserialize(ss, SaveStateManager::FileFormatVersion);

		vector<CheatCode> cheats;
		for(uint32_t i = 0; i < _cheatArraySize / sizeof(CheatCode); i++) {
			cheats.push_back(((CheatCode*)_cheats)[i]);
		}
		console->GetCheatManager()->SetCheats(cheats);

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