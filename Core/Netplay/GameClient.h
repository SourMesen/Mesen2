#pragma once
#include "stdafx.h"
#include "Shared/Interfaces/INotificationListener.h"

class Socket;
class GameClientConnection;
class ClientConnectionData;
class Emulator;

class GameClient : public INotificationListener, public std::enable_shared_from_this<GameClient>
{
private:
	Emulator* _emu;
	unique_ptr<thread> _clientThread;
	unique_ptr<GameClientConnection> _connection;

	atomic<bool> _stop;
	atomic<bool> _connected;

	void Exec();

public:
	GameClient(Emulator* emu);
	virtual ~GameClient();

	bool Connected();
	void Connect(ClientConnectionData &connectionData);
	void Disconnect();

	void SelectController(uint8_t port);
	uint8_t GetControllerPort();
	uint8_t GetAvailableControllers();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};