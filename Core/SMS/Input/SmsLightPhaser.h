#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsVdp.h"
#include "Shared/Emulator.h"
#include "Shared/InputHud.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Utilities/Serializer.h"

class SmsLightPhaser : public BaseControlDevice
{
private:
	SmsConsole* _console;

protected:
	bool HasCoordinates() override { return true; }

	string GetKeyNames() override
	{
		return "F";
	}

	enum Buttons { Fire };

	void InternalSetStateFromInput() override
	{
		MousePosition pos = KeyManager::GetMousePosition();

		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::Fire, KeyManager::IsKeyPressed(keyMapping.CustomKeys[0]));
			if(KeyManager::IsKeyPressed(keyMapping.CustomKeys[1])) {
				pos.X = -1;
				pos.Y = -1;
			}
		}

		SetCoordinates(pos);
	}

	bool IsLightFound(MousePosition pos, bool ignoreVdpPos)
	{
		SmsVdp* vdp = _console->GetVdp();

		int32_t scanline = vdp->GetScanline();
		int32_t cycle = vdp->GetCycle();
		constexpr int radius = 0;

		pos.Y -= vdp->GetViewportYOffset();

		if(pos.X >= 0 && pos.Y >= 0) {
			for(int yOffset = -radius; yOffset <= radius; yOffset++) {
				int yPos = pos.Y + yOffset;
				if(yPos >= 0 && yPos < 240) {
					for(int xOffset = -(radius + 1) * 5; xOffset <= (radius + 1) * 5; xOffset++) {
						int xPos = pos.X + xOffset;
						if(xPos >= 0 && xPos < 256) {
							bool inRange = (
								yPos <= scanline && yPos >= scanline - 5 &&
								xPos <= cycle && xPos >= cycle - 50
							);
							if((ignoreVdpPos || inRange) && vdp->GetPixelBrightness(xPos, yPos) >= 85) {
								//Light cannot be detected if the Y/X position is further ahead than the PPU, or if the PPU drew a dark color
								return true;
							}
						}
					}
				}
			}
		}

		return false;
	}

public:
	SmsLightPhaser(SmsConsole* console, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(console->GetEmulator(), ControllerType::SmsLightPhaser, port, keyMappings)
	{
		_console = console;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t value = 0xFF;
		if(addr == 0) {
			if(_port == 0) {
				value ^= IsPressed(Buttons::Fire) ? 0x10 : 0;
			}
		} else {
			if(_port == 0) {
				value ^= IsLightFound(GetCoordinates(), false) ? 0x40 : 0;
			} else {
				value ^= IsLightFound(GetCoordinates(), false) ? 0x80 : 0;
				value ^= IsPressed(Buttons::Fire) ? 0x04 : 0;
			}
		}
		return value;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	void OnAfterSetState() override
	{
		MousePosition pos = GetCoordinates();

		//Make the VDP latch the H counter at the mouse's position
		if(pos.X >= 0 && pos.Y >= 0 && IsLightFound(pos, true)) {
			_console->GetVdp()->SetLocationLatchRequest(pos.X+38);
		}
	}

	void InternalDrawController(InputHud& hud) override
	{
		hud.DrawOutline(11, 14);
		hud.DrawButton(2, 1, 7, 5, IsPressed(Buttons::Fire));
		hud.DrawNumber(hud.GetControllerIndex() + 1, 4, 7);

		hud.DrawMousePosition(GetCoordinates());
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "x", BaseControlDevice::DeviceXCoordButtonId, true },
			{ "y", BaseControlDevice::DeviceYCoordButtonId, true },
			{ "trigger", Buttons::Fire },
		};
	}
};