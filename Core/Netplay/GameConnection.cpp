#include "pch.h"
#include "Netplay/GameConnection.h"
#include "Netplay/HandShakeMessage.h"
#include "Netplay/InputDataMessage.h"
#include "Netplay/MovieDataMessage.h"
#include "Netplay/GameInformationMessage.h"
#include "Netplay/SaveStateMessage.h"
#include "Netplay/PlayerListMessage.h"
#include "Netplay/SelectControllerMessage.h"
#include "Netplay/ClientConnectionData.h"
#include "Netplay/ForceDisconnectMessage.h"
#include "Netplay/ServerInformationMessage.h"

GameConnection::GameConnection(Emulator* emu, unique_ptr<Socket> socket)
{
	_emu = emu;
	_socket.swap(socket);
}

GameConnection::~GameConnection()
{
	Disconnect();
}

void GameConnection::ReadSocket()
{
	auto lock = _socketLock.AcquireSafe();
	int bytesReceived = _socket->Recv((char*)_readBuffer + _readPosition, GameConnection::MaxMsgLength - _readPosition, 0);
	if(bytesReceived > 0) {
		_readPosition += bytesReceived;
	}
}

bool GameConnection::ExtractMessage(void *buffer, uint32_t &messageLength)
{
	messageLength = _readBuffer[0] | (_readBuffer[1] << 8) | (_readBuffer[2] << 16) | (_readBuffer[3] << 24);

	if(messageLength > GameConnection::MaxMsgLength) {
		MessageManager::Log("[Netplay] Invalid data received, closing connection.");
		Disconnect();
		return false;
	}

	int packetLength = messageLength + sizeof(messageLength);

	if(_readPosition >= packetLength) {
		memcpy(buffer, _readBuffer+sizeof(messageLength), messageLength);
		memmove(_readBuffer, _readBuffer + packetLength, _readPosition - packetLength);
		_readPosition -= packetLength;
		return true;
	}
	return false;
}

NetMessage* GameConnection::ReadMessage()
{
	ReadSocket();

	if(_readPosition > 4) {
		uint32_t messageLength;
		if(ExtractMessage(_messageBuffer, messageLength)) {
			switch((MessageType)_messageBuffer[0]) {
				case MessageType::HandShake: return new HandShakeMessage(_messageBuffer, messageLength);
				case MessageType::SaveState: return new SaveStateMessage(_messageBuffer, messageLength);
				case MessageType::InputData: return new InputDataMessage(_messageBuffer, messageLength);
				case MessageType::MovieData: return new MovieDataMessage(_messageBuffer, messageLength);
				case MessageType::GameInformation: return new GameInformationMessage(_messageBuffer, messageLength);
				case MessageType::PlayerList: return new PlayerListMessage(_messageBuffer, messageLength);
				case MessageType::SelectController: return new SelectControllerMessage(_messageBuffer, messageLength);
				case MessageType::ForceDisconnect: return new ForceDisconnectMessage(_messageBuffer, messageLength);
				case MessageType::ServerInformation: return new ServerInformationMessage(_messageBuffer, messageLength);
			}
		}
	}
	return nullptr;
}

void GameConnection::SendNetMessage(NetMessage &message)
{
	auto lock = _socketLock.AcquireSafe();
	message.Send(*_socket.get());
}

void GameConnection::Disconnect()
{
	auto lock = _socketLock.AcquireSafe();
	_socket->Close();
}

bool GameConnection::ConnectionError()
{
	return _socket->ConnectionError();
}

void GameConnection::ProcessMessages()
{
	NetMessage* message;
	while((message = ReadMessage()) != nullptr) {
		//Loop until all messages have been processed
		message->Initialize();
		ProcessMessage(message);
		delete message;
	}		
}