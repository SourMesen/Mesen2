#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "GameboyHeader.h"
#include "../Utilities/ISerializable.h"

class Console;
class GbPpu;
class GbApu;
class GbCpu;
class GbCart;
class GbTimer;
class GbMemoryManager;
class GbDmaController;
class VirtualFile;

class Gameboy : public ISerializable
{
private:
	static constexpr int SpriteRamSize = 0xA0;
	static constexpr int HighRamSize = 0x7F;

	Console* _console = nullptr;

	unique_ptr<GbMemoryManager> _memoryManager;
	unique_ptr<GbCpu> _cpu;
	unique_ptr<GbPpu> _ppu;
	unique_ptr<GbApu> _apu;
	unique_ptr<GbCart> _cart;
	unique_ptr<GbTimer> _timer;
	unique_ptr<GbDmaController> _dmaController;

	bool _hasBattery = false;
	bool _cgbMode = false;
	bool _useBootRom = false;

	uint8_t* _prgRom = nullptr;
	uint32_t _prgRomSize = 0;

	uint8_t* _cartRam = nullptr;
	uint32_t _cartRamSize = 0;

	uint8_t* _workRam = nullptr;
	uint32_t _workRamSize = 0;

	uint8_t* _videoRam = nullptr;
	uint32_t _videoRamSize = 0;

	uint8_t* _spriteRam = nullptr;
	uint8_t* _highRam = nullptr;

	uint8_t* _bootRom = nullptr;
	uint32_t _bootRomSize = 0;

public:
	static constexpr int HeaderOffset = 0x134;

	static Gameboy* Create(Console* console, VirtualFile& romFile);
	virtual ~Gameboy();

	void PowerOn();

	void Exec();
	void Run(uint64_t masterClock);
	
	void LoadBattery();
	void SaveBattery();

	GbPpu* GetPpu();
	GbCpu* GetCpu();
	GbState GetState();
	GameboyHeader GetHeader();

	uint32_t DebugGetMemorySize(SnesMemoryType type);
	uint8_t* DebugGetMemory(SnesMemoryType type);
	GbMemoryManager* GetMemoryManager();
	AddressInfo GetAbsoluteAddress(uint16_t addr);
	int32_t GetRelativeAddress(AddressInfo& absAddress);

	bool IsCgb();
	bool UseBootRom();
	uint64_t GetCycleCount();
	uint64_t GetApuCycleCount();

	void Serialize(Serializer& s) override;
};