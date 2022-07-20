#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "Utilities/Serializer.h"

class MMC1 : public BaseMapper
{
	private:
		uint8_t _writeBuffer = 0;
		uint8_t _shiftCount = 0;

		bool _wramDisable = false;
		bool _chrMode = false;
		bool _prgMode = false;
		bool _slotSelect = false;

		uint8_t _chrReg0 = 0;
		uint8_t _chrReg1 = 0;
		uint8_t _prgReg = 0;

		uint64_t _lastWriteCycle = 0;
		
		bool _forceWramOn = false;
		uint16_t _lastChrReg = 0;
		
	private:
		void ResetBuffer()
		{
			_shiftCount = 0;
			_writeBuffer = 0;
		}

		void ProcessBitWrite(uint16_t addr, uint8_t value)
		{
			if((value & 0x80) == 0x80) {
				//When 'r' is set:
				//	- 'd' is ignored
				//	- hidden temporary reg is reset (so that the next write is the "first" write)
				//	- bits 2,3 of reg $8000 are set (16k PRG mode, $8000 swappable)
				//	- other bits of $8000 (and other regs) are unchanged
				ResetBuffer();
				_prgMode = true;
				_slotSelect = true;
				UpdateState();
			} else {
				_writeBuffer >>= 1;
				_writeBuffer |= ((value << 4) & 0x10);

				_shiftCount++;

				if(_shiftCount == 5) {
					ProcessRegisterWrite(addr, _writeBuffer);
					UpdateState();

					//Reset buffer after writing 5 bits
					ResetBuffer();
				}
			}
		}

		void ProcessRegisterWrite(uint16_t addr, uint8_t val)
		{
			switch(addr & 0xE000) {
				case 0x8000:
					switch(val & 0x03) {
						case 0: SetMirroringType(MirroringType::ScreenAOnly); break;
						case 1: SetMirroringType(MirroringType::ScreenBOnly); break;
						case 2: SetMirroringType(MirroringType::Vertical); break;
						case 3: SetMirroringType(MirroringType::Horizontal); break;
					}
					_slotSelect = (val & 0x04) != 0;
					_prgMode = (val & 0x08) != 0;
					_chrMode = (val & 0x10) != 0;
					break;

				case 0xA000:
					_lastChrReg = addr;
					_chrReg0 = val & 0x1F;
					break;

				case 0xC000:
					_lastChrReg = addr;
					_chrReg1 = val & 0x1F;
					break;

				case 0xE000:
					_prgReg = val & 0x0F;
					_wramDisable = (val & 0x10) != 0;
					break;
			}
		}

	protected:
		uint16_t GetPRGPageSize() override { return 0x4000; }
		uint16_t GetCHRPageSize() override { return 0x1000; }

		void UpdateState()
		{
			uint8_t extraReg = (_lastChrReg == 0xC000 && _chrMode) ? _chrReg1 : _chrReg0;
			uint8_t prgBankSelect = 0;
			if(_prgSize == 0x80000) {
				//512kb carts use bit 7 of $A000/$C000 to select page
				//This is used for SUROM (Dragon Warrior 3/4, Dragon Quest 4)
				prgBankSelect = extraReg & 0x10;
			} 

			MemoryAccessType access = _wramDisable && !_forceWramOn ? MemoryAccessType::NoAccess : MemoryAccessType::ReadWrite;
			PrgMemoryType memType = HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam;
			if(_saveRamSize + _workRamSize > 0x4000) {
				//SXROM, 32kb of save ram
				SetCpuMemoryMapping(0x6000, 0x7FFF, (extraReg >> 2) & 0x03, memType, access);
			} else if(_saveRamSize + _workRamSize > 0x2000) {
				if(_saveRamSize == 0x2000 && _workRamSize == 0x2000) {
					//SOROM, half of the 16kb ram is battery backed
					SetCpuMemoryMapping(0x6000, 0x7FFF, 0, (extraReg >> 3) & 0x01 ? PrgMemoryType::WorkRam : PrgMemoryType::SaveRam, access);
				} else {
					//Unknown, shouldn't happen
					SetCpuMemoryMapping(0x6000, 0x7FFF, (extraReg >> 2) & 0x01, memType, access);
				}
			} else {
				if(_saveRamSize + _workRamSize == 0) {
					RemoveCpuMemoryMapping(0x6000, 0x7FFF);
				} else {
					//Everything else - 8kb of work or save ram
					SetCpuMemoryMapping(0x6000, 0x7FFF, 0, memType, access);
				}
			}

			if(_romInfo.SubMapperID == 5) {
				//SubMapper 5
				//"001: 5 Fixed PRG    SEROM, SHROM, SH1ROM use a fixed 32k PRG ROM with no banking support.
				SelectPrgPage2x(0, 0);
			} else {
				if(_prgMode) {
					if(_slotSelect) {
						SelectPRGPage(0, _prgReg | prgBankSelect);
						SelectPRGPage(1, 0x0F | prgBankSelect);
					} else {
						SelectPRGPage(0, 0 | prgBankSelect);
						SelectPRGPage(1, _prgReg | prgBankSelect);
					}
				} else {
					SelectPrgPage2x(0, (_prgReg & 0xFE) | prgBankSelect);
				}
			} 

			if(_chrMode) {
				SelectCHRPage(0, _chrReg0);
				SelectCHRPage(1, _chrReg1);
			} else {
				SelectCHRPage(0, _chrReg0 & 0x1E);
				SelectCHRPage(1, (_chrReg0 & 0x1E) + 1);
			}
		}

		void Serialize(Serializer& s) override
		{
			BaseMapper::Serialize(s);
			SV(_writeBuffer); SV(_shiftCount); SV(_wramDisable); SV(_chrMode); SV(_prgMode); SV(_slotSelect); SV(_chrReg0); SV(_chrReg1); SV(_prgReg); SV(_lastWriteCycle); SV(_lastChrReg);
		}

		void InitMapper() override
		{
			ProcessRegisterWrite(0x8000, GetPowerOnByte() | 0x0C); //On powerup: bits 2,3 of $8000 are set (this ensures the $8000 is bank 0, and $C000 is the last bank - needed for SEROM/SHROM/SH1ROM which do no support banking)
			ProcessRegisterWrite(0xA000, GetPowerOnByte());
			ProcessRegisterWrite(0xC000, GetPowerOnByte());
			ProcessRegisterWrite(0xE000, (_romInfo.DatabaseInfo.Board.find("MMC1B") != string::npos ? 0x10 : 0x00)); //WRAM Disable: enabled by default for MMC1B

			//"MMC1A: PRG RAM is always enabled" - Normally these roms should be classified as mapper 155
			_forceWramOn = (_romInfo.DatabaseInfo.Board.compare("MMC1A") == 0);

			_lastChrReg = 0xA000;

			UpdateState();
		}

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			uint64_t currentCycle = _emu->GetMasterClock();
			
			//Ignore write if within 2 cycles of another write (i.e the real write after a dummy write)
			if(currentCycle - _lastWriteCycle >= 2) {
				ProcessBitWrite(addr, value);
			}
			_lastWriteCycle = currentCycle;
		}
};
