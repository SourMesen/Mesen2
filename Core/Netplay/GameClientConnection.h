#pragma once
#include "pch.h"
#include <deque>
#include "Utilities/AutoResetEvent.h"
#include "Utilities/SimpleLock.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/ControlDeviceState.h"
#include "Netplay/GameConnection.h"
#include "Netplay/ClientConnectionData.h"
#include "Netplay/NetplayTypes.h"

class Emulator;

class GameClientConnection final : public GameConnection, public INotificationListener, public IInputProvider
{
private:
	std::deque<ControlDeviceState> _inputData[BaseControlDevice::PortCount];
	atomic<uint32_t> _inputSize[BaseControlDevice::PortCount];
	AutoResetEvent _waitForInput[BaseControlDevice::PortCount];
	SimpleLock _writeLock;
	atomic<bool> _shutdown;
	atomic<bool> _enableControllers;
	atomic<uint32_t> _minimumQueueSize;

	vector<PlayerInfo> _playerList;

	shared_ptr<BaseControlDevice> _controlDevice;
	atomic<ControllerType> _controllerType;
	ControlDeviceState _lastInputSent = {};
	bool _gameLoaded = false;
	NetplayControllerInfo _controllerPort = { GameConnection::SpectatorPort, 0 };
	ClientConnectionData _connectionData = {};
	string _serverSalt;

private:
	void SendHandshake();
	void SendControllerSelection(NetplayControllerInfo controller);
	void ClearInputData();
	void PushControllerState(uint8_t port, ControlDeviceState state);
	void DisableControllers();
	bool AttemptLoadGame(string filename, uint32_t crc32);

protected:
	void ProcessMessage(NetMessage* message) override;

public:
	GameClientConnection(Emulator* emu, unique_ptr<Socket> socket, ClientConnectionData &connectionData);
	virtual ~GameClientConnection();

	void Shutdown();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;

	bool SetInput(BaseControlDevice *device) override;
	void InitControlDevice();
	void SendInput();

	void SelectController(NetplayControllerInfo controller);
	vector<NetplayControllerUsageInfo> GetControllerList();
	NetplayControllerInfo GetControllerPort();
};