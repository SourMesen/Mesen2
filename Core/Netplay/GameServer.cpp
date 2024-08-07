#include "pch.h"
#include "Netplay/GameServer.h"
#include "Netplay/GameServerConnection.h"
#include "Netplay/PlayerListMessage.h"
#include "Shared/Emulator.h"
#include "Shared/BaseControlManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/Socket.h"
#include "Shared/ControllerHub.h"

GameServer::GameServer(Emulator* emu)
{
	_emu = emu;
	_stop = false;
	_initialized = false;
	_hostControllerPort = {};
}

GameServer::~GameServer()
{
}

void GameServer::RegisterServerInput()
{
	_emu->RegisterInputProvider(this);
	_emu->RegisterInputRecorder(this);
}

void GameServer::AcceptConnections()
{
	while(true) {
		unique_ptr<Socket> socket = _listener->Accept();
		if(!socket->ConnectionError()) {
			_openConnections.push_back(unique_ptr<GameServerConnection>(new GameServerConnection(this, _emu, std::move(socket), _password)));
		} else {
			break;
		}
	}
	_listener->Listen(10);
}

void GameServer::UpdateConnections()
{
	vector<unique_ptr<GameServerConnection>> connectionsToRemove;
	for(int i = (int)_openConnections.size() - 1; i >= 0; i--) {
		if(_openConnections[i]->ConnectionError()) {
			//Pause emu thread to ensure nothing else modifies/accesses the _openConnections list while removing dead connections
			auto lock = _emu->AcquireLock();
			_openConnections.erase(_openConnections.begin() + i);
		} else {
			_openConnections[i]->ProcessMessages();
		}
	}
}

bool GameServer::SetInput(BaseControlDevice *device)
{
	uint8_t port = device->GetPort();
	IControllerHub* hub = dynamic_cast<IControllerHub*>(device);
	if(hub) {
		for(int i = 0, len = hub->GetHubPortCount(); i < len; i++) {
			NetplayControllerInfo controller { port, (uint8_t)i };
			GameServerConnection* connection = GetNetPlayDevice(controller);
			if(connection) {
				shared_ptr<BaseControlDevice> hubController = hub->GetController(i);
				if(hubController) {
					hubController->SetRawState(connection->GetState());
				}
			}
		}
		hub->RefreshHubState();
	} else {
		NetplayControllerInfo controller { port, 0 };
		GameServerConnection* connection = GetNetPlayDevice(controller);
		if(connection) {
			//Device is controlled by a client
			device->SetRawState(connection->GetState());
			return true;
		}
	}

	//Host is controlling this device
	return false;
}

void GameServer::RecordInput(vector<shared_ptr<BaseControlDevice>> devices)
{
	for(shared_ptr<BaseControlDevice> &device : devices) {
		for(unique_ptr<GameServerConnection>& connection : _openConnections) {
			if(!connection->ConnectionError()) {
				//Send movie stream
				connection->SendMovieData(device->GetPort(), device->GetRawState());
			}
		}
	}
}

void GameServer::ProcessNotification(ConsoleNotificationType type, void * parameter)
{
	for(unique_ptr<GameServerConnection>& connection : _openConnections) {
		connection->ProcessNotification(type, parameter);
	}

	if(type == ConsoleNotificationType::GameLoaded) {
		//Register the server as an input provider/recorder
		RegisterServerInput();
	}
}

void GameServer::Exec()
{
	_listener.reset(new Socket());
	_listener->Bind(_port);
	_listener->Listen(10);
	_stop = false;
	_initialized = true;
	MessageManager::DisplayMessage("NetPlay" , "ServerStarted", std::to_string(_port));

	while(!_stop) {
		AcceptConnections();
		UpdateConnections();

		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(1));
	}
}

void GameServer::StartServer(uint16_t port, string password)
{
	_port = port;
	_password = password;

	_emu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());

	//If a game is already running, register ourselves as an input recorder/provider
	RegisterServerInput();

	_serverThread.reset(new thread(&GameServer::Exec, this));
}

void GameServer::StopServer()
{
	if(!_serverThread) {
		return;
	}

	_stop = true;

	if(_serverThread) {
		_serverThread->join();
		_serverThread.reset();
	}

	_openConnections.clear();
	_initialized = false;
	_listener.reset();
	MessageManager::DisplayMessage("NetPlay", "ServerStopped");

	_emu->UnregisterInputRecorder(this);
	_emu->UnregisterInputProvider(this);
}

bool GameServer::Started()
{
	return _initialized;
}

NetplayControllerInfo GameServer::GetHostControllerPort()
{
	return _hostControllerPort;
}

void GameServer::SetHostControllerPort(NetplayControllerInfo controller)
{
	if(Started()) {
		auto lock = _emu->AcquireLock();

		if(controller.Port != GameConnection::SpectatorPort) {
			for(NetplayControllerUsageInfo& c : GetControllerList()) {
				if(c.InUse && controller.Port == c.Port.Port && controller.SubPort == c.Port.SubPort) {
					//Controller is in use
					return;
				}
			}
		}

		//Port is available
		_hostControllerPort = controller;
		SendPlayerList();
	}
}

vector<NetplayControllerUsageInfo> GameServer::GetControllerList()
{
	vector<PlayerInfo> players = GetPlayerList();
	return GetControllerList(_emu, players);
}

vector<NetplayControllerUsageInfo> GameServer::GetControllerList(Emulator* emu, vector<PlayerInfo>& players)
{
	vector<NetplayControllerUsageInfo> controllers;
	shared_ptr<IConsole> console = emu->GetConsole();
	if(!console) {
		return controllers;
	}

	for(uint8_t i = 0; i < BaseControlDevice::PortCount; i++) {
		for(uint8_t j = 0; j < IControllerHub::MaxSubPorts; j++) {
			shared_ptr<BaseControlDevice> controller = console->GetControlManager()->GetControlDevice(i, j);
			if(controller) {
				NetplayControllerUsageInfo result = {};
				result.Port.Port = i;
				result.Port.SubPort = j;
				result.Type = controller->GetControllerType();
				result.InUse = false;

				for(PlayerInfo& player : players) {
					if(player.ControllerPort.Port == i && player.ControllerPort.SubPort == j) {
						result.InUse = true;
						break;
					}
				}

				controllers.push_back(result);
			}
		}
	}
	return controllers;
}

vector<PlayerInfo> GameServer::GetPlayerList()
{
	vector<PlayerInfo> playerList;

	PlayerInfo playerInfo;
	playerInfo.ControllerPort = GetHostControllerPort();
	playerInfo.IsHost = true;
	playerList.push_back(playerInfo);

	for(unique_ptr<GameServerConnection>& connection : _openConnections) {
		playerInfo.ControllerPort = connection->GetControllerPort();
		playerInfo.IsHost = false;
		playerList.push_back(playerInfo);
	}

	return playerList;
}

void GameServer::SendPlayerList()
{
	vector<PlayerInfo> playerList = GetPlayerList();

	for(unique_ptr<GameServerConnection>& connection : _openConnections) {
		//Send player list update to all connections
		PlayerListMessage message(playerList);
		connection->SendNetMessage(message);
	}
}

void GameServer::RegisterNetPlayDevice(GameServerConnection* device, NetplayControllerInfo controller)
{
	_netPlayDevices[controller.Port][controller.SubPort] = device;
}

void GameServer::UnregisterNetPlayDevice(GameServerConnection* device)
{
	if(device != nullptr) {
		for(int i = 0; i < BaseControlDevice::PortCount; i++) {
			for(int j = 0; j < IControllerHub::MaxSubPorts; j++) {
				if(_netPlayDevices[i][j] == device) {
					_netPlayDevices[i][j] = nullptr;
					return;
				}
			}
		}
	}
}

GameServerConnection* GameServer::GetNetPlayDevice(NetplayControllerInfo controller)
{
	return _netPlayDevices[controller.Port][controller.SubPort];
}

NetplayControllerInfo GameServer::GetFirstFreeControllerPort()
{
	for(NetplayControllerUsageInfo& c : GetControllerList()) {
		if(!c.InUse) {
			return c.Port;
		}
	}
	return NetplayControllerInfo { GameConnection::SpectatorPort, 0 };
}
