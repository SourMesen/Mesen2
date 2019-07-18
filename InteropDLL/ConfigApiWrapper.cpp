#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/IAudioDevice.h"
#include "../Core/EmuSettings.h"
#include "../Core/SettingTypes.h"

extern shared_ptr<Console> _console;
extern unique_ptr<IAudioDevice> _soundManager;
static string _returnString;

extern "C" {
	DllExport void __stdcall SetVideoConfig(VideoConfig config)
	{
		_console->GetSettings()->SetVideoConfig(config);
	}

	DllExport void __stdcall SetAudioConfig(AudioConfig config)
	{
		_console->GetSettings()->SetAudioConfig(config);
	}

	DllExport void __stdcall SetInputConfig(InputConfig config)
	{
		_console->GetSettings()->SetInputConfig(config);
	}

	DllExport void __stdcall SetEmulationConfig(EmulationConfig config)
	{
		_console->GetSettings()->SetEmulationConfig(config);
	}

	DllExport void __stdcall SetPreferences(PreferencesConfig config)
	{
		_console->GetSettings()->SetPreferences(config);
	}

	DllExport void __stdcall SetShortcutKeys(ShortcutKeyInfo shortcuts[], uint32_t count)
	{
		vector<ShortcutKeyInfo> shortcutList(shortcuts, shortcuts + count);
		_console->GetSettings()->SetShortcutKeys(shortcutList);
	}

	DllExport const char* __stdcall GetAudioDevices()
	{
		_returnString = _soundManager ? _soundManager->GetAvailableDevices() : "";
		return _returnString.c_str();
	}

	DllExport void __stdcall SetEmulationFlag(EmulationFlags flag, bool enabled)
	{
		_console->GetSettings()->SetFlagState(flag, enabled);
	}

	DllExport void __stdcall SetDebuggerFlag(DebuggerFlags flag, bool enabled)
	{
		_console->GetSettings()->SetDebuggerFlag(flag, enabled);
	}
}