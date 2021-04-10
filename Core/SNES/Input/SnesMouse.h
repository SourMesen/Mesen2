#pragma once
#include "stdafx.h"
#include "SNES/Console.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/KeyManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"

class SnesMouse : public BaseControlDevice
{
private:
	uint32_t _stateBuffer = 0;
	uint8_t _sensitivity = 0;
	EmuSettings* _settings = nullptr;

protected:
	bool HasCoordinates() override { return true; }

	void Serialize(Serializer &s) override
	{
		BaseControlDevice::Serialize(s);
		s.Stream(_stateBuffer, _sensitivity);
	}

	string GetKeyNames() override
	{
		return "LR";
	}

	void InternalSetStateFromInput() override
	{
		SetPressedState(Buttons::Left, KeyManager::IsMouseButtonPressed(MouseButton::LeftButton));
		SetPressedState(Buttons::Right, KeyManager::IsMouseButtonPressed(MouseButton::RightButton));
		SetMovement(KeyManager::GetMouseMovement(
			_settings->GetVideoConfig().VideoScale,
			_settings->GetInputConfig().MouseSensitivity + 1
		));
	}

public:
	enum Buttons { Left = 0, Right };

	SnesMouse(Console* console, uint8_t port) : BaseControlDevice(console->GetEmulator(), port)
	{
		_settings = _emu->GetSettings().get();
	}

	ControllerType GetControllerType() override
	{
		return ControllerType::SnesMouse;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;
		if((addr == 0x4016 && (_port & 0x01) == 0) || (addr == 0x4017 && (_port & 0x01) == 1)) {
			StrobeProcessRead();
			if(_strobe) {
				_sensitivity = (_sensitivity + 1) % 3;
			}

			output = (_stateBuffer & 0x80000000) >> 31;
			if(_port >= 2) {
				output <<= 1;
			}
			_stateBuffer <<= 1;
		}
		return output;
	}

	void RefreshStateBuffer() override
	{
		MouseMovement mov = GetMovement();
		int32_t dx = mov.dx * (1 + _sensitivity);
		int32_t dy = mov.dy * (1 + _sensitivity);

		uint32_t upFlag = dy < 0 ? 0x80 : 0;
		uint32_t leftFlag = dx < 0 ? 0x80 : 0;

		dx = std::min(std::abs(dx), 127);
		dy = std::min(std::abs(dy), 127);

		uint8_t byte1 = 0;
		uint8_t byte2 = 0x01 | ((_sensitivity & 0x03) << 4) | (IsPressed(SnesMouse::Buttons::Left) ? 0x40 : 0) | (IsPressed(SnesMouse::Buttons::Right) ? 0x80 : 0);
		uint8_t byte3 = dy | upFlag;
		uint8_t byte4 = dx | leftFlag;

		_stateBuffer = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
	}
};