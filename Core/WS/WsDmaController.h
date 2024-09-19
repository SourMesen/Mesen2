#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"

class WsMemoryManager;
class WsApu;

class WsDmaController final : public ISerializable
{
private:
	WsMemoryManager* _memoryManager = nullptr;
	WsDmaControllerState _state = {};
	WsApu* _apu = nullptr;

public:
	void Init(WsMemoryManager* memoryManager, WsApu* apu);
	
	void RunGeneralDma();
	void ProcessSoundDma();

	WsDmaControllerState& GetState() { return _state; }

	uint8_t ReadPort(uint16_t port);
	void WritePort(uint16_t port, uint8_t value);

	void Serialize(Serializer& s) override;
};
