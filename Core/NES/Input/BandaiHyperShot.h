#pragma once
#include "pch.h"
#include "NES/Input/NesController.h"
#include "NES/Input/Zapper.h"
#include "NES/NesConsole.h"
#include "Shared/KeyManager.h"

class BandaiHyperShot : public NesController
{
private:
	NesConsole* _console;
	uint32_t _hypershotState = 0;

protected:
	enum ZapperButtons { Fire = 9 };
	
	bool HasCoordinates() override { return true; }

	string GetKeyNames() override
	{
		return NesController::GetKeyNames() + "F";
	}

	void InternalSetStateFromInput() override
	{
		NesController::InternalSetStateFromInput();
		MousePosition pos = KeyManager::GetMousePosition();

		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(ZapperButtons::Fire, KeyManager::IsKeyPressed(keyMapping.CustomKeys[0]));
			if(KeyManager::IsKeyPressed(keyMapping.CustomKeys[1])) {
				pos.X = -1;
				pos.Y = -1;
			}
		}

		SetCoordinates(pos);
	}

	bool IsLightFound()
	{
		return Zapper::StaticIsLightFound(GetCoordinates(), _console);
	}

	void Serialize(Serializer& s) override
	{
		NesController::Serialize(s);
		SV(_hypershotState);
	}

public:
	BandaiHyperShot(NesConsole* console, KeyMappingSet keyMappings) : NesController(console->GetEmulator(), ControllerType::BandaiHyperShot, BaseControlDevice::ExpDevicePort, keyMappings)
	{
		_console = console;
	}

	void RefreshStateBuffer() override
	{
		_hypershotState = (uint32_t)ToByte();
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4016) {
			StrobeProcessRead();
			uint8_t output = (_hypershotState & 0x01) << 1;
			_hypershotState >>= 1;
			return output;
		} else {
			return (IsLightFound() ? 0 : 0x08) | (IsPressed(ZapperButtons::Fire) ? 0x10 : 0x00);
		}
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}
};