#include "stdafx.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Netplay/ClientConnectionData.h"
#include "Core/Netplay/GameServer.h"
#include "Core/Netplay/GameClient.h"

extern shared_ptr<Emulator> _emu;

extern "C" {
	DllExport void __stdcall StartServer(uint16_t port, char* password) { GameServer::StartServer(_emu, port, password); }
	DllExport void __stdcall StopServer() { GameServer::StopServer(); }
	DllExport bool __stdcall IsServerRunning() { return GameServer::Started(); }

	DllExport void __stdcall Connect(char* host, uint16_t port, char* password, bool spectator)
	{
		ClientConnectionData connectionData(host, port, password, spectator);
		GameClient::Connect(_emu, connectionData);
	}

	DllExport void __stdcall Disconnect() { GameClient::Disconnect(); }
	DllExport bool __stdcall IsConnected() { return GameClient::Connected(); }

	DllExport int32_t __stdcall NetPlayGetAvailableControllers()
	{
		if(GameServer::Started()) {
			return GameServer::GetAvailableControllers();
		} else {
			return GameClient::GetAvailableControllers();
		}
	}

	DllExport void __stdcall NetPlaySelectController(int32_t port)
	{
		if(GameServer::Started()) {
			return GameServer::SetHostControllerPort(port);
		} else {
			return GameClient::SelectController(port);
		}
	}

	DllExport int32_t __stdcall NetPlayGetControllerPort()
	{
		if(GameServer::Started()) {
			return GameServer::GetHostControllerPort();
		} else {
			return GameClient::GetControllerPort();
		}
	}
}