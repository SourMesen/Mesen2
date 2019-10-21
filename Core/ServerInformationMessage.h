#pragma once
#include "stdafx.h"
#include "NetMessage.h"

class ServerInformationMessage : public NetMessage
{
private:
	char* _hashSalt = nullptr;
	uint32_t _hashSaltLength = 0;

protected:
	virtual void ProtectedStreamState()
	{
		StreamArray((void**)&_hashSalt, _hashSaltLength);
	}

public:
	ServerInformationMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) {}
	ServerInformationMessage(string hashSalt) : NetMessage(MessageType::ServerInformation)
	{
		CopyString(&_hashSalt, _hashSaltLength, hashSalt);
	}

	string GetHashSalt()
	{
		return string(_hashSalt);
	}
};
