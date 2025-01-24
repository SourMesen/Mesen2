#include "pch.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaControlManager.h"
#include "GBA/GbaDmaController.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaTimer.h"
#include "GBA/GbaSerial.h"
#include "GBA/GbaRomPrefetch.h"
#include "GBA/GbaDefaultVideoFilter.h"
#include "GBA/Cart/GbaCart.h"
#include "GBA/APU/GbaApu.h"
#include "GBA/Debugger/DummyGbaCpu.h"
#include "Debugger/DebugTypes.h"
#include "Shared/CheatManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/Audio/AudioPlayerTypes.h"
#include "Shared/BaseControlManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/FirmwareHelper.h"
#include "Shared/MessageManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/Serializer.h"
#include "Utilities/StringUtilities.h"

static bool _needStaticInit = true;
static SimpleLock _staticInitLock;

GbaConsole::GbaConsole(Emulator* emu)
{
	_emu = emu;

	if(_needStaticInit) {
		auto lock = _staticInitLock.AcquireSafe();
		if(_needStaticInit) {
			GbaCpu::StaticInit();
			DummyGbaCpu::StaticInit();
			_needStaticInit = false;
		}
	}
}

GbaConsole::~GbaConsole()
{
	delete[] _saveRam;
	delete[] _prgRom;

	delete[] _spriteRam;
	delete[] _paletteRam;
	delete[] _videoRam;

	delete[] _intWorkRam;
	delete[] _extWorkRam;

	delete[] _bootRom;
}

LoadRomResult GbaConsole::LoadRom(VirtualFile& romFile)
{
	vector<uint8_t> romData;
	romFile.ReadFile(romData);

	if(romData.size() < 0xC0) {
		return LoadRomResult::Failure;
	}

	InitCart(romFile, romData);

	_prgRomSize = (uint32_t)romData.size();
	_prgRom = new uint8_t[_prgRomSize];
	memcpy(_prgRom, romData.data(), _prgRomSize);
	_emu->RegisterMemory(MemoryType::GbaPrgRom, _prgRom, _prgRomSize);

	_bootRom = new uint8_t[GbaConsole::BootRomSize];
	if(!FirmwareHelper::LoadGbaBootRom(_emu, &_bootRom)) {
		memset(_bootRom, 0, GbaConsole::BootRomSize);
	}
	_emu->RegisterMemory(MemoryType::GbaBootRom, _bootRom, GbaConsole::BootRomSize);

	_intWorkRam = new uint8_t[GbaConsole::IntWorkRamSize];
	InitializeRam(_intWorkRam, GbaConsole::IntWorkRamSize);
	_emu->RegisterMemory(MemoryType::GbaIntWorkRam, _intWorkRam, GbaConsole::IntWorkRamSize);

	_extWorkRam = new uint8_t[GbaConsole::ExtWorkRamSize];
	InitializeRam(_extWorkRam, GbaConsole::ExtWorkRamSize);
	_emu->RegisterMemory(MemoryType::GbaExtWorkRam, _extWorkRam, GbaConsole::ExtWorkRamSize);

	_videoRam = new uint16_t[GbaConsole::VideoRamSize / 2];
	InitializeRam(_videoRam, GbaConsole::VideoRamSize);
	_emu->RegisterMemory(MemoryType::GbaVideoRam, _videoRam, GbaConsole::VideoRamSize);

	_spriteRam = new uint32_t[GbaConsole::SpriteRamSize / 4];
	InitializeRam(_spriteRam, GbaConsole::SpriteRamSize);
	_emu->RegisterMemory(MemoryType::GbaSpriteRam, _spriteRam, GbaConsole::SpriteRamSize);

	_paletteRam = new uint16_t[GbaConsole::PaletteRamSize / 2];
	InitializeRam(_paletteRam, GbaConsole::PaletteRamSize);
	_emu->RegisterMemory(MemoryType::GbaPaletteRam, _paletteRam, GbaConsole::PaletteRamSize);

	_cpu.reset(new GbaCpu());
	_ppu.reset(new GbaPpu());
	_dmaController.reset(new GbaDmaController());
	_timer.reset(new GbaTimer());
	_apu.reset(new GbaApu());
	_cart.reset(new GbaCart());
	_serial.reset(new GbaSerial());
	_controlManager.reset(new GbaControlManager(_emu, this));
	_prefetch.reset(new GbaRomPrefetch());

	_memoryManager.reset(new GbaMemoryManager(_emu, this, _ppu.get(), _dmaController.get(), _controlManager.get(), _timer.get(), _apu.get(), _cart.get(), _serial.get(), _prefetch.get()));

	_prefetch->Init(_memoryManager.get(), _cpu.get());
	_cart->Init(_emu, this, _memoryManager.get(), _saveType, _rtcType, _cartType);
	_ppu->Init(_emu, this, _memoryManager.get());
	_apu->Init(_emu, this, _dmaController.get(), _memoryManager.get());
	_timer->Init(_memoryManager.get(), _apu.get());
	_dmaController->Init(_cpu.get(), _memoryManager.get(), _prefetch.get());
	_cpu->Init(_emu, _memoryManager.get(), _prefetch.get());
	_serial->Init(_emu, _memoryManager.get());
	_controlManager->Init(_memoryManager.get());
	
	LoadBattery();

	_cpu->PowerOn();

	return LoadRomResult::Success;
}

void GbaConsole::InitCart(VirtualFile& romFile, vector<uint8_t>& romData)
{
	string title = StringUtilities::GetString(&romData[0xA0], 12);
	string gameCode = StringUtilities::GetString(&romData[0xAC], 4);
	string makerCode = StringUtilities::GetString(&romData[0xB0], 2);

	MessageManager::Log("-----------------------------");
	MessageManager::Log("File: " + romFile.GetFileName());
	MessageManager::Log("Title: " + title);
	MessageManager::Log("Game Code: " + gameCode);
	MessageManager::Log("Maker Code: " + makerCode);
	
	if(gameCode.size() > 0 && gameCode[0] == 'F') {
		MessageManager::Log("Classic series game detected.");
		if(romData.size() == 0x100000) {
			//Mirror up to 4 MB to fix input problems
			romData.insert(romData.end(), romData.begin(), romData.end());
			romData.insert(romData.end(), romData.begin(), romData.end());
		}
	}

	_cartType = GbaCartridgeType::Default;

	if(gameCode == "KYGE" || gameCode == "KHPJ") {
		MessageManager::Log("Tilt Sensor detected.");
		_cartType = GbaCartridgeType::TiltSensor;
	}

	_rtcType = _emu->GetSettings()->GetGbaConfig().RtcType;
	if(_rtcType == GbaRtcType::AutoDetect) {
		string rtcMarker = "SIIRTC_V001";
		if(std::search(romData.begin(), romData.end(), rtcMarker.begin(), rtcMarker.end()) != romData.end()) {
			_rtcType = GbaRtcType::Enabled;
		} else {
			_rtcType = GbaRtcType::Disabled;
		}
	}

	if(_rtcType == GbaRtcType::Enabled) {
		MessageManager::Log("RTC enabled");
	}

	InitSaveRam(gameCode, romData);

	MessageManager::Log("-----------------------------");
}

void GbaConsole::InitSaveRam(string& gameCode, vector<uint8_t>& romData)
{
	_saveType = _emu->GetSettings()->GetGbaConfig().SaveType;

	if(_saveType == GbaSaveType::AutoDetect) {
		if(gameCode == "A2YE") {
			//Force no backup data for Top Gun, otherwise A button doesn't work in menu
			_saveType = GbaSaveType::None;
		} else if(gameCode == "AYGE" || gameCode == "ALUE" || gameCode == "ALUP") {
			//Force 512-byte eeprom for Gauntlet (AYGE) and Super Monkey Ball Jr. (ALUE & ALUP) (auto-detect logic doesn't work)
			_saveType = GbaSaveType::Eeprom512;
		} else if(gameCode == "AI2E") {
			//Iridion II crashes if it has SRAM, force it to none
			_saveType = GbaSaveType::None;
		} else {
			auto checkMarker = [&](string marker, GbaSaveType type) {
				if(_saveType == GbaSaveType::AutoDetect && std::search(romData.begin(), romData.end(), marker.begin(), marker.end()) != romData.end()) {
					_saveType = type;
				}
			};

			checkMarker("SRAM_V", GbaSaveType::Sram);
			checkMarker("EEPROM_V", GbaSaveType::EepromUnknown);
			checkMarker("FLASH_V", GbaSaveType::Flash64);
			checkMarker("FLASH512_V", GbaSaveType::Flash64);
			checkMarker("FLASH1M_V", GbaSaveType::Flash128);

			if(_saveType == GbaSaveType::AutoDetect) {
				//Default to SRAM when nothing is found, for best compatibility
				MessageManager::Log("Save type auto-detect failed - using SRAM.");
				_saveType = GbaSaveType::Sram;
			}
		}
	}

	switch(_saveType) {
		default:
		case GbaSaveType::None:
			MessageManager::Log("Save type: None");
			_saveRamSize = 0;
			break;

		case GbaSaveType::EepromUnknown:
			MessageManager::Log("Save type: EEPROM (auto-detect size)");
			_saveRamSize = 0x2000;
			break;

		case GbaSaveType::Eeprom512:
			MessageManager::Log("Save type: EEPROM (512 bytes)");
			_saveRamSize = 0x200;
			break;

		case GbaSaveType::Eeprom8192:
			MessageManager::Log("Save type: EEPROM (8192 bytes)");
			_saveRamSize = 0x2000;
			break;

		case GbaSaveType::Sram:
			MessageManager::Log("Save type: SRAM");
			_saveRamSize = 0x8000;
			break;

		case GbaSaveType::Flash64:
			MessageManager::Log("Save type: Flash (64 KB)");
			_saveRamSize = 0x10000;
			break;

		case GbaSaveType::Flash128:
			MessageManager::Log("Save type: Flash (128 KB)");
			_saveRamSize = 0x20000;
			break;
	}

	_saveRam = new uint8_t[_saveRamSize];
	//SRAM and Flash are initialized to 0xFF?
	_emu->GetSettings()->InitializeRam(RamState::AllOnes, _saveRam, _saveRamSize);
	_emu->RegisterMemory(MemoryType::GbaSaveRam, _saveRam, _saveRamSize);
}

void GbaConsole::LoadBattery()
{
	_cart->LoadBattery();
}

void GbaConsole::SaveBattery()
{
	_cart->SaveBattery();
}

GbaState GbaConsole::GetState()
{
	GbaState state;
	state.Cpu = _cpu->GetState();
	state.Ppu = _ppu->GetState();
	state.Apu = _apu->GetState();
	state.MemoryManager = _memoryManager->GetState();
	state.Dma = _dmaController->GetState();
	state.Timer = _timer->GetState();
	state.Prefetch = _prefetch->GetState();
	state.ControlManager = _controlManager->GetState();
	return state;
}

void GbaConsole::GetConsoleState(BaseState& state, ConsoleType consoleType)
{
	(GbaState&)state = GetState();
}

GbaMemoryManager* GbaConsole::GetMemoryManager()
{
	return _memoryManager.get();
}

Emulator* GbaConsole::GetEmulator()
{
	return _emu;
}

GbaCpu* GbaConsole::GetCpu()
{
	return _cpu.get();
}

GbaPpu* GbaConsole::GetPpu()
{
	return _ppu.get();
}

GbaDmaController* GbaConsole::GetDmaController()
{
	return _dmaController.get();
}

void GbaConsole::Reset()
{
	//The GB has no reset button, behave like power cycle
	_emu->ReloadRom(true);
}

void GbaConsole::RunFrame()
{
	uint32_t frameCount = _ppu->GetFrameCount();
	uint32_t& newCount = _ppu->GetState().FrameCount;

	if(_emu->IsDebugging()) {
		if(_memoryManager->UseInlineHalt()) {
			while(frameCount == newCount) {
				_cpu->Exec<true, true>();
			}
		} else {
			while(frameCount == newCount) {
				_cpu->Exec<false, true>();
			}
		}
	} else {
		if(_memoryManager->UseInlineHalt()) {
			while(frameCount == newCount) {
				_cpu->Exec<true, false>();
			}
		} else {
			while(frameCount == newCount) {
				_cpu->Exec<false, false>();
			}
		}
	}

	_apu->Run();
	_apu->PlayQueuedAudio();
}

void GbaConsole::ProcessEndOfFrame()
{
	_controlManager->UpdateControlDevices();
	_controlManager->UpdateInputState();
}

BaseControlManager* GbaConsole::GetControlManager()
{
	return _controlManager.get();
}

ConsoleType GbaConsole::GetConsoleType()
{
	return ConsoleType::Gba;
}

double GbaConsole::GetFps()
{
	return 59.72750056960583;
}

PpuFrameInfo GbaConsole::GetPpuFrame()
{
	PpuFrameInfo frame;
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer();
	frame.FrameCount = _ppu->GetFrameCount();
	frame.Width = 240;
	frame.Height = 160;
	frame.FrameBufferSize = frame.Width * frame.Height * sizeof(uint16_t);
	frame.FirstScanline = 0;
	frame.ScanlineCount = 228;
	frame.CycleCount = 308*4;
	return frame;
}

vector<CpuType> GbaConsole::GetCpuTypes()
{
	return { CpuType::Gba };
}

AddressInfo GbaConsole::GetAbsoluteAddress(AddressInfo& relAddress)
{
	return _memoryManager->GetAbsoluteAddress(relAddress.Address);
}

AddressInfo GbaConsole::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	int64_t addr = _memoryManager->GetRelativeAddress(absAddress);
	if(addr >= 0) {
		return { (int32_t)_memoryManager->GetRelativeAddress(absAddress), MemoryType::GbaMemory };
	}
	
	return { -1, MemoryType::None };
}

uint64_t GbaConsole::GetMasterClock()
{
	return _memoryManager->GetMasterClock();
}

uint32_t GbaConsole::GetMasterClockRate()
{
	return 16777216;
}

BaseVideoFilter* GbaConsole::GetVideoFilter(bool getDefaultFilter)
{
	if(getDefaultFilter) {
		return new GbaDefaultVideoFilter(_emu, false);
	}

	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;

	switch(filterType) {
		case VideoFilterType::NtscBlargg:
		case VideoFilterType::NtscBisqwit:
			return new GbaDefaultVideoFilter(_emu, true);

		default:
			return new GbaDefaultVideoFilter(_emu, false);
	}
}

RomFormat GbaConsole::GetRomFormat()
{
	return RomFormat::Gba;
}

AudioTrackInfo GbaConsole::GetAudioTrackInfo()
{
	//TODOGBA
	return {};
}

void GbaConsole::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	//TODOGBA
}

ConsoleRegion GbaConsole::GetRegion()
{
	return ConsoleRegion::Ntsc;
}

void GbaConsole::RefreshRamCheats()
{
	//TODOGBA
}

void GbaConsole::InitializeRam(void* data, uint32_t length)
{
	EmuSettings* settings = _emu->GetSettings();
	settings->InitializeRam(settings->GetGbaConfig().RamPowerOnState, data, length);
}

void GbaConsole::Serialize(Serializer& s)
{
	SV(_cpu);
	SV(_ppu);
	SV(_apu);
	SV(_cart);
	SV(_memoryManager);
	SV(_prefetch);
	SV(_timer);
	SV(_dmaController);
	SV(_serial);

	SVArray(_saveRam, _saveRamSize);
	SVArray(_intWorkRam, GbaConsole::IntWorkRamSize);
	SVArray(_extWorkRam, GbaConsole::ExtWorkRamSize);
	SVArray(_videoRam, GbaConsole::VideoRamSize / 2);
	SVArray(_spriteRam, GbaConsole::SpriteRamSize / 4);
	SVArray(_paletteRam, GbaConsole::PaletteRamSize / 2);

	SV(_controlManager);
}
