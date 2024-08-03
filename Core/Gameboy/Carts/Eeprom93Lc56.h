#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class Eeprom93Lc56 final : public ISerializable
{
private:
	enum class Mode
	{
		Idle,
		Command,
		ReadCommand,
		WriteCommand,
		WriteAllCommand
	};

	uint8_t* _saveRam = nullptr;

	Mode _mode = Mode::Idle;

	bool _chipSelect = false;
	bool _clk = false;
	uint8_t _dataIn = 0;
	bool _writeEnabled = false;

	uint16_t _commandId = 0;
	uint8_t _bitCount = 0;

	uint8_t _readAddress = 0;
	int8_t _readCounter = -1;

	uint16_t _writeData = 0;
	uint8_t _writeCounter = 0;

public:
	void SetRam(uint8_t* saveRam)
	{
		_saveRam = saveRam;
	}

	uint8_t Read()
	{
		uint8_t result = (
			(_chipSelect ? 0x80 : 0) |
			(_clk ? 0x40 : 0) |
			(_dataIn ? 0x02 : 0)
		);

		if(_mode == Mode::ReadCommand) {
			if(_readCounter < 0) {
				return result | 0x01;
			}
			return result | ((_saveRam[(_readAddress << 1) + (_readCounter >= 8 ? 0 : 1)] >> (7 - (_readCounter & 0x07))) & 0x01);
		} else {
			return result | (_mode == Mode::Idle ? 1 : 0);
		}
	}

	void Write(uint8_t value)
	{
		uint8_t prevClk = _clk;
		bool clk = (value & 0x40);
		_dataIn = (value & 0x02) >> 1;
		_chipSelect = value & 0x80;
		_clk = clk;

		if(!_chipSelect) {
			//Chip select is disabled, force idle
			_mode = Mode::Idle;
			return;
		}


		if(!clk || prevClk) {
			//Not a rising clock
			return;
		}

		switch(_mode) {
			case Mode::Idle:
				if(_dataIn) {
					_mode = Mode::Command;
					_commandId = 0;
					_bitCount = 0;
					_writeCounter = 0;
					_writeData = 0;
					_readCounter = -1;
					_readAddress = 0;
				}
				break;

			case Mode::Command:
				_commandId <<= 1;
				_commandId |= _dataIn;
				_bitCount++;

				if(_bitCount >= 10) {
					uint8_t id = _commandId >> 6;
					if((id & 0b1100) == 0b1000) {
						_mode = Mode::ReadCommand;
						_readAddress = _commandId & 0x7F;
					} else if((id & 0b1100) == 0b0100) {
						_mode = Mode::WriteCommand;
					} else if((id & 0b1100) == 0b1100) {
						if(_writeEnabled) {
							uint8_t addr = _commandId & 0x7F;
							_saveRam[addr << 1] = 0xFF;
							_saveRam[(addr << 1) + 1] = 0xFF;
						}
						_mode = Mode::Idle;
					} else if((id & 0b1111) == 0b0000) {
						//Disable writes (EWDS)
						_writeEnabled = false;
						_mode = Mode::Idle;
					} else if((id & 0b1111) == 0b0011) {
						//Enable writes (EWEN)
						_writeEnabled = true;
						_mode = Mode::Idle;
					} else if((id & 0b1111) == 0b0010) {
						//Erase all (ERAL)
						if(_writeEnabled) {
							memset(_saveRam, 0xFF, 256);
						}
						_mode = Mode::Idle;
					} else if((id & 0b1111) == 0b0001) {
						//Write all (WRAL)
						_mode = Mode::WriteAllCommand;
					}
				}
				break;

			case Mode::ReadCommand:
				_readCounter++;
				if(_readCounter == 16) {
					_readAddress = (_readAddress + 1) & 0x7F;
					_readCounter = 0;
					_mode = Mode::Idle;
				}
				break;

			case Mode::WriteAllCommand:
			case Mode::WriteCommand: {
				_writeData <<= 1;
				_writeData |= _dataIn;
				_writeCounter++;
				if(_writeCounter >= 16) {
					if(_writeEnabled) {
						if(_mode == Mode::WriteAllCommand) {
							for(int i = 0; i < 0x80; i++) {
								_saveRam[i << 1] = _writeData;
								_saveRam[(i << 1) + 1] = _writeData >> 8;
							}
						} else {
							uint8_t addr = _commandId & 0x7F;
							_saveRam[addr << 1] = _writeData;
							_saveRam[(addr << 1) + 1] = _writeData >> 8;
						}
					}
					_mode = Mode::Idle;
				}
				break;
			}
		}
	}

	void Serialize(Serializer& s)
	{
		SV(_mode);
		SV(_chipSelect);
		SV(_clk);
		SV(_dataIn);
		SV(_writeEnabled);
		SV(_commandId);
		SV(_bitCount);
		SV(_readAddress);
		SV(_readCounter);
		SV(_writeData);
		SV(_writeCounter);
	}
};