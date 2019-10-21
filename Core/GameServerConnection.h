#pragma once
#include "stdafx.h"
#include <deque>
#include "GameConnection.h"
#include "INotificationListener.h"
#include "BaseControlDevice.h"
#include "ControlDeviceState.h"

class HandShakeMessage;

class GameServerConnection : public GameConnection, public INotificationListener
{
private:
	static GameServerConnection* _netPlayDevices[BaseControlDevice::PortCount];

	list<ControlDeviceState> _inputData;
	string _playerName;
	int _controllerPort;	
	string _connectionHash;
	string _serverPassword;
	bool _handshakeCompleted = false;

	void PushState(ControlDeviceState state);
	void SendServerInformation();
	void SendGameInformation();
	void SelectControllerPort(uint8_t port);

	void SendForceDisconnectMessage(string disconnectMessage);

	void ProcessHandshakeResponse(HandShakeMessage* message);

	static void RegisterNetPlayDevice(GameServerConnection* connection, uint8_t port);
	static void UnregisterNetPlayDevice(GameServerConnection* device);
	static uint8_t GetFirstFreeControllerPort();

protected:
	void ProcessMessage(NetMessage* message) override;
	
public:
	GameServerConnection(shared_ptr<Console> console, shared_ptr<Socket> socket, string serverPassword);
	virtual ~GameServerConnection();

	ControlDeviceState GetState();
	void SendMovieData(uint8_t port, ControlDeviceState state);

	string GetPlayerName();
	uint8_t GetControllerPort();

	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) override;

	static GameServerConnection* GetNetPlayDevice(uint8_t port);
};
