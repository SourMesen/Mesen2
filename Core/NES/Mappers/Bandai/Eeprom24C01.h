#pragma once
#include "pch.h"
#include "NES/Mappers/Bandai/BaseEeprom24C0X.h"
#include "NES/NesConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BatteryManager.h"

class Eeprom24C01 : public BaseEeprom24C0X
{
private:
	void WriteBit(uint8_t &dest, uint8_t value)
	{
		if(_counter < 8) {
			uint8_t mask = ~(1 << _counter);
			dest = (dest & mask) | (value << _counter);
			_counter++;
		}
	}

	void ReadBit()
	{
		if(_counter < 8) {
			_output = (_data & (1 << _counter)) ? 1 : 0;
			_counter++;
		}
	}

public:
	Eeprom24C01(NesConsole* console)
	{
		console->InitializeRam(_romData, 128);
		_emu = console->GetEmulator();
		_emu->GetBatteryManager()->LoadBattery(".eeprom128", _romData, 128);
	}

	void SaveBattery() override
	{
		_emu->GetBatteryManager()->SaveBattery(".eeprom128", _romData, 128);
	}

	void Write(uint8_t scl, uint8_t sda) override
	{
		if(_prevScl && scl && sda < _prevSda) {
			//"START is identified by a high to low transition of the SDA line while the clock SCL is *stable* in the high state"
			_mode = Mode::Address;
			_address = 0;
			_counter = 0;
			_output = 1;
		} else if(_prevScl && scl && sda > _prevSda) {
			//"STOP is identified by a low to high transition of the SDA line while the clock SCL is *stable* in the high state"
			_mode = Mode::Idle;
			_output = 1;
		} else if(scl > _prevScl) {
			//Clock rise
			switch(_mode) {
				case Mode::Address:
					//To initiate a write operation, the master sends a start condition followed by a seven bit word address and a write bit.
					if(_counter < 7) {
						WriteBit(_address, sda);
					} else if(_counter == 7) {
						//8th bit to determine if we're in read or write mode
						_counter = 8;

						if(sda) {
							//Read mode
							_nextMode = Mode::Read;
							_data = _romData[_address & 0x7F];
						} else {
							_nextMode = Mode::Write;
						}
					}
					break;
				case Mode::SendAck: _output = 0; break;
				case Mode::Read: ReadBit(); break;
				case Mode::Write: WriteBit(_data, sda); break;
				case Mode::WaitAck:
					if(!sda) {
						//We expected an ack, but received something else, return to idle mode
						_nextMode = Mode::Idle;
					}
					break;

				default: break;
			}
		} else if(scl < _prevScl) {
			//Clock fall
			switch(_mode) {
				case Mode::Address:
					if(_counter == 8) {
						//After receiving the address, "the X24C01 responds with an acknowledge, then waits for eight bits of data"
						_mode = Mode::SendAck;
						_output = 1;
					}
					break;

				case Mode::SendAck:
					//After sending an ack, move to the next mode of operation
					_mode = _nextMode;
					_counter = 0;
					_output = 1;
					break;

				case Mode::Read:
					if(_counter == 8) {
						//After sending all 8 bits, wait for an ack
						_mode = Mode::WaitAck;
						_address = (_address + 1) & 0x7F;
					}
					break;

				case Mode::Write:
					if(_counter == 8) {
						//After receiving all 8 bits, send an ack and then wait
						_mode = Mode::SendAck;
						_nextMode = Mode::Idle;
						_romData[_address & 0x7F] = _data;
						_address = (_address + 1) & 0x7F;
					}
					break;

				default: break;
			}
		}
		_prevScl = scl;
		_prevSda = sda;
	}
};
