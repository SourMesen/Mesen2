#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

class InternalRegisters;
class SnesController;
class SnesConsole;
class InputHud;

class Multitap : public BaseControlDevice
{
private:
	enum Buttons { A = 0, B, X, Y, L, R, Select, Start, Up, Down, Left, Right };
	static constexpr int ButtonCount = 12;

	vector<KeyMapping> _mappings[4];
	uint8_t _turboSpeed[4] = {};
	uint16_t _stateBuffer[4] = {};
	InternalRegisters *_internalRegs = nullptr;

	void DrawController(InputHud& hud, int port);

protected:
	string GetKeyNames() override;
	void InternalSetStateFromInput() override;
	void UpdateControllerState(uint8_t controllerNumber, SnesController &controller);
	uint16_t ToByte(uint8_t port);
	void Serialize(Serializer &s) override;
	void RefreshStateBuffer() override;

public:
	Multitap(SnesConsole* console, uint8_t port, KeyMappingSet keyMappings1, KeyMappingSet keyMappings2, KeyMappingSet keyMappings3, KeyMappingSet keyMappings4);

	void SetControllerState(uint8_t controllerNumber, ControlDeviceState state);

	uint8_t ReadRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;

	void DrawController(InputHud& hud) override;
};