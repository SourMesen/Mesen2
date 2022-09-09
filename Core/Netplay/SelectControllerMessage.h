#pragma once
#include "pch.h"
#include "Netplay/NetMessage.h"

class SelectControllerMessage : public NetMessage
{
private:
	NetplayControllerInfo _controller = {};

protected:
	void Serialize(Serializer &s) override
	{
		SV(_controller.Port);
		SV(_controller.SubPort);
	}

public:
	SelectControllerMessage(void* buffer, uint32_t length) : NetMessage(buffer, length) { }

	SelectControllerMessage(NetplayControllerInfo controller) : NetMessage(MessageType::SelectController)
	{
		_controller = controller;
	}

	NetplayControllerInfo GetController()
	{
		return _controller;
	}
};