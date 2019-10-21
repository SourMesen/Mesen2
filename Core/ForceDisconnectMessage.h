#pragma once
#include "stdafx.h"
#include "MessageManager.h"
#include "NetMessage.h"
#include "Console.h"
#include "../Utilities/FolderUtilities.h"

class ForceDisconnectMessage : public NetMessage
{
private:
	char* _disconnectMessage = nullptr;
	uint32_t _messageLength = 0;

protected:
	virtual void ProtectedStreamState()
	{
		StreamArray((void**)&_disconnectMessage, _messageLength);
	}

public:
	ForceDisconnectMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	ForceDisconnectMessage(string message) : NetMessage(MessageType::ForceDisconnect)
	{
		CopyString(&_disconnectMessage, _messageLength, message);
	}

	string GetMessage()
	{
		return _disconnectMessage;
	}
};