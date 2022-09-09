#pragma once
#include "pch.h"
#include <thread>
#include "Netplay/GameServerConnection.h"
#include "Netplay/NetplayTypes.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"
#include "Shared/IControllerHub.h"

class Emulator;

class GameServer : public IInputRecorder, public IInputProvider, public INotificationListener, public std::enable_shared_from_this<GameServer>
{
private:
	Emulator* _emu;
	unique_ptr<thread> _serverThread;
	unique_ptr<Socket> _listener;
	atomic<bool> _stop;
	uint16_t _port = 0;
	string _password;
	vector<unique_ptr<GameServerConnection>> _openConnections;
	bool _initialized = false;
	
	GameServerConnection* _netPlayDevices[BaseControlDevice::PortCount][IControllerHub::MaxSubPorts] = {};

	NetplayControllerInfo _hostControllerPort = {};

	void AcceptConnections();
	void UpdateConnections();

	void Exec();

public:
	GameServer(Emulator* emu);
	virtual ~GameServer();

	void RegisterServerInput();

	void StartServer(uint16_t port, string password);
	void StopServer();
	bool Started();

	NetplayControllerInfo GetHostControllerPort();
	void SetHostControllerPort(NetplayControllerInfo controller);
	vector<NetplayControllerUsageInfo> GetControllerList();
	vector<PlayerInfo> GetPlayerList();
	void SendPlayerList();
	
	static vector<NetplayControllerUsageInfo> GetControllerList(Emulator* emu, vector<PlayerInfo>& players);

	bool SetInput(BaseControlDevice *device) override;
	void RecordInput(vector<shared_ptr<BaseControlDevice>> devices) override;

	// Inherited via INotificationListener
	virtual void ProcessNotification(ConsoleNotificationType type, void * parameter) override;

	void RegisterNetPlayDevice(GameServerConnection* connection, NetplayControllerInfo controller);
	void UnregisterNetPlayDevice(GameServerConnection* device);
	NetplayControllerInfo GetFirstFreeControllerPort();
	GameServerConnection* GetNetPlayDevice(NetplayControllerInfo controller);
};