#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "GBA/Cart/GbaEeprom.h"
#include "GBA/Cart/GbaGpio.h"

class Emulator;
class GbaConsole;
class GbaMemoryManager;
class GbaFlash;
class GbaGpio;
class GbaRtc;
class GbaTiltSensor;

class GbaCart final : public ISerializable
{
private:
	Emulator* _emu = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;;

	shared_ptr<GbaTiltSensor> _tiltSensor;
	unique_ptr<GbaEeprom> _eeprom;
	unique_ptr<GbaFlash> _flash;
	unique_ptr<GbaGpio> _gpio;
	unique_ptr<GbaRtc> _rtc;

	uint32_t _eepromAddr = ~1; //by default, eeprom is disabled
	uint32_t _eepromMask = 0;

	uint8_t* _prgRom = nullptr;
	uint32_t _prgRomSize = 0;
	
	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;

	bool _saveRamDirty = false;

	__noinline uint8_t ReadEeprom(uint32_t addr);
	__noinline void WriteEeprom(uint32_t addr, uint8_t value);

public:
	GbaCart();
	~GbaCart();

	void Init(Emulator* emu, GbaConsole* console, GbaMemoryManager* memoryManager, GbaSaveType saveType, GbaRtcType rtcType, GbaCartridgeType cartType);

	template<bool checkEeprom>
	__forceinline uint8_t ReadRom(uint32_t addr)
	{
		if constexpr(checkEeprom) {
			if((addr & _eepromMask) == _eepromAddr) {
				return ReadEeprom(addr);
			}
		} else if(_gpio && addr >= 0x80000C4 && addr <= 0x80000C9) {
			return _gpio->Read(addr);
		}

		addr &= 0x1FFFFFF;
		if(addr < _prgRomSize) {
			return _prgRom[addr];
		}

		//Cartridge uses the same lines for the bottom 16-bits of the address and the data.
		//After a load outside of the rom's bounds, the value on the bus is the address, which becomes
		//the value returned by open bus.
		//Addresses are in half-words, so the data received is shifted 1 compared to "addr" here
		//which is in bytes, not half-words.
		return addr & 0x01 ? (addr >> 9) : (addr >> 1);
	}

	void WriteRom(uint32_t addr, uint8_t value);

	uint8_t ReadRam(uint32_t addr, uint32_t readAddr);
	void WriteRam(GbaAccessModeVal mode, uint32_t addr, uint8_t value, uint32_t writeAddr, uint32_t fullValue);
	
	void DebugWriteRam(uint32_t addr, uint8_t value);
	AddressInfo GetRamAbsoluteAddress(uint32_t addr);
	int64_t GetRamRelativeAddress(AddressInfo& absAddress);

	void LoadBattery();
	void SaveBattery();

	void Serialize(Serializer& s) override;
};