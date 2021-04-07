#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "GameboyHeader.h"
#include "SettingTypes.h"
#include "GbTypes.h"
#include "IConsole.h"
#include "Utilities/ISerializable.h"

class Emulator;
class GbPpu;
class GbApu;
class GbCpu;
class GbCart;
class GbTimer;
class GbMemoryManager;
class GbDmaController;
class GbControlManager;
class SuperGameboy;
class VirtualFile;

class Gameboy : public IConsole
{
private:
	static constexpr int SpriteRamSize = 0xA0;
	static constexpr int HighRamSize = 0x7F;

	Emulator* _emu = nullptr;
	SuperGameboy* _superGameboy = nullptr;
	bool _allowSgb = false;

	unique_ptr<GbMemoryManager> _memoryManager;
	unique_ptr<GbCpu> _cpu;
	unique_ptr<GbPpu> _ppu;
	unique_ptr<GbApu> _apu;
	unique_ptr<GbCart> _cart;
	unique_ptr<GbTimer> _timer;
	unique_ptr<GbDmaController> _dmaController;
	shared_ptr<GbControlManager> _controlManager;

	GameboyModel _model = GameboyModel::Auto;

	bool _hasBattery = false;

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

	Gameboy(Emulator* emu, bool allowSgb);
	virtual ~Gameboy();

	void Init(GbCart* cart, std::vector<uint8_t>& romData, GameboyHeader& header);
	void PowerOn(SuperGameboy* sgb);

	void Run(uint64_t runUntilClock);
	
	void LoadBattery();
	void SaveBattery();

	GbPpu* GetPpu();
	GbCpu* GetCpu();
	void GetSoundSamples(int16_t* &samples, uint32_t& sampleCount);
	GbState GetState();
	GameboyHeader GetHeader();

	uint32_t DebugGetMemorySize(SnesMemoryType type);
	uint8_t* DebugGetMemory(SnesMemoryType type);
	GbMemoryManager* GetMemoryManager();
	AddressInfo GetAbsoluteAddress(uint16_t addr);
	int32_t GetRelativeAddress(AddressInfo& absAddress);

	bool IsCgb();
	bool IsSgb();
	SuperGameboy* GetSgb();

	uint64_t GetCycleCount();
	uint64_t GetApuCycleCount();
	
	void ProcessEndOfFrame();

	void Serialize(Serializer& s) override;

	// Inherited via IConsole
	virtual void Stop() override;
	virtual void Reset() override;
	virtual void OnBeforeRun() override;
	virtual bool LoadRom(VirtualFile& romFile, VirtualFile& patchFile) override;
	virtual void Init() override;
	virtual void RunFrame() override;
	virtual shared_ptr<IControlManager> GetControlManager() override;
	virtual ConsoleType GetConsoleType() override;
	virtual double GetFrameDelay() override;
	virtual double GetFps() override;
	virtual RomInfo GetRomInfo() override;
	virtual void RunSingleFrame() override;
	virtual PpuFrameInfo GetPpuFrame() override;
	virtual vector<CpuType> GetCpuTypes() override;

	// Inherited via IConsole
	virtual AddressInfo GetAbsoluteAddress(AddressInfo relAddress) override;
	virtual AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) override;

	// Inherited via IConsole
	virtual uint64_t GetMasterClock() override;
};