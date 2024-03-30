#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "GBA/Cart/GbaEeprom.h"

class Emulator;
class GbaMemoryManager;
class GbaFlash;

class GbaCart final : public ISerializable
{
private:
	Emulator* _emu = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;;

	unique_ptr<GbaEeprom> _eeprom;
	unique_ptr<GbaFlash> _flash;
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

	void Init(Emulator* emu, GbaMemoryManager* memoryManager, GbaSaveType saveType);

	template<bool checkEeprom>
	__forceinline uint8_t ReadRom(uint32_t addr)
	{
		if constexpr(checkEeprom) {
			if((addr & _eepromMask) == _eepromAddr) {
				return ReadEeprom(addr);
			}
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
	
	bool IsSaveRamDirty() { return _saveRamDirty; }

	void DebugWriteRam(uint32_t addr, uint8_t value);
	AddressInfo GetRamAbsoluteAddress(uint32_t addr);
	int64_t GetRamRelativeAddress(AddressInfo& absAddress);

	void Serialize(Serializer& s) override;
};