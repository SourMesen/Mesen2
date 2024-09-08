#pragma once
#include "pch.h"
#include "Shared/MessageManager.h"

class MgbaLogHandler
{
private:
	uint8_t _enableMarker[2] = {};
	bool _enabled = false;
	uint8_t _message[257] = {};

	void UpdateEnableFlag()
	{
		_enabled |= _enableMarker[1] == 0xC0 && _enableMarker[0] == 0xDE;
	}

public:
	void Write(uint32_t addr, uint8_t value)
	{
		if(_enabled && addr < 0xFFF700) {
			_message[addr & 0xFF] = value;
		} else if(addr == 0xFFF780) {
			_enableMarker[0] = value;
			UpdateEnableFlag();
		} else if(addr == 0xFFF781) {
			_enableMarker[1] = value;
			UpdateEnableFlag();
		} else if(addr == 0xFFF701) {
			if(_enabled && (value & 0x01)) {
				if(_message[0] != 0) {
					MessageManager::Log("[Debug] " + string((char*)_message));
				}
				memset(_message, 0, sizeof(_message));
			}
		}
	}

	uint8_t Read(uint32_t addr)
	{
		switch(addr & 0x03) {
			case 0: return _enabled ? 0xEA : 0x00;
			case 1: return _enabled ? 0x1D : 0x00;
			default: return 0;
		}
	}
};