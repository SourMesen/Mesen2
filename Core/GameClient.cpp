#include "stdafx.h"
#include <thread>
using std::thread;

#include "MessageManager.h"
#include "GameClient.h"
#include "Console.h"
#include "NotificationManager.h"
#include "../Utilities/Socket.h"
#include "ClientConnectionData.h"
#include "GameClientConnection.h"

shared_ptr<GameClient> GameClient::_instance;

GameClient::GameClient(shared_ptr<Console> console)
{
	_console = console;
	_stop = false;
}

GameClient::~GameClient()
{
	_stop = true;
	if(_clientThread) {
		_clientThread->join();
	}
}

bool GameClient::Connected()
{
	shared_ptr<GameClient> instance = _instance;
	return instance ? instance->_connected : false;
}

void GameClient::Connect(shared_ptr<Console> console, ClientConnectionData &connectionData)
{
	_instance.reset(new GameClient(console));
	console->GetNotificationManager()->RegisterNotificationListener(_instance);
	
	shared_ptr<GameClient> instance = _instance;
	if(instance) {
		instance->PrivateConnect(connectionData);
		instance->_clientThread.reset(new thread(&GameClient::Exec, instance.get()));
	}
}

void GameClient::Disconnect()
{
	_instance.reset();
}

shared_ptr<GameClientConnection> GameClient::GetConnection()
{
	shared_ptr<GameClient> instance = _instance;
	return instance ? instance->_connection : nullptr;
}

void GameClient::PrivateConnect(ClientConnectionData &connectionData)
{
	_stop = false;
	shared_ptr<Socket> socket(new Socket());
	if(socket->Connect(connectionData.Host.c_str(), connectionData.Port)) {
		_connection.reset(new GameClientConnection(_console, socket, connectionData));
		_console->GetNotificationManager()->RegisterNotificationListener(_connection);
		_connected = true;
	} else {
		MessageManager::DisplayMessage("NetPlay", "CouldNotConnect");
		_connected = false;
	}
}

void GameClient::Exec()
{
	if(_connected) {
		while(!_stop) {
			if(!_connection->ConnectionError()) {
				_connection->ProcessMessages();
				_connection->SendInput();
			} else {
				_connected = false;
				_connection->Shutdown();
				_connection.reset();
				break;
			}
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(1));
		}
	}
}

void GameClient::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::GameLoaded &&
		std::this_thread::get_id() != _clientThread->get_id() && 
		std::this_thread::get_id() != _console->GetEmulationThreadId()
	) {
		//Disconnect if the client tried to manually load a game
		//A deadlock occurs if this is called from the emulation thread while a network message is being processed
		GameClient::Disconnect();
	}
}

void GameClient::SelectController(uint8_t port)
{
	shared_ptr<GameClientConnection> connection = GetConnection();
	if(connection) {
		connection->SelectController(port);
	}
}

uint8_t GameClient::GetAvailableControllers()
{
	shared_ptr<GameClientConnection> connection = GetConnection();
	return connection ? connection->GetAvailableControllers() : 0;
}

uint8_t GameClient::GetControllerPort()
{
	shared_ptr<GameClientConnection> connection = GetConnection();
	return connection ? connection->GetControllerPort() : GameConnection::SpectatorPort;
}