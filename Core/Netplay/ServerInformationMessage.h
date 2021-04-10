#pragma once
#include "stdafx.h"
#include "NetMessage.h"

class ServerInformationMessage : public NetMessage
{
private:
	string _hashSalt;

protected:
	void Serialize(Serializer &s) override
	{
		s.Stream(_hashSalt);
	}

public:
	ServerInformationMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) {}
	ServerInformationMessage(string hashSalt) : NetMessage(MessageType::ServerInformation)
	{
		_hashSalt = hashSalt;
	}

	string GetHashSalt()
	{
		return _hashSalt;
	}
};
