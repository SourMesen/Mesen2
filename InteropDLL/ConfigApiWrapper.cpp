#include "stdafx.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Interfaces/IAudioDevice.h"
#include "Core/Shared/Interfaces/IControlManager.h"
#include "Core/Shared/BaseControlDevice.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/SettingTypes.h"

extern shared_ptr<Emulator> _emu;
extern unique_ptr<IAudioDevice> _soundManager;
static string _returnString;

extern "C" {
	DllExport void __stdcall SetVideoConfig(VideoConfig config)
	{
		_emu->GetSettings()->SetVideoConfig(config);
	}

	DllExport void __stdcall SetAudioConfig(AudioConfig config)
	{
		_emu->GetSettings()->SetAudioConfig(config);
	}

	DllExport void __stdcall SetInputConfig(InputConfig config)
	{
		_emu->GetSettings()->SetInputConfig(config);
	}

	DllExport void __stdcall SetEmulationConfig(EmulationConfig config)
	{
		_emu->GetSettings()->SetEmulationConfig(config);
	}
	
	DllExport void __stdcall SetGameboyConfig(GameboyConfig config)
	{
		_emu->GetSettings()->SetGameboyConfig(config);
	}

	DllExport void __stdcall SetNesConfig(NesConfig config)
	{
		_emu->GetSettings()->SetNesConfig(config);
	}
	
	DllExport void __stdcall SetSnesConfig(SnesConfig config)
	{
		_emu->GetSettings()->SetSnesConfig(config);
	}

	DllExport void __stdcall SetPreferences(PreferencesConfig config)
	{
		_emu->GetSettings()->SetPreferences(config);
	}

	DllExport void __stdcall SetAudioPlayerConfig(AudioPlayerConfig config)
	{
		_emu->GetSettings()->SetAudioPlayerConfig(config);
	}

	DllExport void __stdcall SetShortcutKeys(ShortcutKeyInfo shortcuts[], uint32_t count)
	{
		vector<ShortcutKeyInfo> shortcutList(shortcuts, shortcuts + count);
		_emu->GetSettings()->SetShortcutKeys(shortcutList);
	}

	DllExport ControllerType __stdcall GetControllerType(int player)
	{
		shared_ptr<IControlManager> controlManager = _emu->GetControlManager();
		if(controlManager) {
			shared_ptr<BaseControlDevice> device = controlManager->GetControlDevice(player);
			if(device) {
				return device->GetControllerType();
			}
		}
		return ControllerType::None;
	}

	DllExport const char* __stdcall GetAudioDevices()
	{
		_returnString = _soundManager ? _soundManager->GetAvailableDevices() : "";
		return _returnString.c_str();
	}

	DllExport void __stdcall SetEmulationFlag(EmulationFlags flag, bool enabled)
	{
		_emu->GetSettings()->SetFlagState(flag, enabled);
	}

	DllExport void __stdcall SetDebuggerFlag(DebuggerFlags flag, bool enabled)
	{
		_emu->GetSettings()->SetDebuggerFlag(flag, enabled);
	}
}