#include "pch.h"
#include "Netplay/GameClientConnection.h"
#include "Netplay/HandShakeMessage.h"
#include "Netplay/InputDataMessage.h"
#include "Netplay/MovieDataMessage.h"
#include "Netplay/GameInformationMessage.h"
#include "Netplay/SaveStateMessage.h"
#include "Netplay/ClientConnectionData.h"
#include "Netplay/SelectControllerMessage.h"
#include "Netplay/PlayerListMessage.h"
#include "Netplay/ForceDisconnectMessage.h"
#include "Netplay/ServerInformationMessage.h"
#include "Netplay/GameServer.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/NotificationManager.h"
#include "Shared/RomFinder.h"

GameClientConnection::GameClientConnection(Emulator* emu, unique_ptr<Socket> socket, ClientConnectionData &connectionData) : GameConnection(emu, std::move(socket))
{
	_connectionData = connectionData;
	_shutdown = false;
	_enableControllers = false;
	_minimumQueueSize = 3;
	_controllerType = ControllerType::None;

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

		_emu->UnregisterInputProvider(this);

		MessageManager::DisplayMessage("NetPlay", "ConnectionLost");
		_emu->GetSettings()->ClearFlag(EmulationFlags::MaximumSpeed);
	}
	Disconnect();
}

void GameClientConnection::SendHandshake()
{
	HandShakeMessage message(HandShakeMessage::GetPasswordHash(_connectionData.Password, _serverSalt), _connectionData.Spectator, _emu->GetSettings()->GetVersion());
	SendNetMessage(message);
}

void GameClientConnection::SendControllerSelection(NetplayControllerInfo controller)
{
	SelectControllerMessage message(controller);
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

				auto lock = _emu->AcquireLock();
				ClearInputData();
				((SaveStateMessage*)message)->LoadState(_emu);
				_enableControllers = true;
				InitControlDevice();
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

			{
				auto lock = _emu->AcquireLock();
				gameInfo = (GameInformationMessage*)message;
				if(gameInfo->GetPort().Port != _controllerPort.Port || gameInfo->GetPort().SubPort != _controllerPort.SubPort) {
					_controllerPort = gameInfo->GetPort();
				}

				ClearInputData();
			}

			_gameLoaded = AttemptLoadGame(gameInfo->GetRomFilename(), gameInfo->GetCrc32());
			if(!_gameLoaded) {
				_emu->Stop(true);
			} else {
				_emu->UnregisterInputProvider(this);
				_emu->RegisterInputProvider(this);
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

bool GameClientConnection::AttemptLoadGame(string filename, uint32_t crc32)
{
	if(filename.size() > 0) {
		if(!RomFinder::LoadMatchingRom(_emu, filename, crc32)) {
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
	shared_ptr<IConsole> console = _emu->GetConsole();
	if(!console) {
		return;
	}

	BaseControlManager* controlManager = console->GetControlManager();
	shared_ptr<BaseControlDevice> device = controlManager->GetControlDevice(_controllerPort.Port, _controllerPort.SubPort);
	if(device) {
		_controllerType = device->GetControllerType();
	} else {
		_controllerType = ControllerType::None;
	}
}

void GameClientConnection::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::ConfigChanged) {
		InitControlDevice();
	} else if(type == ConsoleNotificationType::GameLoaded) {
		_emu->RegisterInputProvider(this);
	}
}

void GameClientConnection::SendInput()
{
	if(_gameLoaded) {
		if(!_controlDevice || _controllerType != _controlDevice->GetControllerType()) {
			//Pretend we are using port 0 (to use player 1's keybindings during netplay)
			shared_ptr<IConsole> console = _emu->GetConsole();
			if(!console) {
				return;
			}
			_controlDevice = console->GetControlManager()->CreateControllerDevice(_controllerType, 0);
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

void GameClientConnection::SelectController(NetplayControllerInfo controller)
{
	SendControllerSelection(controller);
}

vector<NetplayControllerUsageInfo> GameClientConnection::GetControllerList()
{
	return GameServer::GetControllerList(_emu, _playerList);
}

NetplayControllerInfo GameClientConnection::GetControllerPort()
{
	return _controllerPort;
}