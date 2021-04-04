#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"

class Socket;
class NetMessage;
class Emulator;

struct PlayerInfo
{
	string Name;
	uint8_t ControllerPort;
	bool IsHost;
};

class GameConnection
{
protected:
	static constexpr int MaxMsgLength = 1500000;

	shared_ptr<Socket> _socket;
	shared_ptr<Emulator> _emu;

	uint8_t _readBuffer[GameConnection::MaxMsgLength] = {};
	uint8_t _messageBuffer[GameConnection::MaxMsgLength] = {};
	int _readPosition = 0;
	SimpleLock _socketLock;

private:

	void ReadSocket();

	bool ExtractMessage(void *buffer, uint32_t &messageLength);
	NetMessage* ReadMessage();

	virtual void ProcessMessage(NetMessage* message) = 0;

protected:
	void Disconnect();

public:
	static constexpr uint8_t SpectatorPort = 0xFF;
	GameConnection(shared_ptr<Emulator> emu, shared_ptr<Socket> socket);

	bool ConnectionError();
	void ProcessMessages();
	void SendNetMessage(NetMessage &message);
};