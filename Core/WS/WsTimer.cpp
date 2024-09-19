#include "pch.h"
#include "WS/WsTimer.h"
#include "WS/WsMemoryManager.h"
#include "Utilities/BitUtilities.h"

void WsTimer::Init(WsMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
}

void WsTimer::TickHorizontalTimer()
{
	if(_state.HTimer == 1) {
		_memoryManager->SetIrqSource(WsIrqSource::HorizontalBlankTimer);
	}

	if(_state.HBlankEnabled) {
		if(_state.HTimer && !--_state.HTimer && _state.HBlankAutoReload) {
			_state.HTimer = _state.HReloadValue;
		}
	}
}

void WsTimer::TickVerticalTimer()
{
	if(_state.VTimer == 1) {
		_memoryManager->SetIrqSource(WsIrqSource::VerticalBlankTimer);
	}

	if(_state.VBlankEnabled) {
		if(_state.VTimer && !--_state.VTimer && _state.VBlankAutoReload) {
			_state.VTimer = _state.VReloadValue;
		}
	}
}

uint8_t WsTimer::ReadPort(uint16_t port)
{
	switch(port) {
		case 0xA2: return _state.Control;
		case 0xA4: return BitUtilities::GetBits<0>(_state.HReloadValue); break;
		case 0xA5: return BitUtilities::GetBits<8>(_state.HReloadValue); break;
		case 0xA6: return BitUtilities::GetBits<0>(_state.VReloadValue); break;
		case 0xA7: return BitUtilities::GetBits<8>(_state.VReloadValue); break;
		case 0xA8: return BitUtilities::GetBits<0>(_state.HTimer); break;
		case 0xA9: return BitUtilities::GetBits<8>(_state.HTimer); break;
		case 0xAA: return BitUtilities::GetBits<0>(_state.VTimer); break;
		case 0xAB: return BitUtilities::GetBits<8>(_state.VTimer); break;
	}

	return 0;
}

void WsTimer::WritePort(uint16_t port, uint8_t value)
{
	switch(port) {
		case 0xA2:
			_state.Control = value & 0x0F;
			_state.HBlankEnabled = value & 0x01;
			_state.HBlankAutoReload = value & 0x02;
			_state.VBlankEnabled = value & 0x04;
			_state.VBlankAutoReload = value & 0x08;
			break;

		case 0xA4:
			BitUtilities::SetBits<0>(_state.HReloadValue, value);
			_state.HTimer = _state.HReloadValue;
			break;

		case 0xA5:
			BitUtilities::SetBits<8>(_state.HReloadValue, value);
			_state.HTimer = _state.HReloadValue;
			break;

		case 0xA6:
			BitUtilities::SetBits<0>(_state.VReloadValue, value);
			_state.VTimer = _state.VReloadValue;
			break;

		case 0xA7:
			BitUtilities::SetBits<8>(_state.VReloadValue, value);
			_state.VTimer = _state.VReloadValue;
			break;
	}
}

void WsTimer::Serialize(Serializer& s)
{
	SV(_state.HTimer);
	SV(_state.VTimer);
	SV(_state.HReloadValue);
	SV(_state.VReloadValue);
	SV(_state.Control);
	SV(_state.HBlankEnabled);
	SV(_state.HBlankAutoReload);
	SV(_state.VBlankEnabled);
	SV(_state.VBlankAutoReload);
}
