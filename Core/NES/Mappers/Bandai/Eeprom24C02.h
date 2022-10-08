#pragma once
#include "pch.h"
#include "NES/Mappers/Bandai/BaseEeprom24C0X.h"
#include "NES/NesConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BatteryManager.h"

class Eeprom24C02 : public BaseEeprom24C0X
{
private:
	void WriteBit(uint8_t &dest, uint8_t value)
	{
		if(_counter < 8) {
			uint8_t mask = ~(1 << (7 - _counter));
			dest = (dest & mask) | (value << (7 - _counter));
			_counter++;
		}
	}

	void ReadBit()
	{
		if(_counter < 8) {
			_output = (_data & (1 << (7 - _counter))) ? 1 : 0;
			_counter++;
		}
	}

public:
	Eeprom24C02(NesConsole* console)
	{
		console->InitializeRam(_romData, 256);
		_emu = console->GetEmulator();
		_emu->GetBatteryManager()->LoadBattery(".eeprom256", _romData, 256);
	}

	void SaveBattery() override
	{
		_emu->GetBatteryManager()->SaveBattery(".eeprom256", _romData, 256);
	}

	void Write(uint8_t scl, uint8_t sda) override
	{
		if(_prevScl && scl && sda < _prevSda) {
			//"START is identified by a high to low transition of the SDA line while the clock SCL is *stable* in the high state"
			_mode = Mode::ChipAddress;
			_counter = 0;
			_output = 1;
		} else if(_prevScl && scl && sda > _prevSda) {
			//"STOP is identified by a low to high transition of the SDA line while the clock SCL is *stable* in the high state"
			_mode = Mode::Idle;
			_output = 1;
		} else if(scl > _prevScl) {
			//Clock rise
			switch(_mode) {
				default: break;
				case Mode::ChipAddress: WriteBit(_chipAddress, sda); break;
				case Mode::Address: WriteBit(_address, sda); break;
				case Mode::Read: ReadBit(); break;
				case Mode::Write: WriteBit(_data, sda); break;
				case Mode::SendAck: _output = 0; break;
				case Mode::WaitAck:
					if(!sda) {
						_nextMode = Mode::Read;
						_data = _romData[_address];
					}
					break;
			}
		} else if(scl < _prevScl) {
			//Clock fall
			switch(_mode) {
				case Mode::ChipAddress:
					//"Upon a correct compare the X24C02 outputs an acknowledge on the SDA line"
					if(_counter == 8) {
						if((_chipAddress & 0xA0) == 0xA0) {
							_mode = Mode::SendAck;
							_counter = 0;
							_output = 1;

							//"The last bit of the slave address defines the operation to
							//be performed. When set to one a read operation is
							//selected, when set to zero a write operations is selected"
							if(_chipAddress & 0x01) {
								//"Current Address Read"
								//"Upon receipt of the slave address with the R/W
								//bit set to one, the X24C02 issues an acknowledge 
								//and transmits the eight bit word during the next eight clock cycles"
								_nextMode = Mode::Read;
								_data = _romData[_address];
							} else {
								_nextMode = Mode::Address;
							}
						} else {
							//This chip wasn't selected, go back to idle mode
							_mode = Mode::Idle;
							_counter = 0;
							_output = 1;
						}
					}
					break;

				case Mode::Address:
					if(_counter == 8) {
						//Finished receiving all 8 bits of the address, send an ack and then starting writing the value
						_counter = 0;
						_mode = Mode::SendAck;
						_nextMode = Mode::Write;
						_output = 1;
					}
					break;

				case Mode::Read:
					if(_counter == 8) {
						//Finished sending all 8 bits, wait for an ack
						_mode = Mode::WaitAck;
						_address = (_address + 1) & 0xFF;
					}
					break;

				case Mode::Write:
					if(_counter == 8) {
						//Finished receiving all 8 bits, send an ack
						_counter = 0;
						_mode = Mode::SendAck;
						_nextMode = Mode::Write;
						_romData[_address] = _data;
						_address = (_address + 1) & 0xFF;
					}
					break;

				case Mode::SendAck:
				case Mode::WaitAck:
					_mode = _nextMode;
					_counter = 0;
					_output = 1;
					break;

				default:
					break;
			}
		}
		_prevScl = scl;
		_prevSda = sda;
	}
};
