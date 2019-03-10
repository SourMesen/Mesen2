#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/EmuSettings.h"
#include "../Core/SettingTypes.h"

extern shared_ptr<Console> _console;

extern "C" {
	DllExport void __stdcall SetVideoConfig(VideoConfig config)
	{
		_console->GetSettings()->SetVideoConfig(config);
	}
}