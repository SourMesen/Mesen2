#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/Mappers/Audio/Mmc5Audio.h"
#include "NES/Debugger/IExtModeMapperDebug.h"
#include "NES/Mappers/Nintendo/Mmc5MemoryHandler.h"
#include "Utilities/HexUtilities.h"

class MMC5 : public BaseMapper, public IExtModeMapperDebug
{
private:
	unique_ptr<Mmc5Audio> _audio;
	unique_ptr<Mmc5MemoryHandler> _mmc5MemoryHandler;

	uint8_t* _fillNametable = nullptr;
	uint8_t* _emptyNametable = nullptr;

	uint8_t _prgRamProtect1 = 0;
	uint8_t _prgRamProtect2 = 0;

	uint8_t _fillModeTile = 0;
	uint8_t _fillModeColor = 0;

	bool _verticalSplitEnabled = false;
	bool _verticalSplitRightSide = false;
	uint8_t _verticalSplitDelimiterTile = 0;
	uint8_t _verticalSplitScroll = 0;
	uint8_t _verticalSplitBank = 0;

	bool _splitInSplitRegion = false;
	uint32_t _splitVerticalScroll = 0;
	uint32_t _splitTile = 0;
	int32_t _splitTileNumber = 0;

	uint8_t _multiplierValue1 = 0;
	uint8_t _multiplierValue2 = 0;

	uint8_t _nametableMapping = 0;
	uint8_t _extendedRamMode = 0;

	//Extended attribute mode fields (used when _extendedRamMode == 1)
	uint16_t _exAttributeLastNametableFetch = 0;
	int8_t _exAttrLastFetchCounter = 0;
	uint8_t _exAttrSelectedChrBank = 0;

	uint8_t _prgMode = 0;
	uint8_t _prgBanks[5] = {};

	//CHR-related fields
	uint8_t _chrMode = 0;
	uint8_t _chrUpperBits = 0;
	uint16_t _chrBanks[12] = {};
	uint16_t _lastChrReg = 0;
	bool _prevChrA = false;

	//IRQ counter related fields
	uint8_t _irqCounterTarget = 0;
	bool _irqEnabled = false;
	uint8_t _scanlineCounter = 0;
	bool _irqPending = false;

	bool _needInFrame = false;
	bool _ppuInFrame = false;
	uint8_t _ppuIdleCounter = 0;
	uint16_t _lastPpuReadAddr = 0;
	uint8_t _ntReadCounter = 0;

	void SwitchPrgBank(uint16_t reg, uint8_t value)
	{
		_prgBanks[reg - 0x5113] = value;
		UpdatePrgBanks();
	}

	void GetCpuBankInfo(uint16_t reg, uint8_t &bankNumber, PrgMemoryType &memoryType, uint8_t &accessType)
	{
		bankNumber = _prgBanks[reg-0x5113];
		memoryType = PrgMemoryType::PrgRom;
		if((((bankNumber & 0x80) == 0x00) && reg != 0x5117) || reg == 0x5113) {
			accessType = MemoryAccessType::Read;
			if(_prgRamProtect1 == 0x02 && _prgRamProtect2 == 0x01) {
				accessType |= MemoryAccessType::Write;
			}

			if(IsNes20() && (_workRamSize == 0x20000 || _saveRamSize == 0x20000)) {
				bankNumber &= 0x0F;
				memoryType = HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam;
			} else {
				bankNumber &= 0x07;

				// WRAM/SRAM mirroring logic (only supports existing/known licensed MMC5 boards)
				//            Bank number
				//            0 1 2 3 4 5 6 7
				// --------------------------
				// None     : - - - - - - - -
				// 1x 8kb   : 0 0 0 0 - - - -
				// 2x 8kb   : 0 0 0 0 1 1 1 1
				// 1x 32kb  : 0 1 2 3 - - - -
				if(IsNes20() || _romInfo.IsInDatabase) {
					memoryType = PrgMemoryType::WorkRam;
					if(HasBattery() && (bankNumber <= 3 || _saveRamSize > 0x2000)) {
						memoryType = PrgMemoryType::SaveRam;
					}

					if(_saveRamSize + _workRamSize != 0x4000 && bankNumber >= 4) {
						//When not 2x 8kb (=16kb), banks 4/5/6/7 select the empty socket and return open bus
						accessType = MemoryAccessType::NoAccess;
					}
				} else {
					memoryType = HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam;
				}
			}
		} else {
			accessType = MemoryAccessType::Read;
			bankNumber &= 0x7F;
		}
	}

	void UpdatePrgBanks()
	{
		uint8_t value;
		PrgMemoryType memoryType;
		uint8_t accessType;

		GetCpuBankInfo(0x5113, value, memoryType, accessType);
		if(accessType == MemoryAccessType::NoAccess) {
			RemoveCpuMemoryMapping(0x6000, 0x7FFF);
		} else {
			SetCpuMemoryMapping(0x6000, 0x7FFF, value, memoryType, accessType);
		}

		//PRG Bank 0
		//Mode 0,1,2 - Ignored
		//Mode 3 - Select an 8KB PRG bank at $8000-$9FFF
		if(_prgMode == 3) {
			GetCpuBankInfo(0x5114, value, memoryType, accessType);
			SetCpuMemoryMapping(0x8000, 0x9FFF, value, memoryType, accessType);
		}

		//PRG Bank 1
		//Mode 0 - Ignored
		//Mode 1,2 - Select a 16KB PRG bank at $8000-$BFFF (ignore bottom bit)
		//Mode 3 - Select an 8KB PRG bank at $A000-$BFFF
		GetCpuBankInfo(0x5115, value, memoryType, accessType);
		if(_prgMode == 1 || _prgMode == 2) {
			SetCpuMemoryMapping(0x8000, 0xBFFF, value & 0xFE, memoryType, accessType);
		} else if(_prgMode == 3) {
			SetCpuMemoryMapping(0xA000, 0xBFFF, value, memoryType, accessType);
		}

		//Mode 0,1 - Ignored
		//Mode 2,3 - Select an 8KB PRG bank at $C000-$DFFF
		if(_prgMode == 2 || _prgMode == 3) {
			GetCpuBankInfo(0x5116, value, memoryType, accessType);
			SetCpuMemoryMapping(0xC000, 0xDFFF, value, memoryType, accessType);
		}

		//Mode 0 - Select a 32KB PRG ROM bank at $8000-$FFFF (ignore bottom 2 bits)
		//Mode 1 - Select a 16KB PRG ROM bank at $C000-$FFFF (ignore bottom bit)
		//Mode 2,3 - Select an 8KB PRG ROM bank at $E000-$FFFF
		GetCpuBankInfo(0x5117, value, memoryType, accessType);
		if(_prgMode == 0) {
			SetCpuMemoryMapping(0x8000, 0xFFFF, value & 0x7C, memoryType, accessType);
		} else if(_prgMode == 1) {
			SetCpuMemoryMapping(0xC000, 0xFFFF, value & 0x7E, memoryType, accessType);
		} else if(_prgMode == 2 || _prgMode == 3) {
			SetCpuMemoryMapping(0xE000, 0xFFFF, value & 0x7F, memoryType, accessType);
		}
	}

	void SwitchChrBank(uint16_t reg, uint8_t value)
	{
		uint16_t newValue = value | (_chrUpperBits << 8);
		if(newValue != _chrBanks[reg - 0x5120] || _lastChrReg != reg) {
			_chrBanks[reg - 0x5120] = newValue;
			_lastChrReg = reg;
			UpdateChrBanks(true);
		}
	}

	void UpdateChrBanks(bool forceUpdate)
	{
		bool largeSprites = (_mmc5MemoryHandler->GetReg(0x2000) & 0x20) != 0;

		if(!largeSprites) {
			//Using 8x8 sprites resets the last written to bank logic
			_lastChrReg = 0;
		}

		bool chrA = !largeSprites || (_splitTileNumber >= 32 && _splitTileNumber < 40) || (!_ppuInFrame && _lastChrReg <= 0x5127);
		if(!forceUpdate && chrA == _prevChrA) {
			return;
		}
		_prevChrA = chrA;

		if(_chrMode == 0) {
			SelectChrPage8x(0, _chrBanks[chrA ? 0x07 : 0x0B] << 3);
		} else if(_chrMode == 1) {
			SelectChrPage4x(0, _chrBanks[chrA ? 0x03 : 0x0B] << 2);
			SelectChrPage4x(1, _chrBanks[chrA ? 0x07 : 0x0B] << 2);
		} else if(_chrMode == 2) {
			SelectChrPage2x(0, _chrBanks[chrA ? 0x01 : 0x09] << 1);
			SelectChrPage2x(1, _chrBanks[chrA ? 0x03 : 0x0B] << 1);
			SelectChrPage2x(2, _chrBanks[chrA ? 0x05 : 0x09] << 1);
			SelectChrPage2x(3, _chrBanks[chrA ? 0x07 : 0x0B] << 1);
		} else if(_chrMode == 3) {
			SelectChrPage(0, _chrBanks[chrA ? 0x00 : 0x08]);
			SelectChrPage(1, _chrBanks[chrA ? 0x01 : 0x09]);
			SelectChrPage(2, _chrBanks[chrA ? 0x02 : 0x0A]);
			SelectChrPage(3, _chrBanks[chrA ? 0x03 : 0x0B]);
			SelectChrPage(4, _chrBanks[chrA ? 0x04 : 0x08]);
			SelectChrPage(5, _chrBanks[chrA ? 0x05 : 0x09]);
			SelectChrPage(6, _chrBanks[chrA ? 0x06 : 0x0A]);
			SelectChrPage(7, _chrBanks[chrA ? 0x07 : 0x0B]);
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		_audio->Clock();

		if(_ppuIdleCounter) {
			_ppuIdleCounter--;
			if(_ppuIdleCounter == 0) {
				//"The "in-frame" flag is cleared when the PPU is no longer rendering. This is detected when 3 CPU cycles pass without a PPU read having occurred (PPU /RD has not been low during the last 3 M2 rises)."
				_ppuInFrame = false;
				UpdateChrBanks(true);
			}
		}
	}

	void SetNametableMapping(uint8_t value)
	{
		_nametableMapping = value;

		for(int i = 0; i < 4; i++) {
			uint8_t nametableId = (value >> (i * 2)) & 0x03;
			if(nametableId <= 1) {
				SetNametable(i, nametableId);
			} else if(nametableId == 2) {
				if(_extendedRamMode <= 1) {
					SetPpuMemoryMapping(0x2000 + i * 0x400, 0x2000 + i * 0x400 + 0x3FF, ChrMemoryType::MapperRam, 0, MemoryAccessType::ReadWrite);
				} else {
					SetPpuMemoryMapping(0x2000 + i * 0x400, 0x2000 + i * 0x400 + 0x3FF, _emptyNametable, 0, BaseMapper::NametableSize, MemoryAccessType::Read);
				}
			} else {
				SetPpuMemoryMapping(0x2000 + i * 0x400, 0x2000 + i * 0x400 + 0x3FF, _fillNametable, 0, BaseMapper::NametableSize, MemoryAccessType::Read);
			}
		}
	}

	void SetExtendedRamMode(uint8_t mode)
	{
		_extendedRamMode = mode;

		MemoryAccessType accessType;
		if(_extendedRamMode <= 1) {
			//"Mode 0/1 - Not readable (returns open bus), can only be written while the PPU is rendering (otherwise, 0 is written)"
			//See overridden WriteRam function for implementation
			accessType = MemoryAccessType::Write;
		} else if(_extendedRamMode == 2) {
			//"Mode 2 - Readable and writable"
			accessType = MemoryAccessType::ReadWrite;
		} else {
			//"Mode 3 - Read-only"
			accessType = MemoryAccessType::Read;
		}

		SetCpuMemoryMapping(0x5C00, 0x5FFF, PrgMemoryType::MapperRam, 0, accessType);

		SetNametableMapping(_nametableMapping);
	}

	void SetFillModeTile(uint8_t tile)
	{
		_fillModeTile = tile;
		memset(_fillNametable, tile, 32 * 30); //32 tiles per row, 30 rows
	}

	void SetFillModeColor(uint8_t color)
	{
		_fillModeColor = color;
		uint8_t attributeByte = color | color << 2 | color << 4 | color << 6;
		memset(_fillNametable + 32 * 30, attributeByte, 64); //Attribute table is 64 bytes
	}

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	uint32_t GetMapperRamSize() override { return 0x400; }
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5206; }
	uint32_t GetSaveRamPageSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x2000; }
	bool ForceSaveRamSize() override { return true; }
	bool ForceWorkRamSize() override { return true; }
	bool AllowRegisterRead() override { return true; }
	bool EnableCpuClockHook() override { return true; }
	bool EnableCustomVramRead() override { return true; }

	uint32_t GetSaveRamSize() override
	{
		uint32_t size;
		if(IsNes20()) {
			size = _romInfo.Header.GetSaveRamSize();
		} else if(_romInfo.IsInDatabase) {
			size = _romInfo.DatabaseInfo.SaveRamSize;
		} else {
			//Emulate as if a single 64k block of work/save ram existed
			size = _romInfo.HasBattery ? 0x10000 : 0;
		}
		return size;
	}

	uint32_t GetWorkRamSize() override
	{
		uint32_t size;
		if(IsNes20()) {
			size = _romInfo.Header.GetWorkRamSize();
		} else if(_romInfo.IsInDatabase) {
			size = _romInfo.DatabaseInfo.WorkRamSize;
		} else {
			//Emulate as if a single 64k block of work/save ram existed (+ 1kb of ExRAM)
			size = (_romInfo.HasBattery ? 0 : 0x10000);
		}
		return size;
	}

	void InitMapper() override
	{
		AddRegisterRange(0xFFFA, 0xFFFB, MemoryOperation::Read);

		_audio.reset(new Mmc5Audio(_console));
		
		//Override the 2000-2007 registers to catch all writes to the PPU registers (but not their mirrors)
		_mmc5MemoryHandler.reset(new Mmc5MemoryHandler(_console));

		_splitTileNumber = -1;

		_emptyNametable = new uint8_t[BaseMapper::NametableSize];
		memset(_emptyNametable, 0, BaseMapper::NametableSize);

		_fillNametable = new uint8_t[BaseMapper::NametableSize];
		memset(_fillNametable, 0, BaseMapper::NametableSize);

		SetExtendedRamMode(0);

		//"Additionally, Romance of the 3 Kingdoms 2 seems to expect it to be in 8k PRG mode ($5100 = $03)."
		WriteRegister(0x5100, 0x03);

		//"Games seem to expect $5117 to be $FF on powerup (last PRG page swapped in)."
		WriteRegister(0x5117, 0xFF);

		UpdateChrBanks(true);
	}

	virtual ~MMC5()
	{
		delete[] _fillNametable;
		delete[] _emptyNametable;
	}

	void Reset(bool softReset) override
	{
		_console->GetMemoryManager()->RegisterWriteHandler(_mmc5MemoryHandler.get(), 0x2000, 0x2007);
	}

	void LoadBattery() override
	{
		if(HasBattery() && _saveRamSize > 0) {
			//Load EXRAM and save ram from the same file
			vector<uint8_t> data(_saveRamSize + _mapperRamSize);
			_emu->GetBatteryManager()->LoadBattery(".sav", data.data(), _saveRamSize + _mapperRamSize);
			memcpy(_saveRam, data.data(), _saveRamSize);
			memcpy(_mapperRam, data.data()+_saveRamSize, _mapperRamSize);
		}
	}

	void SaveBattery() override
	{
		if(HasBattery()) {
			//Save EXRAM and save ram to the same file
			vector<uint8_t> data(_saveRam, _saveRam + _saveRamSize);
			data.insert(data.end(), _mapperRam, _mapperRam + _mapperRamSize);
			_emu->GetBatteryManager()->SaveBattery(".sav", data.data(), (uint32_t)data.size());
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_prgBanks, 5);
		SVArray(_chrBanks, 12);
		SV(_audio);
		SV(_prgRamProtect1); SV(_prgRamProtect2); SV(_fillModeTile); SV(_fillModeColor); SV(_verticalSplitEnabled); SV(_verticalSplitRightSide);
		SV(_verticalSplitDelimiterTile); SV(_verticalSplitScroll); SV(_verticalSplitBank); SV(_multiplierValue1); SV(_multiplierValue2);
		SV(_nametableMapping); SV(_extendedRamMode); SV(_exAttributeLastNametableFetch); SV(_exAttrLastFetchCounter); SV(_exAttrSelectedChrBank);
		SV(_prgMode); SV(_chrMode); SV(_chrUpperBits); SV(_lastChrReg);
		SV(_irqCounterTarget); SV(_irqEnabled); SV(_scanlineCounter); SV(_irqPending); SV(_ppuInFrame);
		SV(_splitInSplitRegion); SV(_splitVerticalScroll); SV(_splitTile); SV(_splitTileNumber); SV(_needInFrame);

		SV(_prevChrA);
		SV(_ppuIdleCounter);
		SV(_lastPpuReadAddr);
		SV(_ntReadCounter);

		if(!s.IsSaving()) {
			UpdatePrgBanks();
			SetFillModeTile(_fillModeTile);
			SetFillModeColor(_fillModeColor);
			SetNametableMapping(_nametableMapping);
		}
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x5C00 && addr <= 0x5FFF && _extendedRamMode <= 1 && !_ppuInFrame) {
			//Expansion RAM ($5C00-$5FFF, read/write)
			//Mode 0/1 - Not readable (returns open bus), can only be written while the PPU is rendering (otherwise, 0 is written)
			value = 0;
		}
		BaseMapper::WriteRam(addr, value);
	}

	void DetectScanlineStart(uint16_t addr)
	{
		if(_ntReadCounter >= 2) {
			//After 3 identical NT reads, trigger IRQ when the following attribute byte is read
			if(!_ppuInFrame && !_needInFrame) {
				_needInFrame = true;
				_scanlineCounter = 0;
			} else {
				_scanlineCounter++;
				if(_irqCounterTarget == _scanlineCounter) {
					_irqPending = true;
					if(_irqEnabled) {
						_console->GetCpu()->SetIrqSource(IRQSource::External);
					}
				}
			}
			_splitTileNumber = 0;
			_ntReadCounter = 0;
		} else if(addr >= 0x2000 && addr <= 0x2FFF) {
			if(_lastPpuReadAddr == addr) {
				//Count consecutive identical reads
				_ntReadCounter++;
			} else {
				_ntReadCounter = 0;
			}
		} else {
			_ntReadCounter = 0;
		}
	}

	uint8_t MapperReadVram(uint16_t addr, MemoryOperationType memoryOperationType) override
	{
		bool isNtFetch = addr >= 0x2000 && addr <= 0x2FFF && (addr & 0x3FF) < 0x3C0;
		if(isNtFetch) {
			//Nametable data, not an attribute fetch
			_splitInSplitRegion = false;
			_splitTileNumber++;

			if(_ppuInFrame) {
				UpdateChrBanks(false);
			} else if(_needInFrame) {
				_needInFrame = false;
				_ppuInFrame = true;
				UpdateChrBanks(false);
			}
		}
		DetectScanlineStart(addr);

		_ppuIdleCounter = 3;
		_lastPpuReadAddr = addr;

		if(_extendedRamMode <= 1 && _ppuInFrame) {
			if(_verticalSplitEnabled) {
				uint16_t verticalSplitScroll = (_verticalSplitScroll + _scanlineCounter) % 240;
				if(addr >= 0x2000) {
					if(isNtFetch) {
						uint8_t tileNumber = (_splitTileNumber + 2) % 42;
						if(tileNumber <= 32 && ((_verticalSplitRightSide && tileNumber >= _verticalSplitDelimiterTile) || (!_verticalSplitRightSide && tileNumber < _verticalSplitDelimiterTile))) {
							//Split region (for next 3 fetches, attribute + 2x tile data)
							_splitInSplitRegion = true;
							_splitTile = ((verticalSplitScroll & 0xF8) << 2) | tileNumber;
							return InternalReadRam(0x5C00 + _splitTile);
						} else {
							//Outside of split region (or sprite data), result can get modified by ex ram mode code below
							_splitInSplitRegion = false;
						}
					} else if(_splitInSplitRegion) {
						return InternalReadRam(0x5FC0 | ((_splitTile & 0x380) >> 4) | ((_splitTile & 0x1F) >> 2));
					}
				} else if(_splitInSplitRegion) {
					//CHR tile fetches for split region
					return ReadFromChr((_verticalSplitBank << 12) + (((addr & ~0x07) | (verticalSplitScroll & 0x07)) & 0xFFF));
				}
			}

			if(_extendedRamMode == 1 && (_splitTileNumber < 32 || _splitTileNumber >= 40)) {
				//"In Mode 1, nametable fetches are processed normally, and can come from CIRAM nametables, fill mode, or even Expansion RAM, but attribute fetches are replaced by data from Expansion RAM."
				//"Each byte of Expansion RAM is used to enhance the tile at the corresponding address in every nametable"

				//When fetching NT data, we set a flag and then alter the VRAM values read by the PPU on the following 3 cycles (palette, tile low/high byte)
				if(isNtFetch) {
					//Nametable fetches
					_exAttributeLastNametableFetch = addr & 0x03FF;
					_exAttrLastFetchCounter = 3;
				} else if(_exAttrLastFetchCounter > 0) {
					//Attribute fetches
					_exAttrLastFetchCounter--;
					switch(_exAttrLastFetchCounter) {
						case 2:
						{
							//PPU palette fetch
							//Check work ram (expansion ram) to see which tile/palette to use
							//Use InternalReadRam to bypass the fact that the ram is supposed to be write-only in mode 0/1
							uint8_t value = InternalReadRam(0x5C00 + _exAttributeLastNametableFetch);

							//"The pattern fetches ignore the standard CHR banking bits, and instead use the top two bits of $5130 and the bottom 6 bits from Expansion RAM to choose a 4KB bank to select the tile from."
							_exAttrSelectedChrBank = (value & 0x3F) | (_chrUpperBits << 6);

							//Return a byte containing the same palette 4 times - this allows the PPU to select the right palette no matter the shift value
							uint8_t palette = (value & 0xC0) >> 6;
							return palette | palette << 2 | palette << 4 | palette << 6;
						}

						case 1:
						case 0:
							//PPU tile data fetch (high byte & low byte)
							return ReadFromChr((_exAttrSelectedChrBank << 12) + (addr & 0xFFF));
					}
				}
			}
		}
		
		return InternalReadVram(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x5113 && addr <= 0x5117) {
			SwitchPrgBank(addr, value);
		} else if(addr >= 0x5120 && addr <= 0x512B) {
			SwitchChrBank(addr, value);
		} else {
			switch(addr) {
				case 0x5000: case 0x5001: case 0x5002: case 0x5003: case 0x5004: case 0x5005: case 0x5006: case 0x5007: case 0x5010: case 0x5011: case 0x5015:
					_audio->WriteRegister(addr, value);
					break;

				case 0x5100: _prgMode = value & 0x03; UpdatePrgBanks(); break;
				case 0x5101: _chrMode = value & 0x03; UpdateChrBanks(true); break;
				case 0x5102: _prgRamProtect1 = value & 0x03; UpdatePrgBanks(); break;
				case 0x5103: _prgRamProtect2 = value & 0x03; UpdatePrgBanks(); break;
				case 0x5104: SetExtendedRamMode(value & 0x03); break;
				case 0x5105: SetNametableMapping(value); break;
				case 0x5106: SetFillModeTile(value); break;
				case 0x5107: SetFillModeColor(value & 0x03); break;
				case 0x5130: _chrUpperBits = value & 0x03; break;
				case 0x5200: 
					_verticalSplitEnabled = (value & 0x80) == 0x80; 
					_verticalSplitRightSide = (value & 0x40) == 0x40; 
					_verticalSplitDelimiterTile = (value & 0x1F);
					break;
				case 0x5201: _verticalSplitScroll = value; break;
				case 0x5202: _verticalSplitBank = value; break;
				case 0x5203: _irqCounterTarget = value; break;
				case 0x5204: 
					_irqEnabled = (value & 0x80) == 0x80;
					if(!_irqEnabled) {
						_console->GetCpu()->ClearIrqSource(IRQSource::External);
					} else if(_irqEnabled && _irqPending) {
						_console->GetCpu()->SetIrqSource(IRQSource::External);
					}
					break;
				case 0x5205: _multiplierValue1 = value; break;
				case 0x5206: _multiplierValue2 = value; break;

				default:
					break;
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		switch(addr) {
			case 0x5010: case 0x5015: 
				return _audio->ReadRegister(addr);

			case 0x5204:
			{
				uint8_t value = (_ppuInFrame ? 0x40 : 0x00) | (_irqPending ? 0x80 : 0x00);
				_irqPending = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				return value;
			}

			case 0x5205: return (_multiplierValue1*_multiplierValue2) & 0xFF;
			case 0x5206: return (_multiplierValue1*_multiplierValue2) >> 8;

			case 0xFFFA: 
			case 0xFFFB:
				_ppuInFrame = false;
				UpdateChrBanks(true);
				_lastPpuReadAddr = 0;
				_scanlineCounter = 0;
				_irqPending = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				return DebugReadRam(addr);
		}

		return _console->GetMemoryManager()->GetOpenBus();
	}

	vector<MapperStateEntry> GetMapperStateEntries() override
	{
		vector<MapperStateEntry> entries;

		entries.push_back(MapperStateEntry("$5100", "PRG Mode", _prgMode, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5101", "CHR Mode", _chrMode, MapperStateValueType::Number8));
		
		entries.push_back(MapperStateEntry("$5102", "Work RAM Write Protect", _prgRamProtect1, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5103", "Work RAM Write Protect", _prgRamProtect1, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5102/3", "Work RAM Write Protected", _prgRamProtect1 != 0x02 || _prgRamProtect2 != 0x01));

		entries.push_back(MapperStateEntry("$5104", "Extended RAM Mode", _extendedRamMode, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5105.0-1", "Nametable 0", _nametableMapping & 0x03, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5105.2-3", "Nametable 1", (_nametableMapping >> 2) & 0x03, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5105.4-5", "Nametable 2", (_nametableMapping >> 4) & 0x03, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5105.6-7", "Nametable 3", (_nametableMapping >> 6) & 0x03, MapperStateValueType::Number8));
		
		entries.push_back(MapperStateEntry("$5106", "Fill Mode Tile", _fillModeTile, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5107", "Fill Mode Color", _fillModeColor, MapperStateValueType::Number8));

		for(int i = 0; i < 5; i++) {
			entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(0x5113 + i), "PRG Bank Register " + std::to_string(i), _prgBanks[i], MapperStateValueType::Number8));
		}

		for(int i = 0; i < 12; i++) {
			entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(0x5120 + i), "CHR Bank Register " + std::to_string(i), _chrBanks[i], MapperStateValueType::Number8));
		}

		entries.push_back(MapperStateEntry("$5130", "CHR Upper Bits", _chrUpperBits, MapperStateValueType::Number8));
		
		entries.push_back(MapperStateEntry("$5200.0-4", "Vertical Split - Delimiter Tile", _verticalSplitDelimiterTile, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5200.6", "Vertical Split - Right Side", _verticalSplitRightSide));
		entries.push_back(MapperStateEntry("$5200.7", "Vertical Split - Enabled", _verticalSplitEnabled));
		entries.push_back(MapperStateEntry("$5201", "Vertical Split - Scroll", _verticalSplitScroll, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5202", "Vertical Split - Bank", _verticalSplitBank, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("$5203", "IRQ Counter Target", _irqCounterTarget, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5204.7", "IRQ Enabled", _irqEnabled));

		entries.push_back(MapperStateEntry("$5205", "Multiplicand", _multiplierValue1, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5206", "Multiplier", _multiplierValue2, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5205/6", "Multiplication Result", _multiplierValue1*_multiplierValue2, MapperStateValueType::Number16));

		_audio->GetMapperStateEntries(entries);

		return entries;
	}

	__forceinline uint8_t ReadFromChr(uint32_t pos)
	{
		uint32_t size = (_chrRomSize || !_chrRam) ? _chrRomSize : _chrRamSize;
		if(size == 0) {
			return 0;
		}

		return ((_chrRomSize || !_chrRam) ? _chrRom : _chrRam)[pos & (size - 1)];
	}

public:
	bool HasExtendedAttributes(ExtModeConfig& cfg, uint8_t ntIndex) override
	{
		return cfg.Nametables[0].AttrExtMode;
	}
	
	bool HasExtendedBackground(ExtModeConfig& cfg, uint8_t ntIndex) override
	{
		if(ntIndex == 4) {
			return true;
		}

		return cfg.Nametables[0].BgExtMode;
	}

	uint8_t GetExAttributePalette(ExtModeConfig& cfg, uint8_t ntIndex, uint16_t ntOffset) override
	{
		uint8_t value = cfg.ExtRam[ntOffset];
		return (value & 0xC0) >> 6;
	}

	uint8_t GetExBackgroundChrData(ExtModeConfig& cfg, uint8_t ntIndex, uint16_t ntOffset, uint16_t chrAddr) override
	{
		if(ntIndex == 4) {
			//Split mode
			return ReadFromChr((cfg.Nametables[4].SourceOffset << 12) + (chrAddr & 0xFFF));
		} else {
			uint8_t value = cfg.ExtRam[ntOffset];
			uint16_t chrBank = (value & 0x3F) | (cfg.BgExtBank << 6);
			uint32_t addr = (chrBank << 12) + (chrAddr & 0xFFF);
			return ReadFromChr(addr);
		}
	}

	ExtModeConfig GetExModeConfig() override
	{
		ExtModeConfig cfg = {};

		cfg.Nametables[0].AttrExtMode = _extendedRamMode == 1;
		cfg.Nametables[0].BgExtMode = _extendedRamMode == 1;
		cfg.BgExtBank = _chrUpperBits;

		cfg.Nametables[4].SourceOffset = _verticalSplitBank;
		cfg.WindowScrollY = _verticalSplitScroll;

		memcpy(cfg.ExtRam, _mapperRam, _mapperRamSize);

		return cfg;
	}
};
