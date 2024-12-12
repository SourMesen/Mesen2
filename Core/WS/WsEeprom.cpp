#include "pch.h"
#include "WS/WsEeprom.h"
#include "WS/WsConsole.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"

WsEeprom::WsEeprom(Emulator* emu, WsConsole* console, WsEepromSize size, uint8_t* eepromData, bool isInternal)
{
	_emu = emu;
	_console = console;
	_isInternal = isInternal;
	_data = eepromData;
	_state.Size = size;
	_state.Idle = true;
	_state.ReadDone = true;
	_state.WriteDisabled = true;

	if(_isInternal) {
		InitInternalEepromData();
	}
}

WsEepromCommand WsEeprom::GetCommand()
{
	uint16_t command = _state.Command;
	if(GetSize() != WsEepromSize::Size128) {
		command >>= 4;
	}

	switch(command & 0xFFC0) {
		case 0x0180: return WsEepromCommand::Read;
		case 0x0140: return WsEepromCommand::Write;
		case 0x01C0: return WsEepromCommand::Erase;
		case 0x0100:
			switch((command >> 4) & 0x03) {
				case 0x00: return WsEepromCommand::WriteDisable;
				case 0x01: return WsEepromCommand::WriteAll;
				case 0x02: return WsEepromCommand::EraseAll;
				case 0x03: return WsEepromCommand::WriteEnable;
			}
	}

	return WsEepromCommand::Unknown;
}

WsEepromSize WsEeprom::GetSize()
{
	if(_isInternal && _state.Size == WsEepromSize::Size2kb && !_console->IsColorMode()) {
		//Process commands as is the eeprom was the monochrome model's 128-byte eeprom when color mode is disabled
		return WsEepromSize::Size128;
	}
	return _state.Size;
}

uint16_t WsEeprom::GetCommandAddress()
{
	switch(GetSize()) {
		case WsEepromSize::Size128: return _state.Command & 0x3F;
		case WsEepromSize::Size1kb: return _state.Command & 0x1FF;
		case WsEepromSize::Size2kb: return _state.Command & 0x3FF;
	}

	return 0;
}

void WsEeprom::WriteValue(uint16_t addr, uint16_t value)
{
	if(!_state.WriteDisabled && (!_state.InternalEepromWriteProtected || addr < 0x30)) {
		if(_isInternal) {
			_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsInternalEeprom, MemoryOperationType::Write>(addr << 1, value);
		} else {
			_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsCartEeprom, MemoryOperationType::Write>(addr << 1, value);
		}
		_data[addr << 1] = (uint8_t)value;
		_data[(addr << 1) + 1] = (value >> 8);
	}
}

string WsEeprom::ConvertToEepromString(string in)
{
	for(size_t i = 0; i < in.size(); i++) {
		in[i] = in[i] - 0x36;
	}
	return in;
}

void WsEeprom::InitInternalEepromData()
{
	memset(_data, 0, (uint32_t)_state.Size);

	string name;
	switch(_console->GetModel()) {
		case WsModel::Monochrome:
			name = "WONDERSWAN";
			break;

		case WsModel::Color:
		case WsModel::SwanCrystal:
			//Set volume to max by default
			_data[0x83] = 0x03;

			if(_console->GetModel() == WsModel::Color) {
				name = "WONDERSWANCOLOR";
			} else {
				name = "SWANCRYSTAL";
			}
			break;
	}

	memcpy(_data + 0x60, ConvertToEepromString(name).c_str(), name.size());
}

void WsEeprom::WritePort(uint8_t port, uint8_t value)
{
	switch(port) {
		case 0x00: BitUtilities::SetBits<0>(_state.WriteBuffer, value); break;
		case 0x01: BitUtilities::SetBits<8>(_state.WriteBuffer, value); break;
		case 0x02: BitUtilities::SetBits<0>(_state.Command, value); break;
		case 0x03: BitUtilities::SetBits<8>(_state.Command, value); break;

		case 0x04:
			bool read = value & 0x10;
			bool write = value & 0x20;
			bool other = value & 0x40;
			bool writeProtect = value & 0x80;
			if((uint8_t)read + (uint8_t)write + (uint8_t)other + (uint8_t)writeProtect > 1) {
				//Invalid operation, do nothing
				return;
			}

			//TODOWS abort (cart eeprom)
			if(writeProtect && _isInternal) {
				_state.InternalEepromWriteProtected = true;
				return;
			}

			WsEepromCommand cmd = GetCommand();
			uint16_t addr = GetCommandAddress();

			if(read) {
				if(cmd == WsEepromCommand::Read) {
					//TODOWS
					//For the internal eeprom, "ReadDone" flag should be cleared here, and set again after a small delay
					//On cartridge eeprom, the flag is bugged and not reset when a read starts
					//The read buffer should also only be set after a delay (e.g after the read is done), for both eeproms
					_state.ReadBuffer = _data[addr << 1] | (_data[(addr << 1) + 1] << 8);
					if(_isInternal) {
						_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsInternalEeprom, MemoryOperationType::Read>(addr << 1, _state.ReadBuffer);
					} else {
						_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsCartEeprom, MemoryOperationType::Read>(addr << 1, _state.ReadBuffer);
					}
					_state.ReadDone = true;
				}
			} else if(write) {
				_state.ReadDone = false;

				if(cmd == WsEepromCommand::Write) {
					WriteValue(addr, _state.WriteBuffer);
				} else if(cmd == WsEepromCommand::WriteAll) {
					for(int i = 0; i < (int)_state.Size; i += 2) {
						WriteValue(i >> 1, _state.WriteBuffer);
					}
				}
			} else if(other) {
				_state.ReadDone = false;

				switch(cmd) {
					case WsEepromCommand::Erase:
						WriteValue(addr, 0xFFFF);
						break;

					case WsEepromCommand::WriteDisable:
						_state.WriteDisabled = true;
						break;

					case WsEepromCommand::EraseAll:
						for(int i = 0; i < (int)_state.Size; i += 2) {
							WriteValue(i >> 1, 0xFFFF);
						}
						break;

					case WsEepromCommand::WriteEnable:
						_state.WriteDisabled = false;
						break;
				}
			}
			break;
	}
}

uint8_t WsEeprom::ReadPort(uint8_t port)
{
	switch(port) {
		case 0x00: return BitUtilities::GetBits<0>(_state.ReadBuffer);
		case 0x01: return BitUtilities::GetBits<8>(_state.ReadBuffer);
		case 0x02: return BitUtilities::GetBits<0>(_state.Command);
		case 0x03: return BitUtilities::GetBits<8>(_state.Command);

		case 0x04:
			return (
				(_state.ReadDone ? 0x01 : 0) |
				(_state.Idle ? 0x02 : 0) |
				(_state.InternalEepromWriteProtected ? 0x80 : 0)
			);
	}

	return 0;
}

void WsEeprom::LoadBattery()
{
	_emu->GetBatteryManager()->LoadBattery(_isInternal ? ".ieeprom" : ".eeprom", _data, (uint32_t)_state.Size);
}

void WsEeprom::SaveBattery()
{
	_emu->GetBatteryManager()->SaveBattery(_isInternal ? ".ieeprom" : ".eeprom", _data, (uint32_t)_state.Size);
}

void WsEeprom::Serialize(Serializer& s)
{
	SV(_state.Size);
	SV(_state.ReadBuffer);
	SV(_state.WriteBuffer);
	SV(_state.Command);
	SV(_state.Control);
	SV(_state.WriteDisabled);
	SV(_state.ReadDone);
	SV(_state.Idle);
	SV(_state.InternalEepromWriteProtected);
}
