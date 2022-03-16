#include "stdafx.h"
#include "Netplay/GameServer.h"
#include "Netplay/GameServerConnection.h"
#include "Netplay/PlayerListMessage.h"
#include "Shared/Emulator.h"
#include "Shared/BaseControlManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/Socket.h"

GameServer::GameServer(Emulator* emu)
{
	_emu = emu;
	_stop = false;
	_initialized = false;
	_hostControllerPort = 0;
}

GameServer::~GameServer()
{
}

void GameServer::RegisterServerInput()
{
	if(_emu->IsRunning()) {
		BaseControlManager* controlManager = _emu->GetControlManager();
		if(controlManager) {
			controlManager->RegisterInputRecorder(this);
			controlManager->RegisterInputProvider(this);
		}
	}
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
			_openConnections.erase(_openConnections.begin() + i);
		} else {
			_openConnections[i]->ProcessMessages();
		}
	}
}

bool GameServer::SetInput(BaseControlDevice *device)
{
	uint8_t port = device->GetPort();

	if(device->GetControllerType() == ControllerType::Multitap) {
		//TODO
		//Need special handling for the multitap, merge data from P3/4/5 with P1 (or P2, depending which port the multitap is plugged into)
		/*GameServerConnection* connection = GetNetPlayDevice(port);
		if(connection) {
			((Multitap*)device)->SetControllerState(0, connection->GetState());
		}

		for(int i = 2; i < 5; i++) {
			connection = GetNetPlayDevice(i);
			if(connection) {
				((Multitap*)device)->SetControllerState(i - 1, connection->GetState());
			}
		}*/
	} else {
		GameServerConnection* connection = GetNetPlayDevice(port);
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
	_stop = true;

	if(_serverThread) {
		_serverThread->join();
		_serverThread.reset();
	}

	_initialized = false;
	_listener.reset();
	MessageManager::DisplayMessage("NetPlay", "ServerStopped");

	if(_emu->IsRunning()) {
		BaseControlManager* controlManager = _emu->GetControlManager();
		if(controlManager) {
			controlManager->UnregisterInputRecorder(this);
			controlManager->UnregisterInputProvider(this);
		}
	}
}

bool GameServer::Started()
{
	return _initialized;
}

uint8_t GameServer::GetHostControllerPort()
{
	return _hostControllerPort;
}

void GameServer::SetHostControllerPort(uint8_t port)
{
	if(Started()) {
		auto lock = _emu->AcquireLock();
		if(port == GameConnection::SpectatorPort || GetAvailableControllers() & (1 << port)) {
			//Port is available
			_hostControllerPort = port;
			SendPlayerList();
		}
	}
}

uint8_t GameServer::GetAvailableControllers()
{
	uint8_t availablePorts = (1 << BaseControlDevice::PortCount) - 1;
	for(PlayerInfo &playerInfo : GetPlayerList()) {
		if(playerInfo.ControllerPort < BaseControlDevice::PortCount) {
			availablePorts &= ~(1 << playerInfo.ControllerPort);
		}
	}
	return availablePorts;
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

void GameServer::RegisterNetPlayDevice(GameServerConnection* device, uint8_t port)
{
	_netPlayDevices[port] = device;
}

void GameServer::UnregisterNetPlayDevice(GameServerConnection* device)
{
	if(device != nullptr) {
		for(int i = 0; i < BaseControlDevice::PortCount; i++) {
			if(_netPlayDevices[i] == device) {
				_netPlayDevices[i] = nullptr;
				break;
			}
		}
	}
}

GameServerConnection* GameServer::GetNetPlayDevice(uint8_t port)
{
	return _netPlayDevices[port];
}

uint8_t GameServer::GetFirstFreeControllerPort()
{
	uint8_t hostPost = _emu->GetGameServer()->GetHostControllerPort();
	for(int i = 0; i < BaseControlDevice::PortCount; i++) {
		if(hostPost != i && _netPlayDevices[i] == nullptr) {
			return i;
		}
	}
	return GameConnection::SpectatorPort;
}
