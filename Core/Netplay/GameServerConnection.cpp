#include "pch.h"
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
#include "Netplay/NetplayTypes.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BaseControlDevice.h"

GameServerConnection::GameServerConnection(GameServer* gameServer, Emulator* emu, unique_ptr<Socket> socket, string serverPassword) : GameConnection(emu, std::move(socket))
{
	//Server-side connection
	_server = gameServer;
	_serverPassword = serverPassword;
	_controllerPort = NetplayControllerInfo { GameConnection::SpectatorPort, 0 };
	SendServerInformation();
}

GameServerConnection::~GameServerConnection()
{
	MessageManager::DisplayMessage("NetPlay", "Player disconnected.");
	_server->UnregisterNetPlayDevice(this);
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
	auto lock = _emu->AcquireLock();
	RomInfo romInfo = _emu->GetRomInfo();
	GameInformationMessage gameInfo(romInfo.RomFile.GetFileName(), _emu->GetCrc32(), _controllerPort, _emu->IsPaused());
	SendNetMessage(gameInfo);
	SaveStateMessage saveState(_emu);
	SendNetMessage(saveState);
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
	auto lock = _inputLock.AcquireSafe();
	_inputData = state;
}

ControlDeviceState GameServerConnection::GetState()
{
	ControlDeviceState stateData;
	{
		auto lock = _inputLock.AcquireSafe();
		stateData = _inputData;
	}
	return stateData;
}

void GameServerConnection::ProcessHandshakeResponse(HandShakeMessage* message)
{
	//Send the game's current state to the client and register the controller
	if(message->IsValid(_emu->GetSettings()->GetVersion())) {
		if(message->CheckPassword(_serverPassword, _connectionHash)) {
			auto lock = _emu->AcquireLock();

			_controllerPort = message->IsSpectator() ? NetplayControllerInfo { GameConnection::SpectatorPort, 0 } : _server->GetFirstFreeControllerPort();

			MessageManager::DisplayMessage("NetPlay", "Player connected.");

			if(_emu->IsRunning()) {
				SendGameInformation();
			}

			_handshakeCompleted = true;
			_server->RegisterNetPlayDevice(this, _controllerPort);
			_server->SendPlayerList();
		} else {
			SendForceDisconnectMessage("The password you provided did not match - you have been disconnected.");
		}
	} else {
		SendForceDisconnectMessage("Server is using a different version of Mesen (" + _emu->GetSettings()->GetVersionString() + ") - you have been disconnected.");
		MessageManager::DisplayMessage("NetPlay", + "NetplayVersionMismatch");
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
			SelectControllerPort(((SelectControllerMessage*)message)->GetController());
			break;

		default:
			break;
	}
}

void GameServerConnection::SelectControllerPort(NetplayControllerInfo controller)
{
	auto lock = _emu->AcquireLock();
	if(controller.Port == GameConnection::SpectatorPort) {
		//Client wants to be a spectator, make sure we are not using any controller
		_server->UnregisterNetPlayDevice(this);
		_controllerPort = controller;
	} else {
		GameServerConnection* netPlayDevice = _server->GetNetPlayDevice(controller);
		if(netPlayDevice == this) {
			//Nothing to do, we're already this player
		} else if(netPlayDevice == nullptr) {
			//This port is free, we can switch
			_server->UnregisterNetPlayDevice(this);
			_server->RegisterNetPlayDevice(this, controller);
			_controllerPort = controller;
		} else {
			//Another player is using this port, we can't use it
		}
	}
	SendGameInformation();
	_server->SendPlayerList();
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
		
		case ConsoleNotificationType::PpuFrameDone: {
			//Detect any configuration change that impacts emulation
			//Send a save state to clients if any change is done
			Serializer s(0, true);
			EmuSettings* settings = _emu->GetSettings();
			s.Stream(*settings, "", -1);
			stringstream currentConfig;
			s.SaveTo(currentConfig, 0);

			if(_previousConfig != currentConfig.str()) {
				SendGameInformation();
			}
			_previousConfig = currentConfig.str();
			break;
		}

		case ConsoleNotificationType::BeforeEmulationStop: {
			//Make clients unload the current game
			GameInformationMessage gameInfo("", 0, _controllerPort, true);
			SendNetMessage(gameInfo);
			break;
		}

		default:
			break;
	}
}

NetplayControllerInfo GameServerConnection::GetControllerPort()
{
	return _controllerPort;
}