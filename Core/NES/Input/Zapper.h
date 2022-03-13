#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "NES/NesConstants.h"
#include "NES/BaseNesPpu.h"
#include "NES/NesConsole.h"
#include "Shared/Emulator.h"
#include "Shared/InputHud.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Utilities/Serializer.h"

class Zapper : public BaseControlDevice
{
private:
	NesConsole* _console;

protected:
	bool HasCoordinates() override { return true; }

	string GetKeyNames() override
	{
		return "F";
	}

	enum Buttons { Fire };

	void InternalSetStateFromInput() override
	{
		SetPressedState(Buttons::Fire, KeyManager::IsMouseButtonPressed(MouseButton::LeftButton));

		MousePosition pos = KeyManager::GetMousePosition();
		if(KeyManager::IsMouseButtonPressed(MouseButton::RightButton)) {
			pos.X = -1;
			pos.Y = -1;
		}
		SetCoordinates(pos);
	}

	bool IsLightFound()
	{
		return StaticIsLightFound(GetCoordinates(), _console);
	}

public:
	Zapper(NesConsole* console, ControllerType type, uint8_t port) : BaseControlDevice(console->GetEmulator(), type, port)
	{
		_console = console;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;
		if((IsExpansionDevice() && addr == 0x4017) || IsCurrentPort(addr)) {
			output = (IsLightFound() ? 0 : 0x08) | (IsPressed(Zapper::Buttons::Fire) ? 0x10 : 0x00);
		}
		return output;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	static bool StaticIsLightFound(MousePosition pos, NesConsole* console)
	{
		BaseNesPpu* ppu = console ? console->GetPpu() : nullptr;
		if(!ppu) {
			return false;
		}

		int32_t scanline = ppu->GetCurrentScanline();
		int32_t cycle = ppu->GetCurrentCycle();
		int radius = (int)console->GetNesConfig().LightDetectionRadius;

		if(pos.X >= 0 && pos.Y >= 0) {
			for(int yOffset = -radius; yOffset <= radius; yOffset++) {
				int yPos = pos.Y + yOffset;
				if(yPos >= 0 && yPos < NesConstants::ScreenHeight) {
					for(int xOffset = -radius; xOffset <= radius; xOffset++) {
						int xPos = pos.X + xOffset;
						if(xPos >= 0 && xPos < NesConstants::ScreenWidth) {
							if(scanline >= yPos && (scanline - yPos <= 20) && (scanline != yPos || cycle > xPos) && ppu->GetPixelBrightness(xPos, yPos) >= 85) {
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

	void InternalDrawController(InputHud& hud) override
	{
		hud.DrawOutline(11, 14);
		hud.DrawButton(2, 1, 7, 5, IsPressed(Buttons::Fire));
		hud.DrawNumber(hud.GetControllerIndex() + 1, 4, 7);
		
		hud.DrawMousePosition(GetCoordinates());
	}
};