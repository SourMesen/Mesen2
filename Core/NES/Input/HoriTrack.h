#pragma once
#include "stdafx.h"
#include "NES/Input/NesController.h"
#include "Shared/KeyManager.h"

class HoriTrack : public NesController
{
protected:
	bool HasCoordinates() override { return true; }
	
	void InternalSetStateFromInput() override
	{
		NesController::InternalSetStateFromInput();
		SetPressedState(NesController::Buttons::A, KeyManager::IsMouseButtonPressed(MouseButton::LeftButton));
		SetPressedState(NesController::Buttons::B, KeyManager::IsMouseButtonPressed(MouseButton::RightButton));
		SetMovement(KeyManager::GetMouseMovement(
			_emu->GetSettings()->GetVideoConfig().VideoScale,
			_emu->GetSettings()->GetInputConfig().MouseSensitivity + 1
		));
	}

public:
	HoriTrack(Emulator* emu,  KeyMappingSet keyMappings) : NesController(emu, ControllerType::HoriTrack, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;
		if(addr == 0x4016) {
			StrobeProcessRead();
			output = (_stateBuffer & 0x01) << 1;
			_stateBuffer >>= 1;
		}
		return output;
	}
	
	void RefreshStateBuffer() override
	{
		MouseMovement mov = GetMovement();

		mov.dx = std::max(-8, std::min((int)mov.dx, 7));
		mov.dy = std::max(-8, std::min((int)mov.dy, 7));

		mov.dx = ((mov.dx & 0x08) >> 3) | ((mov.dx & 0x04) >> 1) | ((mov.dx & 0x02) << 1) | ((mov.dx & 0x01) << 3);
		mov.dy = ((mov.dy & 0x08) >> 3) | ((mov.dy & 0x04) >> 1) | ((mov.dy & 0x02) << 1) | ((mov.dy & 0x01) << 3);
		
		uint8_t byte1 = (~mov.dy & 0x0F) | ((~mov.dx & 0x0F) << 4);
		uint8_t byte2 = 0x09;

		NesController::RefreshStateBuffer();
		_stateBuffer = (_stateBuffer & 0xFF) | (byte1 << 8) | (byte2 << 16);
	}
};