#pragma once
#include "stdafx.h"
#include "NetMessage.h"
#include "ControlDeviceState.h"

class MovieDataMessage : public NetMessage
{
private:
	uint8_t _portNumber;
	ControlDeviceState _inputState;

protected:
	virtual void ProtectedStreamState()
	{
		Stream<uint8_t>(_portNumber);
		StreamArray(_inputState.State);
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