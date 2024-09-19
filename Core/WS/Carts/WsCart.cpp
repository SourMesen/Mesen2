#include "pch.h"
#include "WS/Carts/WsCart.h"
#include "WS/WsMemoryManager.h"
#include "WS/WsEeprom.h"
#include "Utilities/Serializer.h"

void WsCart::Map(uint32_t start, uint32_t end, MemoryType type, uint32_t offset, bool readonly)
{
	_memoryManager->Map(start, end, type, offset, readonly);
}

void WsCart::Unmap(uint32_t start, uint32_t end)
{
	_memoryManager->Unmap(start, end);
}

WsCart::WsCart()
{
	_state.SelectedBanks[0] = 0xFF;
	_state.SelectedBanks[1] = 0xFF;
	_state.SelectedBanks[2] = 0xFF;
	_state.SelectedBanks[3] = 0xFF;
}

void WsCart::Init(WsMemoryManager* memoryManager, WsEeprom* cartEeprom)
{
	_memoryManager = memoryManager;
	_cartEeprom = cartEeprom;
}

void WsCart::RefreshMappings()
{
	Map(0x10000, 0x1FFFF, MemoryType::WsCartRam, _state.SelectedBanks[1] * 0x10000, false);
	Map(0x20000, 0x2FFFF, MemoryType::WsPrgRom, _state.SelectedBanks[2] * 0x10000, true);
	Map(0x30000, 0x3FFFF, MemoryType::WsPrgRom, _state.SelectedBanks[3] * 0x10000, true);
	Map(0x40000, 0xFFFFF, MemoryType::WsPrgRom, _state.SelectedBanks[0] * 0x100000 + 0x40000, true);
}

uint8_t WsCart::ReadPort(uint16_t port)
{
	if(port < 0xC4) {
		return _state.SelectedBanks[port - 0xC0];
	} else if(port < 0xC9 && _cartEeprom) {
		return _cartEeprom->ReadPort(port - 0xC4);
	}

	//TODOWS open bus
	return 0x90;
}

void WsCart::WritePort(uint16_t port, uint8_t value)
{
	if(port < 0xC4) {
		_state.SelectedBanks[port - 0xC0] = value;
		_memoryManager->RefreshMappings();
	} else if(port < 0xC9 && _cartEeprom) {
		_cartEeprom->WritePort(port - 0xC4, value);
	}
}

void WsCart::Serialize(Serializer& s)
{
	SVArray(_state.SelectedBanks, 4);
}
