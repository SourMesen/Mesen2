#pragma once
#include "pch.h"
#include "Utilities/SimpleLock.h"

class Socket;
class NetMessage;
class Emulator;

class GameConnection
{
protected:
	static constexpr int MaxMsgLength = 1500000;

	unique_ptr<Socket> _socket;
	Emulator* _emu;

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
	GameConnection(Emulator* emu, unique_ptr<Socket> socket);
	virtual ~GameConnection();

	bool ConnectionError();
	void ProcessMessages();
	void SendNetMessage(NetMessage &message);
};