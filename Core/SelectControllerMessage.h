#pragma once
#include "stdafx.h"
#include "NetMessage.h"

class SelectControllerMessage : public NetMessage
{
private:
	uint8_t _portNumber;

protected:
	virtual void ProtectedStreamState()
	{
		Stream<uint8_t>(_portNumber);
	}

public:
	SelectControllerMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	SelectControllerMessage(uint8_t port) : NetMessage(MessageType::SelectController)
	{
		_portNumber = port;
	}

	uint8_t GetPortNumber()
	{
		return _portNumber;
	}
};