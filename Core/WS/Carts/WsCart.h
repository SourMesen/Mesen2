#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"
#include "Shared/MemoryType.h"

class WsMemoryManager;
class WsEeprom;

//TODOWS RTC
//TODOWS Flash

class WsCart final: public ISerializable
{
protected:
	WsCartState _state = {};

	WsMemoryManager* _memoryManager = nullptr;
	WsEeprom* _cartEeprom = nullptr;
	
	void Map(uint32_t start, uint32_t end, MemoryType type, uint32_t offset, bool readonly);
	void Unmap(uint32_t start, uint32_t end);

public:
	WsCart();
	virtual ~WsCart() {}

	void Init(WsMemoryManager* memoryManager, WsEeprom* cartEeprom);
	void RefreshMappings();
	
	WsCartState& GetState() { return _state; }

	virtual uint8_t ReadPort(uint16_t port);
	virtual void WritePort(uint16_t port, uint8_t value);

	void Serialize(Serializer& s) override;
};
