#include "Common.h"
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

	DllExport void __stdcall NetPlayGetControllerList(NetplayControllerUsageInfo* list, int32_t& length)
	{
		vector<NetplayControllerUsageInfo> controllers;
		if(_emu->GetGameServer()->Started()) {
			controllers = _emu->GetGameServer()->GetControllerList();
		} else {
			controllers = _emu->GetGameClient()->GetControllerList();
		}

		for(size_t i = 0; i < controllers.size() && i < length; i++) {
			list[i] = controllers[i];
		}
		length = (int32_t)controllers.size();
	}

	DllExport void __stdcall NetPlaySelectController(NetplayControllerInfo controller)
	{
		if(_emu->GetGameServer()->Started()) {
			return _emu->GetGameServer()->SetHostControllerPort(controller);
		} else {
			return _emu->GetGameClient()->SelectController(controller);
		}
	}

	DllExport NetplayControllerInfo __stdcall NetPlayGetControllerPort()
	{
		if(_emu->GetGameServer()->Started()) {
			return _emu->GetGameServer()->GetHostControllerPort();
		} else {
			return _emu->GetGameClient()->GetControllerPort();
		}
	}
}