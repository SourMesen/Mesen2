#pragma once
#include "stdafx.h"
#include "NetMessage.h"
#include "ControlDeviceState.h"

class InputDataMessage : public NetMessage
{
private:
	ControlDeviceState _inputState;

protected:	
	virtual void ProtectedStreamState()
	{
		StreamArray(_inputState.State);
	}

public:
	InputDataMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	InputDataMessage(ControlDeviceState inputState) : NetMessage(MessageType::InputData)
	{
		_inputState = inputState;
	}

	ControlDeviceState GetInputState()
	{
		return _inputState;
	}
};