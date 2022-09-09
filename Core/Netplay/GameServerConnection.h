#pragma once
#include "pch.h"
#include <deque>
#include "Netplay/GameConnection.h"
#include "Netplay/NetplayTypes.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControlDeviceState.h"
#include "Utilities/SimpleLock.h"

class HandShakeMessage;
class GameServer;

class GameServerConnection final : public GameConnection, public INotificationListener
{
private:
	GameServer* _server = nullptr;

	SimpleLock _inputLock;
	ControlDeviceState _inputData = {};

	string _previousConfig = "";

	NetplayControllerInfo _controllerPort = {};
	string _connectionHash;
	string _serverPassword;
	bool _handshakeCompleted = false;

	void PushState(ControlDeviceState state);
	void SendServerInformation();
	void SendGameInformation();
	void SelectControllerPort(NetplayControllerInfo port);

	void SendForceDisconnectMessage(string disconnectMessage);

	void ProcessHandshakeResponse(HandShakeMessage* message);

protected:
	void ProcessMessage(NetMessage* message) override;
	
public:
	GameServerConnection(GameServer* gameServer, Emulator* emu, unique_ptr<Socket> socket, string serverPassword);
	virtual ~GameServerConnection();

	ControlDeviceState GetState();
	void SendMovieData(uint8_t port, ControlDeviceState state);

	NetplayControllerInfo GetControllerPort();

	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};
