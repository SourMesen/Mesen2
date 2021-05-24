#include "stdafx.h"
#include <random>
#include "Netplay/GameServerConnection.h"
#include "Netplay/HandShakeMessage.h"
#include "Netplay/InputDataMessage.h"
#include "Netplay/MovieDataMessage.h"
#include "Netplay/GameInformationMessage.h"
#include "Netplay/SaveStateMessage.h"
#include "Netplay/ClientConnectionData.h"
#include "Netplay/SelectControllerMessage.h"
#include "Netplay/PlayerListMessage.h"
#include "Netplay/GameServer.h"
#include "Netplay/ForceDisconnectMessage.h"
#include "Netplay/ServerInformationMessage.h"
#include "SNES/ControlManager.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BaseControlDevice.h"

GameServerConnection* GameServerConnection::_netPlayDevices[BaseControlDevice::PortCount] = { };

GameServerConnection::GameServerConnection(shared_ptr<Emulator> emu, shared_ptr<Socket> socket, string serverPassword) : GameConnection(emu, socket)
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
	_emu->Lock();
	RomInfo romInfo = _emu->GetRomInfo();
	GameInformationMessage gameInfo(romInfo.RomFile.GetFileName(), _emu->GetHash(HashType::Sha1), _controllerPort, _emu->IsPaused());
	SendNetMessage(gameInfo);
	SaveStateMessage saveState(_emu);
	SendNetMessage(saveState);
	_emu->Unlock();
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
	string playerPortMessage = _controllerPort == GameConnection::SpectatorPort ? "Spectator" : "Player " + std::to_string(_controllerPort + 1);
	if(message->IsValid(_emu->GetSettings()->GetVersion())) {
		if(message->CheckPassword(_serverPassword, _connectionHash)) {
			_emu->Lock();

			_controllerPort = message->IsSpectator() ? GameConnection::SpectatorPort : GetFirstFreeControllerPort();

			MessageManager::DisplayMessage("NetPlay", playerPortMessage + " connected.");

			if(_emu->IsRunning()) {
				SendGameInformation();
			}

			_handshakeCompleted = true;
			RegisterNetPlayDevice(this, _controllerPort);
			GameServer::SendPlayerList();
			_emu->Unlock();
		} else {
			SendForceDisconnectMessage("The password you provided did not match - you have been disconnected.");
		}
	} else {
		SendForceDisconnectMessage("Server is using a different version of Mesen-S (" + _emu->GetSettings()->GetVersionString() + ") - you have been disconnected.");
		MessageManager::DisplayMessage("NetPlay", + "NetplayVersionMismatch", playerPortMessage);
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
	_emu->Lock();
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
	_emu->Unlock();
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