#include "stdafx.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Interfaces/IAudioDevice.h"
#include "Core/Shared/BaseControlManager.h"
#include "Core/Shared/BaseControlDevice.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/SettingTypes.h"
#include "Utilities/StringUtilities.h"

extern unique_ptr<Emulator> _emu;
extern unique_ptr<IAudioDevice> _soundManager;

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

	DllExport void __stdcall SetPcEngineConfig(PcEngineConfig config)
	{
		_emu->GetSettings()->SetPcEngineConfig(config);
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

	DllExport void __stdcall SetDebugConfig(DebugConfig config)
	{
		_emu->GetSettings()->SetDebugConfig(config);
	}

	DllExport void __stdcall SetShortcutKeys(ShortcutKeyInfo shortcuts[], uint32_t count)
	{
		vector<ShortcutKeyInfo> shortcutList(shortcuts, shortcuts + count);
		_emu->GetSettings()->SetShortcutKeys(shortcutList);
	}

	DllExport ControllerType __stdcall GetControllerType(int player)
	{
		BaseControlManager* controlManager = _emu->GetControlManager();
		if(controlManager) {
			shared_ptr<BaseControlDevice> device = controlManager->GetControlDevice(player);
			if(device) {
				return device->GetControllerType();
			}
		}
		return ControllerType::None;
	}

	DllExport void __stdcall GetAudioDevices(char* outDeviceList, uint32_t maxLength)
	{
		StringUtilities::CopyToBuffer(_soundManager ? _soundManager->GetAvailableDevices() : "", outDeviceList, maxLength);
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