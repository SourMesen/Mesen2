#pragma once
#include "pch.h"
#include "Shared/Emulator.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

class GbaFlash final : public ISerializable
{
private:
	enum class ChipMode
	{
		WaitingForCommand,
		Write,
		Erase,
		SetMemoryBank
	};
	
	Emulator* _emu = nullptr;

	ChipMode _mode = ChipMode::WaitingForCommand;
	uint8_t _cycle = 0;
	uint8_t _softwareId = false;

	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;

	uint32_t _selectedBank = 0;
	bool _allowBanking = false;

public:
	GbaFlash(Emulator* emu, uint8_t* saveRam, uint32_t saveRamSize)
	{
		_emu = emu;
		_saveRam = saveRam;
		_saveRamSize = saveRamSize;
		_allowBanking = saveRamSize >= 0x20000;
	}

	uint32_t GetSelectedBank()
	{
		return _selectedBank;
	}

	uint8_t Read(uint32_t addr)
	{
		if(_softwareId && (addr & 0x03) < 2) {
			if(addr & 0x01) {
				return _saveRamSize == 0x10000 ? 0x1B : 0x13;
			} else {
				return _saveRamSize == 0x10000 ? 0x32 : 0x62;
			}
		}

		return _saveRam[_selectedBank | (addr & 0xFFFF)];
	}

	void ResetState()
	{
		_mode = ChipMode::WaitingForCommand;
		_cycle = 0;
	}

	void Write(uint32_t addr, uint8_t value)
	{
		uint16_t cmd = addr & 0xFFFF;
		if(_mode == ChipMode::SetMemoryBank) {
			if(cmd == 0) {
				_selectedBank = (value & 0x01) << 16;
			}
			ResetState();
		}
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
					case 0x80:
						_emu->DebugLog("[Flash] 0x80 - Enter erase mode");
						_mode = ChipMode::Erase;
						break;

					case 0x90:
						_emu->DebugLog("[Flash] 0x90 - Enter software ID mode");
						ResetState();  _softwareId = true;
						break;

					case 0xA0:
						_emu->DebugLog("[Flash] 0xA0 - Enter write byte mode");
						_mode = ChipMode::Write;
						break;

					case 0xB0:
						if(_allowBanking) {
							//_emu->DebugLog("[Flash] 0xB0 - Set memory bank");
							_mode = ChipMode::SetMemoryBank;
						}
						break;

					case 0xF0:
						_emu->DebugLog("[Flash] 0xF0 - Exit software ID mode");
						ResetState();
						_softwareId = false;
						break;

					default:
						_emu->DebugLog("[Flash] 0x" + HexUtilities::ToHex(value) + " - Unknown command");
						break;
				}
			} else {
				_cycle = 0;
			}
		} else if(_mode == ChipMode::Write) {
			//Write a single byte
			_saveRam[_selectedBank | (addr & 0xFFFF)] &= value;
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
					_emu->DebugLog("[Flash] Chip erase");
					memset(_saveRam, 0xFF, _saveRamSize);
				} else if(value == 0x30) {
					//Sector erase
					uint32_t offset = _selectedBank | (addr & 0xF000);
					_emu->DebugLog("[Flash] Sector erase: " + HexUtilities::ToHex(offset));
					if(offset + 0x1000 <= _saveRamSize) {
						memset(_saveRam + offset, 0xFF, 0x1000);
					}
				}
				ResetState();
			}
		}
	}

	void Serialize(Serializer& s)
	{
		SV(_mode);
		SV(_cycle);
		SV(_softwareId);
		SV(_selectedBank);
	}
};