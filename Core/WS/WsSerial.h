#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"

class WsConsole;

class WsSerial final : public ISerializable
{
private:
	WsSerialState _state = {};
	WsConsole* _console = nullptr;

	void UpdateState();

public:
	WsSerial(WsConsole* console);

	WsSerialState& GetState() { return _state; }

	uint8_t Read(uint16_t port);
	void Write(uint16_t port, uint8_t value);

	bool HasSendIrq();

	void Serialize(Serializer& s) override;
};