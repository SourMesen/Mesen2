#include "pch.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbCpu.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/APU/GbApu.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbTimer.h"
#include "Gameboy/GbControlManager.h"
#include "Gameboy/GbDmaController.h"
#include "Gameboy/GbMemoryManager.h"
#include "Gameboy/GbCartFactory.h"
#include "Gameboy/GameboyHeader.h"
#include "Gameboy/GbsHeader.h"
#include "Gameboy/Carts/GbsCart.h"
#include "Gameboy/GbBootRom.h"
#include "Gameboy/GbDefaultVideoFilter.h"
#include "Gameboy/GbxFooter.h"
#include "Debugger/DebugTypes.h"
#include "Shared/CheatManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/Audio/AudioPlayerTypes.h"
#include "Shared/BaseControlManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/FirmwareHelper.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/Serializer.h"
#include "Utilities/CRC32.h"

Gameboy::Gameboy(Emulator* emu, bool allowSgb)
{
	_emu = emu;
	_allowSgb = allowSgb;
}

Gameboy::~Gameboy()
{
	delete[] _cartRam;
	delete[] _prgRom;

	delete[] _spriteRam;
	delete[] _videoRam;

	delete[] _highRam;
	delete[] _workRam;

	delete[] _bootRom;
}

void Gameboy::Init(GbCart* cart, std::vector<uint8_t>& romData, uint32_t cartRamSize, bool hasBattery)
{
	_cart.reset(cart);

	_ppu.reset(new GbPpu());
	_apu.reset(new GbApu());
	_cpu.reset(new GbCpu());
	_memoryManager.reset(new GbMemoryManager());
	_timer.reset(new GbTimer());
	_dmaController.reset(new GbDmaController());
	_controlManager.reset(new GbControlManager(_emu, this));

	_prgRomSize = (uint32_t)romData.size();
	_prgRom = new uint8_t[_prgRomSize];
	memcpy(_prgRom, romData.data(), romData.size());
	_emu->RegisterMemory(MemoryType::GbPrgRom, _prgRom, _prgRomSize);

	_cartRamSize = cartRamSize;
	_cartRam = new uint8_t[_cartRamSize];
	_emu->RegisterMemory(MemoryType::GbCartRam, _cartRam, _cartRamSize);

	_hasBattery = hasBattery;

	bool cgbMode = _model == GameboyModel::GameboyColor;
	_workRamSize = cgbMode ? 0x8000 : 0x2000;
	_videoRamSize = cgbMode ? 0x4000 : 0x2000;

	_workRam = new uint8_t[_workRamSize];
	_emu->RegisterMemory(MemoryType::GbWorkRam, _workRam, _workRamSize);

	_videoRam = new uint8_t[_videoRamSize];
	_emu->RegisterMemory(MemoryType::GbVideoRam, _videoRam, _videoRamSize);

	_spriteRam = new uint8_t[Gameboy::SpriteRamSize];
	_emu->RegisterMemory(MemoryType::GbSpriteRam, _spriteRam, Gameboy::SpriteRamSize);

	_highRam = new uint8_t[Gameboy::HighRamSize];
	_emu->RegisterMemory(MemoryType::GbHighRam, _highRam, Gameboy::HighRamSize);

	_bootRomSize = 0;

	EmuSettings* settings = _emu->GetSettings();
	GameboyConfig cfg = settings->GetGameboyConfig();

	FirmwareType type = FirmwareType::Gameboy;
	if(_model == GameboyModel::SuperGameboy) {
		type = cfg.UseSgb2 ? FirmwareType::Sgb2GameboyCpu : FirmwareType::Sgb1GameboyCpu;
	} else if(_model == GameboyModel::GameboyColor) {
		type = FirmwareType::GameboyColor;
	}

	_bootRomSize = cgbMode ? 9 * 256 : 256;
	if(GetRomFormat() == RomFormat::Gbs || !FirmwareHelper::LoadGbBootRom(_emu, &_bootRom, type)) {
		switch(_model) {
			default:
			case GameboyModel::Gameboy:
				_bootRom = new uint8_t[_bootRomSize];
				memcpy(_bootRom, dmgBootRom, _bootRomSize);
				break;

			case GameboyModel::GameboyColor:
				_bootRom = new uint8_t[_bootRomSize];
				memcpy(_bootRom, cgbBootRom, _bootRomSize);
				break;

			case GameboyModel::SuperGameboy:
				_bootRom = new uint8_t[_bootRomSize];
				if(cfg.UseSgb2) {
					memcpy(_bootRom, sgb2BootRom, _bootRomSize);
				} else {
					memcpy(_bootRom, sgbBootRom, _bootRomSize);
				}
				break;
		}
	}
	
	_emu->RegisterMemory(MemoryType::GbBootRom, _bootRom, _bootRomSize);

	InitializeRam(_cartRam, _cartRamSize);
	InitializeRam(_workRam, _workRamSize);
	InitializeRam(_spriteRam, Gameboy::SpriteRamSize);
	InitializeRam(_highRam, Gameboy::HighRamSize);
	InitializeRam(_videoRam, _videoRamSize);

	LoadBattery();
	if(!_allowSgb) {
		PowerOn(nullptr);
	}
}

void Gameboy::PowerOn(SuperGameboy *sgb)
{
	_superGameboy = sgb;

	_timer->Init(_memoryManager.get(), _apu.get());
	_apu->Init(_emu, this);
	_cart->Init(this, _memoryManager.get());
	_memoryManager->Init(_emu, this, _cart.get(), _ppu.get(), _apu.get(), _timer.get(), _dmaController.get());
	_cpu->Init(_emu, this, _memoryManager.get());
	_ppu->Init(_emu, this, _memoryManager.get(), _dmaController.get(), _videoRam, _spriteRam);
	_dmaController->Init(this, _memoryManager.get(), _ppu.get(), _cpu.get());

	_cpu->PowerOn();
}

void Gameboy::Run(uint64_t runUntilClock)
{
	while(_cpu->GetCycleCount() < runUntilClock) {
		_cpu->Exec();
	}
}

void Gameboy::LoadBattery()
{
	if(_hasBattery) {
		_emu->GetBatteryManager()->LoadBattery(".srm", _cartRam, _cartRamSize);
	}
}

void Gameboy::SaveBattery()
{
	if(_hasBattery) {
		_emu->GetBatteryManager()->SaveBattery(".srm", _cartRam, _cartRamSize);
	}
}

GbState Gameboy::GetState()
{
	GbState state;
	state.Type = IsCgb() ? GbType::Cgb : GbType::Gb;
	state.Cpu = _cpu->GetState();
	state.Ppu = _ppu->GetState();
	state.Apu = _apu->GetState();
	state.MemoryManager = _memoryManager->GetState();
	state.ControlManager = _controlManager->GetState();
	state.Dma = _dmaController->GetState();
	state.Timer = _timer->GetState();
	state.HasBattery = _hasBattery;
	return state;
}

void Gameboy::GetConsoleState(BaseState& state, ConsoleType consoleType)
{
	(GbState&)state = GetState();
}

uint32_t Gameboy::DebugGetMemorySize(MemoryType type)
{
	switch(type) {
		case MemoryType::GbPrgRom: return _prgRomSize;
		case MemoryType::GbWorkRam: return _workRamSize;
		case MemoryType::GbCartRam: return _cartRamSize;
		case MemoryType::GbHighRam: return Gameboy::HighRamSize;
		case MemoryType::GbBootRom: return _bootRomSize;
		case MemoryType::GbVideoRam: return _videoRamSize;
		case MemoryType::GbSpriteRam: return Gameboy::SpriteRamSize;
		default: return 0;
	}
}

uint8_t* Gameboy::DebugGetMemory(MemoryType type)
{
	switch(type) {
		case MemoryType::GbPrgRom: return _prgRom;
		case MemoryType::GbWorkRam: return _workRam;
		case MemoryType::GbCartRam: return _cartRam;
		case MemoryType::GbHighRam: return _highRam;
		case MemoryType::GbBootRom: return _bootRom;
		case MemoryType::GbVideoRam: return _videoRam;
		case MemoryType::GbSpriteRam: return _spriteRam;
		default: return nullptr;
	}
}

GbMemoryManager* Gameboy::GetMemoryManager()
{
	return _memoryManager.get();
}

Emulator* Gameboy::GetEmulator()
{
	return _emu;
}

GbPpu* Gameboy::GetPpu()
{
	return _ppu.get();
}

GbCpu* Gameboy::GetCpu()
{
	return _cpu.get();
}

GbTimer* Gameboy::GetTimer()
{
	return _timer.get();
}

void Gameboy::GetSoundSamples(int16_t* &samples, uint32_t& sampleCount)
{
	_apu->GetSoundSamples(samples, sampleCount);
}

AddressInfo Gameboy::GetAbsoluteAddress(uint16_t addr)
{
	AddressInfo addrInfo = { -1, MemoryType::None };

	if(addr >= 0xFF80 && addr <= 0xFFFE) {
		addrInfo.Address = addr & 0x7F;
		addrInfo.Type = MemoryType::GbHighRam;
		return addrInfo;
	} else if(addr >= 0xFE00 && addr < 0xFF00) {
		addrInfo.Address = addr & 0x7F;
		addrInfo.Type = MemoryType::GbSpriteRam;
		return addrInfo;
	} else if(addr >= 0xFF00) {
		//Return empty for registers at >= $FF00 (needed to prevent UI from showing work ram addresses)
		return addrInfo;
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {
		addrInfo.Address = (addr & 0x1FFF) | (_ppu->GetStateRef().CgbVramBank << 13);
		addrInfo.Type = MemoryType::GbVideoRam;
		return addrInfo;
	}

	uint8_t* ptr = _memoryManager->GetMappedBlock(addr);

	if(!ptr) {
		return addrInfo;
	}

	ptr += (addr & 0xFF);

	if(ptr >= _prgRom && ptr < _prgRom + _prgRomSize) {
		addrInfo.Address = (int32_t)(ptr - _prgRom);
		addrInfo.Type = MemoryType::GbPrgRom;
	} else if(ptr >= _workRam && ptr < _workRam + _workRamSize) {
		addrInfo.Address = (int32_t)(ptr - _workRam);
		addrInfo.Type = MemoryType::GbWorkRam;
	} else if(ptr >= _cartRam && ptr < _cartRam + _cartRamSize) {
		addrInfo.Address = (int32_t)(ptr - _cartRam);
		addrInfo.Type = MemoryType::GbCartRam;
	} else if(ptr >= _bootRom && ptr < _bootRom + _bootRomSize) {
		addrInfo.Address = (int32_t)(ptr - _bootRom);
		addrInfo.Type = MemoryType::GbBootRom;
	}
	return addrInfo;
}

int32_t Gameboy::GetRelativeAddress(AddressInfo& absAddress)
{
	if(absAddress.Type == MemoryType::GbHighRam) {
		return 0xFF80 | (absAddress.Address & 0x7F);
	}

	for(int32_t i = 0; i < 0x10000; i += 0x100) {
		AddressInfo blockAddr = GetAbsoluteAddress(i);
		if(blockAddr.Type == absAddress.Type && (blockAddr.Address & ~0xFF) == (absAddress.Address & ~0xFF)) {
			return i | (absAddress.Address & 0xFF);
		}
	}

	return -1;
}

bool Gameboy::IsCpuStopped()
{
	return _cpu->GetState().Stopped;
}

bool Gameboy::IsCgb()
{
	return _model == GameboyModel::GameboyColor;
}

bool Gameboy::IsSgb()
{
	return _model == GameboyModel::SuperGameboy;
}

SuperGameboy* Gameboy::GetSgb()
{
	return _superGameboy;
}

uint64_t Gameboy::GetCycleCount()
{
	return _cpu->GetCycleCount();
}

uint64_t Gameboy::GetApuCycleCount()
{
	return _memoryManager->GetApuCycleCount();
}

void Gameboy::Serialize(Serializer& s)
{
	SV(_cpu);
	SV(_ppu);
	SV(_apu);
	SV(_cart); //Process cart before memory manager to ensure mappings are updated properly
	SV(_memoryManager);
	SV(_timer);
	SV(_dmaController);
	SV(_hasBattery);

	SVArray(_cartRam, _cartRamSize);
	SVArray(_workRam, _workRamSize);
	SVArray(_videoRam, _videoRamSize);
	SVArray(_spriteRam, Gameboy::SpriteRamSize);
	SVArray(_highRam, Gameboy::HighRamSize);

	SV(_controlManager);
}

SaveStateCompatInfo Gameboy::ValidateSaveStateCompatibility(ConsoleType stateConsoleType)
{
	if(stateConsoleType == ConsoleType::Snes) {
		return { true, "", "cart.gameboy." };
	}

	return {};
}

void Gameboy::Reset()
{
	//The GB has no reset button, behave like power cycle
	_emu->ReloadRom(true);
}

LoadRomResult Gameboy::LoadRom(VirtualFile& romFile)
{
	vector<uint8_t> romData;
	romFile.ReadFile(romData);

	if(romData.size() < Gameboy::HeaderOffset + sizeof(GameboyHeader)) {
		return LoadRomResult::Failure;
	}

	GbsHeader gbsHeader = {};
	memcpy(&gbsHeader, romData.data(), sizeof(GbsHeader));
	if(!_allowSgb && memcmp(gbsHeader.Header, "GBS", sizeof(gbsHeader.Header)) == 0) {
		//GBS music file
		uint16_t loadAddr = gbsHeader.LoadAddress[0] | (gbsHeader.LoadAddress[1] << 8);

		//Pad start with 0s until load address
		vector<uint8_t> gbsRomData = vector<uint8_t>(loadAddr, 0);
		gbsRomData.insert(gbsRomData.end(), romData.begin() + sizeof(GbsHeader), romData.end());
		if((gbsRomData.size() & 0x3FFF) != 0) {
			//Pad to multiple of 16kb
			gbsRomData.insert(gbsRomData.end(), 0x4000 - (gbsRomData.size() & 0x3FFF), 0);
		}
		
		MessageManager::Log("-----------------------------");
		MessageManager::Log("File: " + romFile.GetFileName());

		GbsCart* cart = new GbsCart(gbsHeader);
		_model = GameboyModel::GameboyColor;
		Init(cart, gbsRomData, 0x5000, false);
		cart->InitPlayback(gbsHeader.FirstTrack - 1);

		return LoadRomResult::Success;
	} else {
		GbxFooter gbxFooter = {};
		gbxFooter.Init(romData);

		if((romData.size() & 0x3FFF) != 0) {
			//Pad to multiple of 16kb
			romData.insert(romData.end(), 0x4000 - (romData.size() & 0x3FFF), 0);
		}

		GameboyHeader header = GetHeader(romData.data(), (uint32_t)romData.size());

		_model = GetEffectiveModel(header);
		if(_allowSgb && _model != GameboyModel::SuperGameboy) {
			return LoadRomResult::UnknownType;
		}

		MessageManager::Log("-----------------------------");
		MessageManager::Log("File: " + romFile.GetFileName());
		MessageManager::Log("Game: " + header.GetCartName());
		MessageManager::Log("Cart Type: " + std::to_string(header.CartType));
		switch((CgbCompat)((int)header.CgbFlag & 0xC0)) {
			case CgbCompat::Gameboy: MessageManager::Log("Supports: Game Boy"); break;
			case CgbCompat::GameboyColorSupport: MessageManager::Log("Supports: Game Boy Color (compatible with GB)"); break;
			case CgbCompat::GameboyColorExclusive: MessageManager::Log("Supports: Game Boy Color only"); break;
		}
		if(header.SgbFlag == 0x03) {
			MessageManager::Log("Supports: Super Game Boy");
		}
		MessageManager::Log("File size: " + std::to_string(romData.size() / 1024) + " KB");

		if(header.GetCartRamSize() > 0) {
			string sizeString = header.GetCartRamSize() > 1024 ? std::to_string(header.GetCartRamSize() / 1024) + " KB" : std::to_string(header.GetCartRamSize()) + " bytes";
			MessageManager::Log("Cart RAM size: " + sizeString + (header.HasBattery() ? " (with battery)" : ""));
		}

		GbCart* cart = GbCartFactory::CreateCart(_emu, header, gbxFooter, romData);
		
		MessageManager::Log("-----------------------------");

		if(cart) {
			if(gbxFooter.IsValid()) {
				Init(cart, romData, gbxFooter.GetRamSize(), gbxFooter.HasBattery());
			} else {
				Init(cart, romData, header.GetCartRamSize(), header.HasBattery());
			}
			return LoadRomResult::Success;
		} else {
			MessageManager::DisplayMessage("Error", "Unsupported cart type: " + (gbxFooter.IsValid() ? gbxFooter.GetMapperId() : std::to_string(header.CartType)));
			return LoadRomResult::Failure;
		}
	}

	return LoadRomResult::UnknownType;
}

GameboyHeader Gameboy::GetHeader(uint8_t* romData, uint32_t romSize)
{
	GameboyHeader header = {};
	memcpy(&header, romData + Gameboy::HeaderOffset, sizeof(GameboyHeader));

	if(romSize > 0x8000) {
		uint32_t logoPosition = (uint32_t)(romSize - 0x8000 + 0x104);
		if(CRC32::GetCRC(&romData[logoPosition], 0x30) == 0x46195417) {
			//Found logo at the end of the rom, use this header instead
			//MMM01 games have the header here because of their default mappings at power on
			int offset = (int)(romSize - 0x8000 + Gameboy::HeaderOffset);
			memcpy(&header, romData + offset, sizeof(GameboyHeader));

			uint8_t headerChecksum = 0;
			for(int i = 0; i < sizeof(GameboyHeader) - 3; i++) {
				headerChecksum = headerChecksum - romData[offset + i] - 1;
			}
			if(header.CartType == 0x11) {
				//Mani 4-in-1 has carttype set as $11, but is actually a MMM01 cart
				header.CartType = 0x0B;
			}

			if(headerChecksum != header.HeaderChecksum) {
				//Invalid header, ignore it and use the default header location instead
				memcpy(&header, romData + Gameboy::HeaderOffset, sizeof(GameboyHeader));
			}
		}
	}

	return header;
}

GameboyHeader Gameboy::GetHeader()
{
	return GetHeader(_prgRom, _prgRomSize);
}

GameboyModel Gameboy::GetEffectiveModel(GameboyHeader& header)
{
	EmuSettings* settings = _emu->GetSettings();
	GameboyConfig cfg = settings->GetGameboyConfig();
	GameboyModel model = cfg.Model;
	CgbCompat cgbFlag = (CgbCompat)((int)header.CgbFlag & 0xC0);
	bool supportsSgb = header.SgbFlag == 0x03;
	switch(model) {
		case GameboyModel::AutoFavorGb:
			if(cgbFlag == CgbCompat::GameboyColorExclusive) {
				model = GameboyModel::GameboyColor;
			} else {
				model = GameboyModel::Gameboy;
			}
			break;

		case GameboyModel::AutoFavorSgb:
			if(cgbFlag == CgbCompat::GameboyColorExclusive) {
				model = GameboyModel::GameboyColor;
			} else {
				model = GameboyModel::SuperGameboy;
			}
			break;

		case GameboyModel::AutoFavorGbc:
			if(supportsSgb && cgbFlag == CgbCompat::Gameboy) {
				model = GameboyModel::SuperGameboy;
			} else {
				model = GameboyModel::GameboyColor;
			}
			break;
	}

	if(!_allowSgb && model == GameboyModel::SuperGameboy) {
		//SGB isn't available, use gameboy color mode instead
		model = GameboyModel::GameboyColor;
	}

	return model;
}

void Gameboy::RunFrame()
{
	uint32_t frameCount = _ppu->GetFrameCount();
	while(frameCount == _ppu->GetFrameCount()) {
		_cpu->Exec();
	}

	_apu->Run();
	_apu->PlayQueuedAudio();
}

void Gameboy::ProcessEndOfFrame()
{
	_controlManager->UpdateControlDevices();
	_controlManager->UpdateInputState();
}

BaseControlManager* Gameboy::GetControlManager()
{
	return _controlManager.get();
}

ConsoleType Gameboy::GetConsoleType()
{
	return ConsoleType::Gameboy;
}

double Gameboy::GetFps()
{
	return 59.72750056960583;
}

PpuFrameInfo Gameboy::GetPpuFrame()
{
	PpuFrameInfo frame;
	frame.FrameBuffer = (uint8_t*)_ppu->GetOutputBuffer();
	frame.FrameCount = _ppu->GetFrameCount();
	frame.Width = 160;
	frame.Height = 144;
	frame.FrameBufferSize = frame.Width * frame.Height * sizeof(uint16_t);
	frame.FirstScanline = 0;
	frame.ScanlineCount = 154;
	frame.CycleCount = 456;
	return frame;
}

vector<CpuType> Gameboy::GetCpuTypes()
{
	return { CpuType::Gameboy };
}

AddressInfo Gameboy::GetAbsoluteAddress(AddressInfo& relAddress)
{
	return GetAbsoluteAddress(relAddress.Address);
}

AddressInfo Gameboy::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	return { GetRelativeAddress(absAddress), MemoryType::GameboyMemory };
}

uint64_t Gameboy::GetMasterClock()
{
	return _cpu->GetCycleCount();
}

uint32_t Gameboy::GetMasterClockRate()
{
	return _memoryManager->IsHighSpeed() ? 4194304*2 : 4194304;
}

BaseVideoFilter* Gameboy::GetVideoFilter(bool getDefaultFilter)
{
	if(getDefaultFilter || GetRomFormat() == RomFormat::Gbs) {
		return new GbDefaultVideoFilter(_emu, false);
	}

	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;

	switch(filterType) {
		case VideoFilterType::NtscBlargg:
		case VideoFilterType::NtscBisqwit:
			return new GbDefaultVideoFilter(_emu, true);

		default:
			return new GbDefaultVideoFilter(_emu, false);
	}
}

RomFormat Gameboy::GetRomFormat()
{
	return dynamic_cast<GbsCart*>(_cart.get()) ? RomFormat::Gbs : RomFormat::Gb;
}

AudioTrackInfo Gameboy::GetAudioTrackInfo()
{
	GbsCart* cart = dynamic_cast<GbsCart*>(_cart.get());
	if(cart) {
		return cart->GetAudioTrackInfo();
	}
	return {};
}

void Gameboy::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	GbsCart* cart = dynamic_cast<GbsCart*>(_cart.get());
	if(cart) {
		cart->ProcessAudioPlayerAction(p);
	}
}

void Gameboy::InitGbsPlayback(uint8_t selectedTrack)
{
	GbsCart* cart = dynamic_cast<GbsCart*>(_cart.get());
	if(cart) {
		cart->InitPlayback(selectedTrack);
	}
}

ConsoleRegion Gameboy::GetRegion()
{
	return ConsoleRegion::Ntsc;
}

void Gameboy::RefreshRamCheats()
{
	//Used to refresh gameshark codes when vertical blank IRQ is triggered
	_emu->GetCheatManager()->RefreshRamCheats(CpuType::Gameboy);
	for(InternalCheatCode& code : _emu->GetCheatManager()->GetRamRefreshCheats(CpuType::Gameboy)) {
		if(!code.IsAbsoluteAddress) {
			_memoryManager->DebugWrite(code.Address, code.Value);
		}
	}
}

void Gameboy::InitializeRam(void* data, uint32_t length)
{
	EmuSettings* settings = _emu->GetSettings();
	settings->InitializeRam(settings->GetGameboyConfig().RamPowerOnState, data, length);
}
