#include "stdafx.h"
#include <random>
#include "MessageManager.h"
#include "GameServerConnection.h"
#include "HandShakeMessage.h"
#include "InputDataMessage.h"
#include "MovieDataMessage.h"
#include "GameInformationMessage.h"
#include "SaveStateMessage.h"
#include "Console.h"
#include "BaseCartridge.h"
#include "ControlManager.h"
#include "ClientConnectionData.h"
#include "EmuSettings.h"
#include "SelectControllerMessage.h"
#include "PlayerListMessage.h"
#include "GameServer.h"
#include "ForceDisconnectMessage.h"
#include "BaseControlDevice.h"
#include "ServerInformationMessage.h"

GameServerConnection* GameServerConnection::_netPlayDevices[BaseControlDevice::PortCount] = { };

GameServerConnection::GameServerConnection(shared_ptr<Console> console, shared_ptr<Socket> socket, string serverPassword) : GameConnection(console, socket)
{
	//Server-side connection
	_serverPassword = serverPassword;
	_controllerPort = GameConnection::SpectatorPort;
	SendServerInformation();
}

GameServerConnection::~GameServerConnection()
{
	if(!_playerName.empty()) {
		MessageManager::DisplayMessage("NetPlay", _playerName + " (Player " + std::to_string(_controllerPort + 1) + ") disconnected.");
	}

	UnregisterNetPlayDevice(this);
}

void GameServerConnection::SendServerInformation()
{
	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_int_distribution<> dist((int)' ', (int)'~');
	string hash(50, ' ');
	for(int i = 0; i < 50; i++) {
		int random = dist(engine);
		hash[i] = (char)random;
	}

	_connectionHash = hash;

	ServerInformationMessage message(hash);
	SendNetMessage(message);
}

void GameServerConnection::SendGameInformation()
{
	_console->Lock();
	RomInfo romInfo = _console->GetRomInfo();
	GameInformationMessage gameInfo(romInfo.RomFile.GetFileName(), _console->GetCartridge()->GetSha1Hash(), _controllerPort, _console->IsPaused());
	SendNetMessage(gameInfo);
	SaveStateMessage saveState(_console);
	SendNetMessage(saveState);
	_console->Unlock();
}

void GameServerConnection::SendMovieData(uint8_t port, ControlDeviceState state)
{
	if(_handshakeCompleted) {
		MovieDataMessage message(state, port);
		SendNetMessage(message);
	}
}

void GameServerConnection::SendForceDisconnectMessage(string disconnectMessage)
{
	ForceDisconnectMessage message(disconnectMessage);
	SendNetMessage(message);
	Disconnect();
}

void GameServerConnection::PushState(ControlDeviceState state)
{
	if(_inputData.size() == 0 || state != _inputData.back()) {
		_inputData.clear();
		_inputData.push_back(state);
	}
}

ControlDeviceState GameServerConnection::GetState()
{
	size_t inputBufferSize = _inputData.size();
	ControlDeviceState stateData;
	if(inputBufferSize > 0) {
		stateData = _inputData.front();
		if(inputBufferSize > 1) {
			//Always keep the last one the client sent, it will be used until a new one is received
			_inputData.pop_front();
		}
	}
	return stateData;
}

void GameServerConnection::ProcessHandshakeResponse(HandShakeMessage* message)
{
	//Send the game's current state to the client and register the controller
	if(message->IsValid(_console->GetSettings()->GetVersion())) {
		if(message->CheckPassword(_serverPassword, _connectionHash)) {
			_console->Lock();

			_controllerPort = message->IsSpectator() ? GameConnection::SpectatorPort : GetFirstFreeControllerPort();
			_playerName = message->GetPlayerName();

			string playerPortMessage = _controllerPort == GameConnection::SpectatorPort ? "Spectator" : "Player " + std::to_string(_controllerPort + 1);

			MessageManager::DisplayMessage("NetPlay", _playerName + " (" + playerPortMessage + ") connected.");

			if(_console->GetCartridge()) {
				SendGameInformation();
			}

			_handshakeCompleted = true;
			RegisterNetPlayDevice(this, _controllerPort);
			GameServer::SendPlayerList();
			_console->Unlock();
		} else {
			SendForceDisconnectMessage("The password you provided did not match - you have been disconnected.");
		}
	} else {
		SendForceDisconnectMessage("Server is using a different version of Mesen-S (" + _console->GetSettings()->GetVersionString() + ") - you have been disconnected.");
		MessageManager::DisplayMessage("NetPlay", + "NetplayVersionMismatch", message->GetPlayerName());
	}
}

void GameServerConnection::ProcessMessage(NetMessage* message)
{
	switch(message->GetType()) {
		case MessageType::HandShake:
			ProcessHandshakeResponse((HandShakeMessage*)message);
			break;

		case MessageType::InputData:
			if(!_handshakeCompleted) {
				SendForceDisconnectMessage("Handshake has not been completed - invalid packet");
				return;
			}
			PushState(((InputDataMessage*)message)->GetInputState());
			break;

		case MessageType::SelectController:
			if(!_handshakeCompleted) {
				SendForceDisconnectMessage("Handshake has not been completed - invalid packet");
				return;
			}
			SelectControllerPort(((SelectControllerMessage*)message)->GetPortNumber());
			break;

		default:
			break;
	}
}

void GameServerConnection::SelectControllerPort(uint8_t port)
{
	_console->Lock();
	if(port == GameConnection::SpectatorPort) {
		//Client wants to be a spectator, make sure we are not using any controller
		UnregisterNetPlayDevice(this);
		_controllerPort = port;
	} else {
		GameServerConnection* netPlayDevice = GetNetPlayDevice(port);
		if(netPlayDevice == this) {
			//Nothing to do, we're already this player
		} else if(netPlayDevice == nullptr) {
			//This port is free, we can switch
			UnregisterNetPlayDevice(this);
			RegisterNetPlayDevice(this, port);
			_controllerPort = port;
		} else {
			//Another player is using this port, we can't use it
		}
	}
	SendGameInformation();
	GameServer::SendPlayerList();
	_console->Unlock();
}

void GameServerConnection::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	switch(type) {
		case ConsoleNotificationType::GamePaused:
		case ConsoleNotificationType::GameLoaded:
		case ConsoleNotificationType::GameResumed:
		case ConsoleNotificationType::GameReset:
		case ConsoleNotificationType::StateLoaded:
		case ConsoleNotificationType::CheatsChanged:
		case ConsoleNotificationType::ConfigChanged:
			SendGameInformation();
			break;

		case ConsoleNotificationType::BeforeEmulationStop: {
			//Make clients unload the current game
			GameInformationMessage gameInfo("", "0000000000000000000000000000000000000000", _controllerPort, true);
			SendNetMessage(gameInfo);
			break;
		}

		default:
			break;
	}
}

void GameServerConnection::RegisterNetPlayDevice(GameServerConnection* device, uint8_t port)
{
	GameServerConnection::_netPlayDevices[port] = device;
}

void GameServerConnection::UnregisterNetPlayDevice(GameServerConnection* device)
{
	if(device != nullptr) {
		for(int i = 0; i < BaseControlDevice::PortCount; i++) {
			if(GameServerConnection::_netPlayDevices[i] == device) {
				GameServerConnection::_netPlayDevices[i] = nullptr;
				break;
			}
		}
	}
}

GameServerConnection* GameServerConnection::GetNetPlayDevice(uint8_t port)
{
	return GameServerConnection::_netPlayDevices[port];
}

uint8_t GameServerConnection::GetFirstFreeControllerPort()
{
	uint8_t hostPost = GameServer::GetHostControllerPort();
	for(int i = 0; i < BaseControlDevice::PortCount; i++) {
		if(hostPost != i && GameServerConnection::_netPlayDevices[i] == nullptr) {
			return i;
		}
	}
	return GameConnection::SpectatorPort;
}

string GameServerConnection::GetPlayerName()
{
	return _playerName;
}

uint8_t GameServerConnection::GetControllerPort()
{
	return _controllerPort;
}