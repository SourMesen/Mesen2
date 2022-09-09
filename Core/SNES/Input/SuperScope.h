#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/KeyManager.h"
#include "SNES/SnesPpu.h"
#include "Utilities/Serializer.h"

class SuperScope : public BaseControlDevice
{
private:
	enum Buttons { Fire = 0, Cursor = 1, Turbo = 2, Pause = 3 };
	uint32_t _stateBuffer = 0;
	bool _prevFireButton = false;
	bool _prevTurboButton = false;
	bool _prevPauseButton = false;
	bool _turbo = false;
	SnesPpu *_ppu;

protected:
	bool HasCoordinates() override { return true; }
	
	string GetKeyNames() override
	{
		return "FCTP";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::Fire, KeyManager::IsKeyPressed(keyMapping.CustomKeys[0]));
			SetPressedState(Buttons::Cursor, KeyManager::IsKeyPressed(keyMapping.CustomKeys[1]));
			SetPressedState(Buttons::Turbo, KeyManager::IsKeyPressed(keyMapping.CustomKeys[2]));
			SetPressedState(Buttons::Pause, KeyManager::IsKeyPressed(keyMapping.CustomKeys[3]));
		}

		MousePosition pos = KeyManager::GetMousePosition();
		SetCoordinates(pos);
	}

	void OnAfterSetState() override
	{
		MousePosition pos = GetCoordinates();

		//Make the PPU latch the H/V counters at the mouse's position (offset slightly to make target in the center of the mouse cursor)
		if(pos.X >= 0 && pos.Y >= 0 && (IsPressed(Buttons::Fire) || IsPressed(Buttons::Cursor))) {
			_ppu->SetLocationLatchRequest(pos.X + 10, std::max(0, pos.Y - 3));
		}
	}

	void Serialize(Serializer &s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_stateBuffer); SV(_prevTurboButton); SV(_prevFireButton); SV(_prevPauseButton); SV(_turbo);
	}

	void RefreshStateBuffer() override
	{
		_stateBuffer = (uint32_t)ToByte();
		_prevFireButton = IsPressed(Buttons::Fire);
		_prevTurboButton = IsPressed(Buttons::Turbo);
		_prevPauseButton = IsPressed(Buttons::Pause);
	}

	uint16_t ToByte()
	{
		uint16_t output = 0xFF00; //signature bits

		if(!_prevTurboButton && IsPressed(Buttons::Turbo)) {
			_turbo = !_turbo;
		}

		if((_turbo || !_prevFireButton) && IsPressed(Buttons::Fire)) {
			output |= 0x01;
		}

		if(IsPressed(Buttons::Cursor)) {
			output |= 0x02;
		}

		if(_turbo) {
			output |= 0x04;
		}

		if(!_prevPauseButton && IsPressed(Buttons::Pause)) {
			output |= 0x08;
		}

		if(GetCoordinates().X < 0 || GetCoordinates().Y < 0) {
			output |= 0x40; //offscreen flag
		}

		return output;
	}

public:
	SuperScope(SnesConsole* console, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(console->GetEmulator(), ControllerType::SuperScope, port, keyMappings)
	{
		_ppu = console->GetPpu();
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;

		if(IsCurrentPort(addr)) {
			StrobeProcessRead();

			output = (_stateBuffer & 0x01);

			_stateBuffer >>= 1;
			_stateBuffer |= 0x8000;
		}

		return output;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}

	void InternalDrawController(InputHud& hud) override
	{
		hud.DrawOutline(12, 14);
		hud.DrawButton(1, 1, 3, 5, IsPressed(Buttons::Fire));
		hud.DrawButton(5, 1, 2, 4, IsPressed(Buttons::Turbo));
		hud.DrawButton(8, 1, 3, 5, IsPressed(Buttons::Cursor));
		hud.DrawNumber(hud.GetControllerIndex() + 1, 4, 7);

		hud.DrawMousePosition(GetCoordinates());
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "x", BaseControlDevice::DeviceXCoordButtonId, true },
			{ "y", BaseControlDevice::DeviceYCoordButtonId, true },
			{ "fire", Buttons::Fire },
			{ "turbo", Buttons::Turbo },
			{ "cursor", Buttons::Cursor },
			{ "pause", Buttons::Pause },
		};
	}
};