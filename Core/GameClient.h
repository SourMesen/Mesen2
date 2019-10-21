#pragma once
#include "stdafx.h"
#include <thread>
#include "INotificationListener.h"

using std::thread;
class Socket;
class GameClientConnection;
class ClientConnectionData;
class Console;

class GameClient : public INotificationListener
{
private:
	static shared_ptr<GameClient> _instance;

	shared_ptr<Console> _console;
	unique_ptr<thread> _clientThread;
	atomic<bool> _stop;

	shared_ptr<GameClientConnection> _connection;
	bool _connected = false;

	static shared_ptr<GameClientConnection> GetConnection();

	void PrivateConnect(ClientConnectionData &connectionData);
	void Exec();

public:
	GameClient(shared_ptr<Console> console);
	virtual ~GameClient();

	static bool Connected();
	static void Connect(shared_ptr<Console> console, ClientConnectionData &connectionData);
	static void Disconnect();

	static void SelectController(uint8_t port);
	static uint8_t GetControllerPort();
	static uint8_t GetAvailableControllers();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};