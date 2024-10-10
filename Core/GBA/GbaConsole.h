#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Shared/Interfaces/IConsole.h"
#include "Utilities/ISerializable.h"

class Emulator;
class GbaCpu;
class GbaPpu;
class GbaApu;
class GbaCart;
class GbaMemoryManager;
class GbaControlManager;
class GbaDmaController;
class GbaTimer;
class GbaRomPrefetch;
class GbaSerial;
class VirtualFile;
class BaseControlManager;

class GbaConsole final : public IConsole
{
public:
	static constexpr int BootRomSize = 0x4000;

	static constexpr int VideoRamSize = 0x18000;
	static constexpr int SpriteRamSize = 0x400;
	static constexpr int PaletteRamSize = 0x400;

	static constexpr int IntWorkRamSize = 0x8000;
	static constexpr int ExtWorkRamSize = 0x40000;

private:

	Emulator* _emu = nullptr;

	unique_ptr<GbaCpu> _cpu;
	unique_ptr<GbaPpu> _ppu;
	unique_ptr<GbaApu> _apu;
	unique_ptr<GbaDmaController> _dmaController;
	unique_ptr<GbaTimer> _timer;
	unique_ptr<GbaMemoryManager> _memoryManager;
	unique_ptr<GbaControlManager> _controlManager;
	unique_ptr<GbaCart> _cart;
	unique_ptr<GbaSerial> _serial;
	unique_ptr<GbaRomPrefetch> _prefetch;

	GbaSaveType _saveType = GbaSaveType::AutoDetect;
	GbaRtcType _rtcType = GbaRtcType::AutoDetect;
	GbaCartridgeType _cartType = GbaCartridgeType::Default;

	uint8_t* _prgRom = nullptr;
	uint32_t _prgRomSize = 0;

	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;

	uint8_t* _intWorkRam = nullptr;
	uint8_t* _extWorkRam = nullptr;

	uint16_t* _videoRam = nullptr;
	uint32_t* _spriteRam = nullptr;
	uint16_t* _paletteRam = nullptr;

	uint8_t* _bootRom = nullptr;

	void InitSaveRam(string& gameCode, vector<uint8_t>& romData);
	void InitCart(VirtualFile& romFile, vector<uint8_t>& romData);

public:
	GbaConsole(Emulator* emu);
	~GbaConsole();

	static vector<string> GetSupportedExtensions() { return { ".gba" }; }
	static vector<string> GetSupportedSignatures() { return { }; }

	void LoadBattery();
	void SaveBattery() override;

	Emulator* GetEmulator();

	GbaCpu* GetCpu();
	GbaPpu* GetPpu();
	GbaDmaController* GetDmaController();
	GbaState GetState();
	void GetConsoleState(BaseState& state, ConsoleType consoleType) override;

	GbaMemoryManager* GetMemoryManager();

	void ProcessEndOfFrame();

	void Serialize(Serializer& s) override;

	// Inherited via IConsole
	void Reset() override;
	LoadRomResult LoadRom(VirtualFile& romFile) override;
	void RunFrame() override;
	BaseControlManager* GetControlManager() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	double GetFps() override;
	PpuFrameInfo GetPpuFrame() override;
	vector<CpuType> GetCpuTypes() override;

	AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;

	BaseVideoFilter* GetVideoFilter(bool getDefaultFilter) override;

	RomFormat GetRomFormat() override;
	AudioTrackInfo GetAudioTrackInfo() override;
	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;

	void RefreshRamCheats();

	void InitializeRam(void* data, uint32_t length);
};