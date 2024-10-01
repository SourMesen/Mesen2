#pragma once
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "NES/NesControlManager.h"
#include "NES/Input/DatachBarcodeReader.h"
#include "NES/Mappers/Bandai/BaseEeprom24C0X.h"
#include "NES/Mappers/Bandai/Eeprom24C01.h"
#include "NES/Mappers/Bandai/Eeprom24C02.h"

class BandaiFcg : public BaseMapper
{
private:
	bool _irqEnabled = false;
	uint16_t _irqCounter = 0;
	uint16_t _irqReload = 0;
	uint8_t _prgPage = 0;
	uint8_t _prgBankSelect = 0;
	uint8_t _chrRegs[8] = {};
	shared_ptr<DatachBarcodeReader> _barcodeReader;

	unique_ptr<BaseEeprom24C0X> _standardEeprom;
	unique_ptr<BaseEeprom24C0X> _extraEeprom;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool AllowRegisterRead() override { return true; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		memset(_chrRegs, 0, sizeof(_chrRegs));
		_irqEnabled = false;
		_irqCounter = 0;
		_irqReload = 0;
		_prgPage = 0;
		_prgBankSelect = 0;

		//Only allow reads from 0x6000 to 0x7FFF
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);

		if(_romInfo.MapperID == 157) {
			//"Mapper 157 is used for Datach Joint ROM System boards"
			_barcodeReader.reset(new DatachBarcodeReader(_console));
			_console->GetControlManager()->AddSystemControlDevice(_barcodeReader);

			//Datach Joint ROM System
			//"It contains an internal 256-byte serial EEPROM (24C02) that is shared among all Datach games."
			//"One game, Battle Rush: Build up Robot Tournament, has an additional external 128-byte serial EEPROM (24C01) on the game cartridge."
			//"The NES 2.0 header's PRG-NVRAM field will only denote whether the game cartridge has an additional 128-byte serial EEPROM"
			if(!IsNes20() || _romInfo.Header.GetSaveRamSize() == 128) {
				_extraEeprom.reset(new Eeprom24C01(_console));
			}
			
			//All mapper 157 games have an internal 256-byte EEPROM
			_standardEeprom.reset(new Eeprom24C02(_console));
		} else if(_romInfo.MapperID == 159) {
			//LZ93D50 with 128 byte serial EEPROM (24C01)
			_standardEeprom.reset(new Eeprom24C01(_console));
		} else if(_romInfo.MapperID == 16) {
			//"INES Mapper 016 submapper 4: FCG-1/2 ASIC, no serial EEPROM, banked CHR-ROM"
			//"INES Mapper 016 submapper 5: LZ93D50 ASIC and no or 256-byte serial EEPROM, banked CHR-ROM"
			
			//Add a 256 byte serial EEPROM (24C02)
			if(_romInfo.SubMapperID == 0 || (_romInfo.SubMapperID == 5 && _romInfo.Header.GetSaveRamSize() == 256)) {
				//Connect a 256-byte EEPROM for iNES roms, and when submapper 5 + 256 bytes of save ram in header
				_standardEeprom.reset(new Eeprom24C02(_console));
			}
		}

		if(_romInfo.MapperID != 16) {
			//"For iNES Mapper 153 (with SRAM), the writeable ports must only be mirrored across $8000-$FFFF."
			//"Mappers 157 and 159 do not need to support the FCG-1 and -2 and so should only mirror the ports across $8000-$FFFF."
			if(_romInfo.MapperID == 153) {
				//Mapper 153 has regular save ram from $6000-$7FFF, need to remove the register for both read & writes
				RemoveRegisterRange(0x6000, 0x7FFF, MemoryOperation::Any);
			} else {
				RemoveRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
			}
		} else if(_romInfo.MapperID == 16 && _romInfo.SubMapperID == 4) {
			RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);
		} else if(_romInfo.MapperID == 16 && _romInfo.SubMapperID == 5) {
			RemoveRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
		}

		//Last bank
		SelectPrgPage(1, 0x0F);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_chrRegs, 8);
		SV(_irqEnabled);
		SV(_irqCounter);
		SV(_irqReload);
		SV(_prgPage);
		SV(_prgBankSelect);

		if(_standardEeprom) {
			SV(_standardEeprom);
		}

		if(_extraEeprom) {
			SV(_extraEeprom);
		}
	}

	void SaveBattery() override
	{
		if(_standardEeprom) {
			_standardEeprom->SaveBattery();
		}
		if(_extraEeprom) {
			_extraEeprom->SaveBattery();
		} else {
			//Do not call BaseMapper::SaveBattery when the extra EEPROM exists (prevent unused .sav file from being created)
			BaseMapper::SaveBattery();
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			//Checking counter before decrementing seems to be the only way to get both
			//Famicom Jump II - Saikyou no 7 Nin (J) and Magical Taruruuto-kun 2 - Mahou Daibouken (J)
			//to work without glitches with the same code.
			if(_irqCounter == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
			_irqCounter--;
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t output = 0;

		if(_barcodeReader) {
			output |= _barcodeReader->GetOutput();
		}

		if(_extraEeprom && _standardEeprom) {
			output |= (_standardEeprom->Read() && _extraEeprom->Read()) << 4;
		} else if(_standardEeprom) {
			output |= (_standardEeprom->Read() << 4);
		}

		return output | _console->GetMemoryManager()->GetOpenBus(0xE7);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x000F) {
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				_chrRegs[addr & 0x07] = value;
				if(_romInfo.MapperID == 153 || GetPrgPageCount() >= 0x20) {
					_prgBankSelect = 0;
					for(int i = 0; i < 8; i++) {
						_prgBankSelect |= (_chrRegs[i] & 0x01) << 4;
					}
					SelectPrgPage(0, _prgPage | _prgBankSelect);
					SelectPrgPage(1, 0x0F | _prgBankSelect);
				} else if(!HasChrRam() && _romInfo.MapperID != 157) {
					SelectChrPage(addr & 0x07, value);
				}

				if(_extraEeprom && _romInfo.MapperID == 157 && (addr & 0x0F) <= 3) {
					_extraEeprom->WriteScl((value >> 3) & 0x01);
				}
				break;

			case 0x08:
				_prgPage = value & 0x0F;
				SelectPrgPage(0, _prgPage | _prgBankSelect);
				break;

			case 0x09:
				switch(value & 0x03) {
					case 0: SetMirroringType(MirroringType::Vertical); break;
					case 1: SetMirroringType(MirroringType::Horizontal); break;
					case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
				break;

			case 0x0A:
				_irqEnabled = (value & 0x01) == 0x01;

				//Wiki claims there is no reload value, however this seems to be the only way to make Famicom Jump II - Saikyou no 7 Nin work properly 
				if(_romInfo.MapperID != 16 || _romInfo.SubMapperID != 4) {
					//"On the LZ93D50 (Submapper 5), writing to this register also copies the latch to the actual counter."
					_irqCounter = _irqReload;
				}

				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0x0B:
				if(_romInfo.MapperID != 16 || _romInfo.SubMapperID != 4) {
					//"On the LZ93D50 (Submapper 5), these registers instead modify a latch that will only be copied to the actual counter when register $800A is written to."
					_irqReload = (_irqReload & 0xFF00) | value;
				} else {
					//"On the FCG-1/2 (Submapper 4), writing to these two registers directly modifies the counter itself; all such games therefore disable counting before changing the counter value."
					_irqCounter = (_irqCounter & 0xFF00) | value;
				}
				break;

			case 0x0C:
				if(_romInfo.MapperID != 16 || _romInfo.SubMapperID != 4) {
					_irqReload = (_irqReload & 0xFF) | (value << 8);
				} else {
					_irqCounter = (_irqCounter & 0xFF) | (value << 8);
				}
				break;

			case 0x0D:
				if(_romInfo.MapperID == 153) {
					SetCpuMemoryMapping(0x6000, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, value & 0x20 ? MemoryAccessType::ReadWrite : MemoryAccessType::NoAccess);
				} else {
					uint8_t scl = (value & 0x20) >> 5;
					uint8_t sda = (value & 0x40) >> 6;
					if(_standardEeprom) {
						_standardEeprom->Write(scl, sda);
					}
					if(_extraEeprom) {
						_extraEeprom->WriteSda(sda);
					}
				}
				break;
		}
	}
};
