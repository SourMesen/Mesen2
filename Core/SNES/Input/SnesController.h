#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

class SnesController : public BaseControlDevice
{
private:
	uint32_t _stateBuffer = 0;
	uint8_t _turboSpeed = 0;

protected:
	string GetKeyNames() override;
	void InternalSetStateFromInput() override;
	uint16_t ToByte();
	void Serialize(Serializer &s) override;
	void RefreshStateBuffer() override;

public:
	enum Buttons { A = 0, B, X, Y, L, R, Select, Start, Up, Down, Left, Right };

	SnesController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings);

	uint8_t ReadRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;

	void DrawController(InputHud& hud) override;
};