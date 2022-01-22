#include "stdafx.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Netplay/ClientConnectionData.h"
#include "Core/Netplay/GameServer.h"
#include "Core/Netplay/GameClient.h"

extern unique_ptr<Emulator> _emu;

extern "C" {
	DllExport void __stdcall StartServer(uint16_t port, char* password) { _emu->GetGameServer()->StartServer(port, password); }
	DllExport void __stdcall StopServer() { _emu->GetGameServer()->StopServer(); }
	DllExport bool __stdcall IsServerRunning() { return _emu->GetGameServer()->Started(); }

	DllExport void __stdcall Connect(char* host, uint16_t port, char* password, bool spectator)
	{
		ClientConnectionData connectionData(host, port, password, spectator);
		_emu->GetGameClient()->Connect(connectionData);
	}

	DllExport void __stdcall Disconnect() { _emu->GetGameClient()->Disconnect(); }
	DllExport bool __stdcall IsConnected() { return _emu->GetGameClient()->Connected(); }

	DllExport int32_t __stdcall NetPlayGetAvailableControllers()
	{
		if(_emu->GetGameServer()->Started()) {
			return _emu->GetGameServer()->GetAvailableControllers();
		} else {
			return _emu->GetGameServer()->GetAvailableControllers();
		}
	}

	DllExport void __stdcall NetPlaySelectController(int32_t port)
	{
		if(_emu->GetGameServer()->Started()) {
			return _emu->GetGameServer()->SetHostControllerPort(port);
		} else {
			return _emu->GetGameClient()->SelectController(port);
		}
	}

	DllExport int32_t __stdcall NetPlayGetControllerPort()
	{
		if(_emu->GetGameServer()->Started()) {
			return _emu->GetGameServer()->GetHostControllerPort();
		} else {
			return _emu->GetGameClient()->GetControllerPort();
		}
	}
}