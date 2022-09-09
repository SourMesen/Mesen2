#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

//SST39SF040 chip emulation - used by mapper 30 (UNROM512) & mapper 111 (GTROM)
class FlashSST39SF040 final : public ISerializable
{
private:
	enum class ChipMode
	{
		WaitingForCommand,
		Write,
		Erase
	};

	ChipMode _mode = ChipMode::WaitingForCommand;
	uint8_t _cycle = 0;
	uint8_t _softwareId = false;

	//PRG data and size
	uint8_t* _data = nullptr;
	uint32_t _size = 0;

protected:
	void Serialize(Serializer& s)
	{
		SV(_mode);
		SV(_cycle);
		SV(_softwareId);
	}

public:
	FlashSST39SF040(uint8_t* data, uint32_t size)
	{
		_data = data;
		_size = size;
	}

	int16_t Read(uint32_t addr)
	{
		if(_softwareId) {
			switch(addr & 0x1FF) {
				case 0x00: return 0xBF;
				case 0x01: return 0xB7;
				default: return 0xFF;
			}
		}
		return -1;
	}

	void ResetState()
	{
		_mode = ChipMode::WaitingForCommand;
		_cycle = 0;
	}

	void Write(uint32_t addr, uint8_t value)
	{
		uint16_t cmd = addr & 0x7FFF;
		if(_mode == ChipMode::WaitingForCommand) {
			if(_cycle == 0) {
				if(cmd == 0x5555 && value == 0xAA) {
					//1st write, $5555 = $AA
					_cycle++;
				} else if(value == 0xF0) {
					//Software ID exit
					ResetState();
					_softwareId = false;
				}
			} else if(_cycle == 1 && cmd == 0x2AAA && value == 0x55) {
				//2nd write, $2AAA = $55
				_cycle++;
			} else if(_cycle == 2 && cmd == 0x5555) {
				//3rd write, determines command type
				_cycle++;
				switch(value) {
					case 0x80: _mode = ChipMode::Erase; break;
					case 0x90: ResetState();  _softwareId = true; break;
					case 0xA0: _mode = ChipMode::Write; break;
					case 0xF0: ResetState(); _softwareId = false; break;
				}
			} else {
				_cycle = 0;
			}
		} else if(_mode == ChipMode::Write) {
			//Write a single byte
			if(addr < _size) {
				_data[addr] &= value;
			}
			ResetState();
		} else if(_mode == ChipMode::Erase) {
			if(_cycle == 3) {
				//4th write for erase command, $5555 = $AA
				if(cmd == 0x5555 && value == 0xAA) {
					_cycle++;
				} else {
					ResetState();
				}
			} else if(_cycle == 4) {
				//5th write for erase command, $2AAA = $55
				if(cmd == 0x2AAA && value == 0x55) {
					_cycle++;
				} else {
					ResetState();
				}
			} else if(_cycle == 5) {
				if(cmd == 0x5555 && value == 0x10) {
					//Chip erase
					memset(_data, 0xFF, _size);
				} else if(value == 0x30) {
					//Sector erase
					uint32_t offset = (addr & 0x7F000);
					if(offset + 0x1000 <= _size) {
						memset(_data + offset, 0xFF, 0x1000);
					}
				}
				ResetState();
			}
		}
	}
};