#include "stdafx.h"
#include "Emulator.h"
#include "EmuSettings.h"
#include "GbControlManager.h"
#include "SNES/Input/SnesController.h"
#include "KeyManager.h"

GbControlManager::GbControlManager(Emulator* emu)
{
	_emu = emu;
}

void GbControlManager::UpdateInputState()
{
	KeyManager::RefreshKeyState();
	_device->ClearState();
	_device->SetStateFromInput();
	_device->OnAfterSetState();
}

void GbControlManager::RegisterInputProvider(IInputProvider* provider)
{
}

void GbControlManager::UnregisterInputProvider(IInputProvider* provider)
{
}

void GbControlManager::RegisterInputRecorder(IInputRecorder* provider)
{
}

void GbControlManager::UnregisterInputRecorder(IInputRecorder* provider)
{
}

shared_ptr<BaseControlDevice> GbControlManager::GetControlDevice(uint8_t port)
{
	return _device;
}

void GbControlManager::SetPollCounter(uint32_t pollCounter)
{
	//TODO
}

uint32_t GbControlManager::GetPollCounter()
{
	//TODO
	return 0;
}

void GbControlManager::UpdateControlDevices()
{
	InputConfig cfg = _emu->GetSettings()->GetInputConfig();
	_device.reset(new SnesController(_emu, 0, cfg.Controllers[0].Keys));
}
