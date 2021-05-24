#pragma once
#include "stdafx.h"
#include <deque>
#include "Netplay/GameConnection.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControlDeviceState.h"

class HandShakeMessage;

class GameServerConnection : public GameConnection, public INotificationListener
{
private:
	static GameServerConnection* _netPlayDevices[BaseControlDevice::PortCount];

	list<ControlDeviceState> _inputData;
	int _controllerPort = 0;
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
	GameServerConnection(shared_ptr<Emulator> emu, shared_ptr<Socket> socket, string serverPassword);
	virtual ~GameServerConnection();

	ControlDeviceState GetState();
	void SendMovieData(uint8_t port, ControlDeviceState state);

	uint8_t GetControllerPort();

	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) override;

	static GameServerConnection* GetNetPlayDevice(uint8_t port);
};
