#pragma once 

#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/InputHud.h"
#include "Shared/IControllerHub.h"
#include "SNES/Input/SnesController.h"
#include "SNES/Input/SnesMouse.h"
#include "NES/Input/NesController.h"
#include "PCE/Input/PceController.h"
#include "PCE/Input/PceAvenuePad6.h"
#include "Utilities/Serializer.h"
#include "Utilities/StringUtilities.h"

template<int HubPortCount>
class ControllerHub : public BaseControlDevice, public IControllerHub
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
		if(_ports[i]) {
			return _ports[i]->ReadRam(0x4016);
		} else {
			return 0;
		}
	}

	void WritePort(int i, uint8_t value)
	{
		if(_ports[i]) {
			_ports[i]->WriteRam(0x4016, value);
		}
	}

public:
	ControllerHub(Emulator* emu, ControllerType type, int port, ControllerConfig controllers[]) : BaseControlDevice(emu, type, port)
	{
		static_assert(HubPortCount <= MaxSubPorts, "Port count too large");

		for(int i = 0; i < HubPortCount; i++) {
			switch(controllers[i].Type) {
				case ControllerType::FamicomController:
				case ControllerType::FamicomControllerP2:
				case ControllerType::NesController:
					_ports[i].reset(new NesController(emu, controllers[i].Type, 0, controllers[i].Keys));
					break;

				case ControllerType::SnesController:
					_ports[i].reset(new SnesController(emu, 0, controllers[i].Keys));
					break;

				case ControllerType::SnesMouse:
					_ports[i].reset(new SnesMouse(emu, 0, controllers[i].Keys));
					break;

				case ControllerType::PceController:
					_ports[i].reset(new PceController(emu, 0, controllers[i].Keys));
					break;

				case ControllerType::PceAvenuePad6:
					_ports[i].reset(new PceAvenuePad6(emu, 0, controllers[i].Keys));
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
			if(_ports[i] && pos < data.size()) {
				int length = data[pos++];

				if(pos + length > data.size()) {
					break;
				}

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

	void RefreshHubState() override
	{
		//Used when the connected devices are updated by code (e.g by the debugger)
		_state.State.clear();
		UpdateStateFromPorts();
	}

	int GetHubPortCount() override
	{
		return HubPortCount;
	}

	shared_ptr<BaseControlDevice> GetController(int index) override
	{
		if(index >= HubPortCount) {
			return nullptr;
		}

		return _ports[index];
	}
};
