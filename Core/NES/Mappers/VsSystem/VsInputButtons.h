#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/Interfaces/INotificationListener.h"

class VsInputButtons: public BaseControlDevice, public INotificationListener
{
private:
	static constexpr uint8_t InsertCoinFrameCount = 4;

	bool _isDualSystem = false;
	uint8_t _needInsertCoin[4] = { 0, 0, 0, 0 };
	bool _needServiceButton[2] = { false, false };

	void ProcessInsertCoin(uint8_t port)
	{
		if(_needInsertCoin[port] > 0) {
			_needInsertCoin[port]--;

			switch(port) {
				case 0: SetBit(VsInputButtons::VsButtons::InsertCoin1); break;
				case 1: SetBit(VsInputButtons::VsButtons::InsertCoin2); break;
				case 2: SetBit(VsInputButtons::VsButtons::InsertCoin3); break;
				case 3: SetBit(VsInputButtons::VsButtons::InsertCoin4); break;
			}			
		}
	}

	string GetKeyNames() override
	{
		return _isDualSystem ? "12S34S" : "12S";
	}

public:
	enum VsButtons { InsertCoin1 = 2, InsertCoin2, ServiceButton, InsertCoin3, InsertCoin4, ServiceButton2 };

	VsInputButtons(Emulator* emu, bool isDualSystem) : BaseControlDevice(emu, ControllerType::None, BaseControlDevice::MapperInputPort)
	{
		_isDualSystem = isDualSystem;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t value = 0;
		if(addr == 0x4016) {
			if(IsPressed(VsButtons::InsertCoin1)) {
				value |= 0x20;
			}
			if(IsPressed(VsButtons::InsertCoin2)) {
				value |= 0x40;
			}
			if(IsPressed(VsButtons::ServiceButton)) {
				value |= 0x04;
			}
		}
		return value;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	void OnAfterSetState() override
	{
		ProcessInsertCoin(0);
		ProcessInsertCoin(1);
		ProcessInsertCoin(2);
		ProcessInsertCoin(3);

		if(_needServiceButton[0]) {
			SetBit(VsButtons::ServiceButton);
		}
		if(_needServiceButton[1]) {
			SetBit(VsButtons::ServiceButton2);
		}
	}

	void InsertCoin(uint8_t port)
	{
		if(port < 4) {
			_emu->Pause();
			_needInsertCoin[port] = VsInputButtons::InsertCoinFrameCount;
			MessageManager::DisplayMessage("VS System", "CoinInsertedSlot", std::to_string(port + 1));
			_emu->Resume();
		}
	}

	void SetServiceButtonState(int consoleId, bool pressed)
	{
		_emu->Pause();
		_needServiceButton[consoleId] = pressed;
		_emu->Resume();
	}

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override
	{
		if(type == ConsoleNotificationType::ExecuteShortcut) {
			ExecuteShortcutParams* params = (ExecuteShortcutParams*)parameter;
			switch(params->Shortcut) {
				default: break;
				case EmulatorShortcut::VsInsertCoin1: InsertCoin(0); break;
				case EmulatorShortcut::VsInsertCoin2: InsertCoin(1); break;
				case EmulatorShortcut::VsInsertCoin3: InsertCoin(2); break;
				case EmulatorShortcut::VsInsertCoin4: InsertCoin(3); break;
				case EmulatorShortcut::VsServiceButton: SetServiceButtonState(0, true); break;
				case EmulatorShortcut::VsServiceButton2: SetServiceButtonState(1, true); break;
			}
		} else if(type == ConsoleNotificationType::ReleaseShortcut) {
			ExecuteShortcutParams* params = (ExecuteShortcutParams*)parameter;
			switch(params->Shortcut) {
				default: break;
				case EmulatorShortcut::VsServiceButton: SetServiceButtonState(0, false); break;
				case EmulatorShortcut::VsServiceButton2: SetServiceButtonState(1, false); break;
			}
		}
	}
};