#pragma once 

#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/InputHud.h"
#include "SNES/Input/SnesController.h"
#include "SNES/Input/SnesMouse.h"
#include "NES/Input/NesController.h"
#include "Utilities/Serializer.h"
#include "Utilities/StringUtilities.h"

template<int HubPortCount>
class ControllerHub : public BaseControlDevice
{
protected:
	shared_ptr<BaseControlDevice> _ports[HubPortCount];

	void InternalSetStateFromInput() override
	{
		for(int i = 0; i < HubPortCount; i++) {
			if(_ports[i]) {
				_ports[i]->SetStateFromInput();
			}
		}

		UpdateStateFromPorts();
	}

	void UpdateStateFromPorts()
	{
		for(int i = 0; i < HubPortCount; i++) {
			if(_ports[i]) {
				ControlDeviceState portState = _ports[i]->GetRawState();
				_state.State.push_back((uint8_t)portState.State.size());
				_state.State.insert(_state.State.end(), portState.State.begin(), portState.State.end());
			}
		}
	}

	uint8_t ReadPort(int i)
	{
		return _ports[i]->ReadRam(0x4016);
	}

public:
	ControllerHub(Emulator* emu, ControllerType type, int port, ControllerConfig controllers[]) : BaseControlDevice(emu, type, port)
	{
		for(int i = 0; i < HubPortCount; i++) {
			switch(controllers[i].Type) {
				case ControllerType::FamicomController:
				case ControllerType::NesController:
					_ports[i].reset(new NesController(emu, controllers[i].Type, 0, controllers[i].Keys));
					break;

				case ControllerType::SnesController:
					_ports[i].reset(new SnesController(emu, 0, controllers[i].Keys));
					break;

				case ControllerType::SnesMouse:
					_ports[i].reset(new SnesMouse(emu, 0));
					break;
			}
		}

	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
		for(int i = 0; i < HubPortCount; i++) {
			if(_ports[i]) {
				_ports[i]->WriteRam(addr, value);
			}
		}
	}

	void DrawController(InputHud& hud) override
	{
		for(int i = 0; i < HubPortCount; i++) {
			if(_ports[i]) {
				_ports[i]->DrawController(hud);
			} else {
				hud.EndDrawController();
			}
		}
	}

	void SetTextState(string state) override
	{
		vector<string> portStates = StringUtilities::Split(state, ':');
		int i = 0;
		for(string& portState : portStates) {
			if(_ports[i]) {
				_ports[i]->SetTextState(portState);
			}
			i++;
		}

		UpdateStateFromPorts();
	}

	string GetTextState() override
	{
		auto lock = _stateLock.AcquireSafe();

		string state;
		for(int i = 0; i < HubPortCount; i++) {
			if(i != 0) {
				state += ":";
			}
			if(_ports[i]) {
				state += _ports[i]->GetTextState();
			}
		}

		return state;
	}

	void SetRawState(ControlDeviceState state) override
	{
		auto lock = _stateLock.AcquireSafe();
		_state = state;

		vector<uint8_t> data = state.State;
		int pos = 0;

		for(int i = 0; i < HubPortCount; i++) {
			if(_ports[i]) {
				int length = data[pos++];

				ControlDeviceState portState;
				portState.State.insert(portState.State.begin(), data.begin() + pos, data.begin() + pos + length);
				_ports[i]->SetRawState(portState);
				pos += length;
			}
		}
	}

	bool HasControllerType(ControllerType type) override
	{
		if(_type == type) {
			return true;
		}

		for(int i = 0; i < HubPortCount; i++) {
			if(_ports[i] && _ports[i]->HasControllerType(type)) {
				return true;
			}
		}

		return false;
	}
};
