#pragma once
#include "stdafx.h"
#include <deque>
#include "Netplay/GameConnection.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControlDeviceState.h"

class HandShakeMessage;
class GameServer;

class GameServerConnection : public GameConnection, public INotificationListener
{
private:
	GameServer* _server;
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

protected:
	void ProcessMessage(NetMessage* message) override;
	
public:
	GameServerConnection(GameServer* gameServer, Emulator* emu, unique_ptr<Socket> socket, string serverPassword);
	virtual ~GameServerConnection();

	ControlDeviceState GetState();
	void SendMovieData(uint8_t port, ControlDeviceState state);

	uint8_t GetControllerPort();

	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};
