#include "pch.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/Cart/GbaCart.h"
#include "GBA/Cart/GbaEeprom.h"
#include "GBA/Cart/GbaFlash.h"
#include "GBA/Cart/GbaTiltSensor.h"
#include "GBA/Cart/GbaGpio.h"
#include "GBA/Cart/GbaRtc.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Shared/BaseControlManager.h"
#include "Utilities/Serializer.h"

GbaCart::GbaCart()
{
}

GbaCart::~GbaCart()
{
}

void GbaCart::Init(Emulator* emu, GbaConsole* console, GbaMemoryManager* memoryManager, GbaSaveType saveType, GbaRtcType rtcType, GbaCartridgeType cartType)
{
	_emu = emu;

	switch(cartType) {
		case GbaCartridgeType::TiltSensor:
			_tiltSensor.reset(new GbaTiltSensor(emu));
			console->GetControlManager()->AddSystemControlDevice(_tiltSensor);
			break;
	}

	if(rtcType == GbaRtcType::Enabled) {
		_rtc.reset(new GbaRtc(emu));
		_gpio.reset(new GbaGpio(_rtc.get()));
	}

	_prgRom = (uint8_t*)_emu->GetMemory(MemoryType::GbaPrgRom).Memory;
	_prgRomSize = _emu->GetMemory(MemoryType::GbaPrgRom).Size;
	_memoryManager = memoryManager;

	_saveRam = (uint8_t*)_emu->GetMemory(MemoryType::GbaSaveRam).Memory;
	_saveRamSize = _emu->GetMemory(MemoryType::GbaSaveRam).Size;

	switch(saveType) {
		case GbaSaveType::EepromUnknown:
		case GbaSaveType::Eeprom512:
		case GbaSaveType::Eeprom8192: {
			bool over16mb = _prgRomSize > 16 * 1024 * 1024;
			_eepromMask = over16mb ? 0xFFFFF00 : 0xF000000;
			_eepromAddr = over16mb ? 0xDFFFF00 : 0xD000000;
			_eeprom.reset(new GbaEeprom(_saveRam, saveType));
			_saveRamSize = 0; //disable access data via banks E/F
			break;
		}

		case GbaSaveType::Flash64:
		case GbaSaveType::Flash128:
			_flash.reset(new GbaFlash(_emu, _saveRam, _saveRamSize));
			break;

		default: break;
	}
}

void GbaCart::WriteRom(uint32_t addr, uint8_t value)
{
	if((addr & _eepromMask) == _eepromAddr) {
		WriteEeprom(addr, value);
	} else if(_gpio && addr >= 0x80000C4 && addr <= 0x80000C9) {
		_gpio->Write(addr, value);
	}
}

uint8_t GbaCart::ReadRam(uint32_t addr, uint32_t readAddr)
{
	if(_flash) {
		return _flash->Read(readAddr);
	} else if(_saveRamSize) {
		return _saveRam[readAddr & (_saveRamSize - 1)];
	} else if(_tiltSensor && addr >= 0xE008000 && addr <= 0xE008500) {
		return _tiltSensor->Read(addr);
	}
	
	return 0xFF;
}

void GbaCart::WriteRam(GbaAccessModeVal mode, uint32_t addr, uint8_t value, uint32_t writeAddr, uint32_t fullValue)
{
	if(!(mode & GbaAccessMode::Byte)) {
		uint8_t shift = (writeAddr & ((mode & GbaAccessMode::HalfWord) ? 0x01 : 0x03)) << 3;
		if(shift) {
			value = (uint8_t)((fullValue << (32 - shift)) | (fullValue >> shift));
		} else {
			value = (uint8_t)fullValue;
		}
	}

	if(_flash) {
		_flash->Write(writeAddr, value);
	} else if(_saveRamSize) {
		_saveRamDirty = true;
		_saveRam[writeAddr & (_saveRamSize - 1)] = value;
	} else if(_tiltSensor && writeAddr >= 0xE008000 && writeAddr <= 0xE008500) {
		_tiltSensor->Write(addr, value);
	}
}

uint8_t GbaCart::ReadEeprom(uint32_t addr)
{
	//TODOGBA - eeprom is apparently only accessible via DMA,
	//but implementing this seems to break the classic series games?
	uint8_t openBus = _memoryManager->GetOpenBus(addr);
	return (addr & 0x01) ? openBus : ((openBus & 0xFE) | _eeprom->Read());
}

void GbaCart::WriteEeprom(uint32_t addr, uint8_t value)
{
	if(!(addr & 0x01)) {
		_eeprom->Write(value);
	}
}

AddressInfo GbaCart::GetRamAbsoluteAddress(uint32_t addr)
{
	if(_flash) {
		if(addr < 0x10000) {
			return { (int32_t)(_flash->GetSelectedBank() | addr), MemoryType::GbaSaveRam };
		}
	} else if(_saveRamSize) {
		//save ram (not eeprom)
		return { (int32_t)(addr & (_saveRamSize - 1)), MemoryType::GbaSaveRam };
	}
	return { -1, MemoryType::None };
}

int64_t GbaCart::GetRamRelativeAddress(AddressInfo& absAddress)
{
	if(_flash) {
		uint32_t baseAddr = _flash->GetSelectedBank();
		if(((uint32_t)absAddress.Address & 0x10000) == baseAddr) {
			return 0xE000000 | (absAddress.Address & 0xFFFF);
		}
	} else if(_saveRamSize) {
		//save ram (not eeprom)
		return 0xE000000 | absAddress.Address;
	}
	return -1;
}

void GbaCart::DebugWriteRam(uint32_t addr, uint8_t value)
{
	AddressInfo absAddr = GetRamAbsoluteAddress(addr);
	if(absAddr.Address >= 0) {
		_saveRam[absAddr.Address] = value;
	}
}

void GbaCart::LoadBattery()
{
	if(_saveRam) {
		uint32_t size = _eeprom ? _eeprom->GetSaveSize() : _saveRamSize;
		_emu->GetBatteryManager()->LoadBattery(".sav", _saveRam, size);
	}

	if(_rtc) {
		_rtc->LoadBattery();
	}
}

void GbaCart::SaveBattery()
{
	if(_saveRam && (_flash || _eeprom || _saveRamDirty)) {
		uint32_t size = _eeprom ? _eeprom->GetSaveSize() : _saveRamSize;
		_emu->GetBatteryManager()->SaveBattery(".sav", _saveRam, size);
	}

	if(_rtc) {
		_rtc->SaveBattery();
	}
}

void GbaCart::Serialize(Serializer& s)
{
	if(_eeprom) {
		SV(_eeprom);
	}
	if(_flash) {
		SV(_flash);
	}
	if(_tiltSensor) {
		SV(_tiltSensor);
	}
	if(_gpio) {
		SV(_gpio);
	}
	if(_rtc) {
		SV(_rtc);
	}
}
