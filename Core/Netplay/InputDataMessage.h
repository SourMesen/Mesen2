#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"
#include "Shared/ControlDeviceState.h"

class InputDataMessage : public NetMessage
{
private:
	ControlDeviceState _inputState;

protected:	
	void Serialize(Serializer &s) override
	{
		SVVector(_inputState.State);
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