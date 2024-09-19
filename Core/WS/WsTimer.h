#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"

class WsMemoryManager;

class WsTimer final : public ISerializable
{
private:
	WsMemoryManager* _memoryManager = nullptr;
	WsTimerState _state = {};

public:
	void Init(WsMemoryManager* memoryManager);
	WsTimerState& GetState() { return _state; }

	void TickHorizontalTimer();
	void TickVerticalTimer();

	uint8_t ReadPort(uint16_t port);
	void WritePort(uint16_t port, uint8_t value);

	void Serialize(Serializer& s) override;
};
