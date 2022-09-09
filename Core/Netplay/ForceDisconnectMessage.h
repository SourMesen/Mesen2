#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"
#include "Shared/MessageManager.h"
#include "Utilities/FolderUtilities.h"

class ForceDisconnectMessage : public NetMessage
{
private:
	string _disconnectMessage;

protected:
	void Serialize(Serializer &s) override
	{
		SV(_disconnectMessage);
	}

public:
	ForceDisconnectMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	ForceDisconnectMessage(string message) : NetMessage(MessageType::ForceDisconnect)
	{
		_disconnectMessage = message;
	}

	string GetMessage()
	{
		return _disconnectMessage;
	}
};