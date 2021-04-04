#include "stdafx.h"
#include "GameClientConnection.h"
#include "HandShakeMessage.h"
#include "InputDataMessage.h"
#include "MovieDataMessage.h"
#include "GameInformationMessage.h"
#include "SaveStateMessage.h"
#include "Emulator.h"
#include "EmuSettings.h"
#include "ControlManager.h"
#include "ClientConnectionData.h"
#include "SnesController.h"
#include "SelectControllerMessage.h"
#include "PlayerListMessage.h"
#include "ForceDisconnectMessage.h"
#include "ServerInformationMessage.h"
#include "NotificationManager.h"
#include "RomFinder.h"

GameClientConnection::GameClientConnection(shared_ptr<Emulator> emu, shared_ptr<Socket> socket, ClientConnectionData &connectionData) : GameConnection(emu, socket)
{
	_connectionData = connectionData;
	_shutdown = false;
	_enableControllers = false;
	_minimumQueueSize = 3;

	MessageManager::DisplayMessage("NetPlay", "ConnectedToServer");
}

GameClientConnection::~GameClientConnection()
{
	Shutdown();
}

void GameClientConnection::Shutdown()
{
	if(!_shutdown) {
		_shutdown = true;
		DisableControllers();

		shared_ptr<ControlManager> controlManager = _emu->GetControlManager();
		if(controlManager) {
			controlManager->UnregisterInputProvider(this);
		}

		MessageManager::DisplayMessage("NetPlay", "ConnectionLost");
		_emu->GetSettings()->ClearFlag(EmulationFlags::MaximumSpeed);
	}
}

void GameClientConnection::SendHandshake()
{
	HandShakeMessage message(_connectionData.PlayerName, HandShakeMessage::GetPasswordHash(_connectionData.Password, _serverSalt), _connectionData.Spectator, _emu->GetSettings()->GetVersion());
	SendNetMessage(message);
}

void GameClientConnection::SendControllerSelection(uint8_t port)
{
	SelectControllerMessage message(port);
	SendNetMessage(message);
}

void GameClientConnection::ClearInputData()
{
	LockHandler lock = _writeLock.AcquireSafe();
	for(int i = 0; i < BaseControlDevice::PortCount; i++) {
		_inputSize[i] = 0;
		_inputData[i].clear();
	}
}

void GameClientConnection::ProcessMessage(NetMessage* message)
{
	GameInformationMessage* gameInfo;

	switch(message->GetType()) {
		case MessageType::ServerInformation:
			_serverSalt = ((ServerInformationMessage*)message)->GetHashSalt();
			SendHandshake();
			break;

		case MessageType::SaveState:
			if(_gameLoaded) {
				DisableControllers();
				_emu->Lock();
				ClearInputData();
				((SaveStateMessage*)message)->LoadState(_emu);
				_enableControllers = true;
				InitControlDevice();
				_emu->Unlock();
			}
			break;

		case MessageType::MovieData:
			if(_gameLoaded) {
				PushControllerState(((MovieDataMessage*)message)->GetPortNumber(), ((MovieDataMessage*)message)->GetInputState());
			}
			break;

		case MessageType::ForceDisconnect:
			MessageManager::DisplayMessage("NetPlay", ((ForceDisconnectMessage*)message)->GetMessage());
			break;

		case MessageType::PlayerList:
			_playerList = ((PlayerListMessage*)message)->GetPlayerList();
			break;

		case MessageType::GameInformation:
			DisableControllers();
			_emu->Lock();
			gameInfo = (GameInformationMessage*)message;
			if(gameInfo->GetPort() != _controllerPort) {
				_controllerPort = gameInfo->GetPort();

				if(_controllerPort == GameConnection::SpectatorPort) {
					MessageManager::DisplayMessage("NetPlay", "ConnectedAsSpectator");
				} else {
					MessageManager::DisplayMessage("NetPlay", "ConnectedAsPlayer", std::to_string(_controllerPort + 1));
				}
			}

			ClearInputData();
			_emu->Unlock();

			_gameLoaded = AttemptLoadGame(gameInfo->GetRomFilename(), gameInfo->GetSha1Hash());
			if(!_gameLoaded) {
				_emu->Stop(true);
			} else {
				_emu->GetControlManager()->UnregisterInputProvider(this);
				_emu->GetControlManager()->RegisterInputProvider(this);
				if(gameInfo->IsPaused()) {
					_emu->Pause();
				} else {
					_emu->Resume();
				}
			}
			break;
		default:
			break;
	}
}

bool GameClientConnection::AttemptLoadGame(string filename, string sha1Hash)
{
	if(filename.size() > 0) {
		if(!RomFinder::LoadMatchingRom(_emu.get(), filename, sha1Hash)) {
			MessageManager::DisplayMessage("NetPlay", "CouldNotFindRom", filename);
			return false;
		} else {
			return true;
		}
	}
	return false;
}

void GameClientConnection::PushControllerState(uint8_t port, ControlDeviceState state)
{
	LockHandler lock = _writeLock.AcquireSafe();
	_inputData[port].push_back(state);
	_inputSize[port]++;

	if(_inputData[port].size() >= _minimumQueueSize) {
		_waitForInput[port].Signal();
	}
}

void GameClientConnection::DisableControllers()
{
	//Used to prevent deadlocks when client is trying to fill its buffer while the host changes the current game/settings/etc. (i.e situations where we need to call Console::Pause())
	_enableControllers = false;
	ClearInputData();
	for(int i = 0; i < BaseControlDevice::PortCount; i++) {
		_waitForInput[i].Signal();
	}
}

bool GameClientConnection::SetInput(BaseControlDevice *device)
{
	if(_enableControllers) {
		uint8_t port = device->GetPort();
		while(_inputSize[port] == 0) {
			_waitForInput[port].Wait();

			if(port == 0 && _minimumQueueSize < 10) {
				//Increase buffer size - reduces freezes at the cost of additional lag
				_minimumQueueSize++;
			}

			if(_shutdown || !_enableControllers) {
				return true;
			}
		}

		LockHandler lock = _writeLock.AcquireSafe();
		if(_shutdown || !_enableControllers || _inputSize[port] == 0) {
			return true;
		}

		ControlDeviceState state = _inputData[port].front();
		_inputData[port].pop_front();
		_inputSize[port]--;

		if(_inputData[port].size() > _minimumQueueSize) {
			//Too much data, catch up
			_emu->GetSettings()->SetFlag(EmulationFlags::MaximumSpeed);
		} else {
			_emu->GetSettings()->ClearFlag(EmulationFlags::MaximumSpeed);
		}

		device->SetRawState(state);
	}
	return true;
}

void GameClientConnection::InitControlDevice()
{
	//Pretend we are using port 0 (to use player 1's keybindings during netplay)
	//TODO
	//_newControlDevice = ControlManager::CreateControllerDevice(_emu->GetSettings()->GetInputConfig().Controllers[_controllerPort].Type, 0, _emu.get());
}

void GameClientConnection::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::ConfigChanged) {
		InitControlDevice();
	} else if(type == ConsoleNotificationType::GameLoaded) {
		_emu->GetControlManager()->RegisterInputProvider(this);
	}
}

void GameClientConnection::SendInput()
{
	if(_gameLoaded) {
		if(_newControlDevice) {
			_controlDevice = _newControlDevice;
			_newControlDevice.reset();
		}

		ControlDeviceState inputState;
		if(_controlDevice) {
			_controlDevice->SetStateFromInput();
			inputState = _controlDevice->GetRawState();
		}
		
		if(_lastInputSent != inputState) {
			InputDataMessage message(inputState);
			SendNetMessage(message);
			_lastInputSent = inputState;
		}
	}
}

void GameClientConnection::SelectController(uint8_t port)
{
	SendControllerSelection(port);
}

uint8_t GameClientConnection::GetAvailableControllers()
{
	uint8_t availablePorts = (1 << BaseControlDevice::PortCount) - 1;
	for(PlayerInfo &playerInfo : _playerList) {
		if(playerInfo.ControllerPort < BaseControlDevice::PortCount) {
			availablePorts &= ~(1 << playerInfo.ControllerPort);
		}
	}
	return availablePorts;
}

uint8_t GameClientConnection::GetControllerPort()
{
	return _controllerPort;
}