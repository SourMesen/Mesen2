#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"
#include "Shared/ControlDeviceState.h"

class MovieDataMessage : public NetMessage
{
private:
	uint8_t _portNumber = 0;
	ControlDeviceState _inputState = {};

protected:
	void Serialize(Serializer &s) override
	{
		SV(_portNumber);
		SVVector(_inputState.State);
	}

public:
	MovieDataMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	MovieDataMessage(ControlDeviceState state, uint8_t port) : NetMessage(MessageType::MovieData)
	{
		_portNumber = port;
		_inputState = state;
	}

	uint8_t GetPortNumber()
	{
		return _portNumber;
	}

	ControlDeviceState GetInputState()
	{
		return _inputState;
	}
};