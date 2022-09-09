#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"

class ServerInformationMessage : public NetMessage
{
private:
	string _hashSalt;

protected:
	void Serialize(Serializer &s) override
	{
		SV(_hashSalt);
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
