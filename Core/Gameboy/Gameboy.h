#pragma once
#include "stdafx.h"
#include "Gameboy/GameboyHeader.h"
#include "Gameboy/GbTypes.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Shared/Interfaces/IConsole.h"
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

class Gameboy final : public IConsole
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
	unique_ptr<GbControlManager> _controlManager;

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

	void Init(GbCart* cart, std::vector<uint8_t>& romData, uint32_t cartRamSize, bool hasBattery, bool supportsCgb);

public:
	static constexpr int HeaderOffset = 0x134;

	Gameboy(Emulator* emu, bool allowSgb);
	virtual ~Gameboy();

	void PowerOn(SuperGameboy* sgb);

	void Run(uint64_t runUntilClock);
	
	void LoadBattery();
	void SaveBattery() override;

	Emulator* GetEmulator();

	GbPpu* GetPpu();
	GbCpu* GetCpu();
	void GetSoundSamples(int16_t* &samples, uint32_t& sampleCount);
	GbState GetState();
	void GetConsoleState(BaseState& state, ConsoleType consoleType);
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
	void Stop() override;
	void Reset() override;
	void OnBeforeRun() override;
	LoadRomResult LoadRom(VirtualFile& romFile) override;
	void Init() override;
	void RunFrame() override;
	IControlManager* GetControlManager() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	double GetFrameDelay() override;
	double GetFps() override;
	void RunSingleFrame() override;
	PpuFrameInfo GetPpuFrame() override;
	vector<CpuType> GetCpuTypes() override;

	AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;

	BaseVideoFilter* GetVideoFilter() override;

	RomFormat GetRomFormat() override;
	AudioTrackInfo GetAudioTrackInfo() override;
	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;
};