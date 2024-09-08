#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

enum class GbaEepromMode
{
	Idle,
	Command,
	ReadCommand,
	ReadDataReady,
	WriteCommand,
};

class GbaEeprom final : public ISerializable
{
private:
	uint8_t* _saveRam = nullptr;

	uint8_t _addressSize = 0;
	uint16_t _maxAddress = 0x3FF;

	GbaEepromMode _mode = GbaEepromMode::Idle;

	uint32_t _address = 0;
	uint32_t _len = 0;
	uint32_t _counter = 0;
	uint64_t _writeData = 0;

public:
	GbaEeprom(uint8_t* saveRam, GbaSaveType saveType)
	{
		if(saveType == GbaSaveType::Eeprom512) {
			_addressSize = 6;
			_maxAddress = 0x3F;
		} else if(saveType == GbaSaveType::Eeprom8192) {
			_addressSize = 14;
			_maxAddress = 0x3FF;
		}
		_saveRam = saveRam;
	}

	__noinline uint8_t Read()
	{
		switch(_mode) {
			case GbaEepromMode::ReadCommand:
				//Auto-detect EEPROM size and then start reading
				if(_addressSize != 0) {
					return 1;
				}

				_addressSize = _len == 7 ? 6 : 14;
				_maxAddress = _addressSize == 6 ? 0x3F : 0x3FF;
				_address >>= 1;
				_counter = 0;
				_mode = GbaEepromMode::ReadDataReady;
				[[fallthrough]];

			case GbaEepromMode::ReadDataReady:
				if(++_counter > 4) {
					uint8_t value;
					if(_address <= _maxAddress) {
						uint8_t offset = (68 - _counter);
						value = (_saveRam[(_address << 3) + (offset >> 3)] >> (offset & 0x07)) & 0x01;
					} else {
						value = 1;
					}
					if(_counter >= 68) {
						_mode = GbaEepromMode::Idle;
					}
					return value;
				}
				return 1;

			default:
				return 1;
		}
	}

	__noinline void Write(uint8_t value)
	{
		uint8_t bit = value & 0x01;

		switch(_mode) {
			case GbaEepromMode::Idle:
				if(bit) {
					_mode = GbaEepromMode::Command;
				}
				break;

			case GbaEepromMode::Command:
				_address = 0;
				_len = 0;
				_counter = 0;
				_mode = bit ? GbaEepromMode::ReadCommand : GbaEepromMode::WriteCommand;
				break;

			case GbaEepromMode::ReadCommand:
				if(_addressSize && _len == _addressSize){
					_mode = GbaEepromMode::ReadDataReady;
					_counter = 0;
				} else {
					_len++;
					_address = (_address << 1) | bit;
				}
				break;

			case GbaEepromMode::WriteCommand:
				if(_addressSize == 0) {
					_mode = GbaEepromMode::Idle;
					return;
				}

				if(++_len > _addressSize) {
					if(_counter < 64) {
						_writeData = (_writeData << 1) | bit;
						_counter++;
					} else {
						if(_address <= _maxAddress) {
							for(int i = 0; i < 8; i++) {
								_saveRam[(_address << 3) + i] = _writeData & 0xFF;
								_writeData >>= 8;
							}
						}
						_mode = GbaEepromMode::Idle;
					}
				} else {
					_address = (_address << 1) | bit;
				}
				break;
		}
	}

	uint32_t GetSaveSize()
	{
		return (_maxAddress + 1) * 8;
	}

	void Serialize(Serializer& s)
	{
		SV(_addressSize);
		SV(_maxAddress);

		SV(_address);
		SV(_len);
		SV(_counter);
		SV(_writeData);
		SV(_mode);
	}
};