#include "stdafx.h"
#include "Gameboy.h"
#include "GbCpu.h"
#include "GbPpu.h"
#include "GbApu.h"
#include "GbCart.h"
#include "GbTimer.h"
#include "GbDmaController.h"
#include "DebugTypes.h"
#include "GbMemoryManager.h"
#include "GbCartFactory.h"
#include "BatteryManager.h"
#include "GameboyHeader.h"
#include "EmuSettings.h"
#include "MessageManager.h"
#include "FirmwareHelper.h"
#include "GbBootRom.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/Serializer.h"

Gameboy* Gameboy::Create(Emulator* emu, VirtualFile &romFile, bool sgbEnabled)
{
	vector<uint8_t> romData;
	romFile.ReadFile(romData);

	GameboyHeader header;
	memcpy(&header, romData.data() + Gameboy::HeaderOffset, sizeof(GameboyHeader));

	MessageManager::Log("-----------------------------");
	MessageManager::Log("File: " + romFile.GetFileName());
	MessageManager::Log("Game: " + header.GetCartName());
	MessageManager::Log("Cart Type: " + std::to_string(header.CartType));
	switch(header.CgbFlag & 0xC0) {
		case 0x00: MessageManager::Log("Supports: Game Boy"); break;
		case 0x80: MessageManager::Log("Supports: Game Boy Color (compatible with GB)"); break;
		case 0xC0: MessageManager::Log("Supports: Game Boy Color only"); break;
	}
	MessageManager::Log("File size: " + std::to_string(romData.size() / 1024) + " KB");

	if(header.GetCartRamSize() > 0) {
		string sizeString = header.GetCartRamSize() > 1024 ? std::to_string(header.GetCartRamSize() / 1024) + " KB" : std::to_string(header.GetCartRamSize()) + " bytes";
		MessageManager::Log("Cart RAM size: " + sizeString + (header.HasBattery() ? " (with battery)" : ""));
	}
	MessageManager::Log("-----------------------------");

	GbCart* cart = GbCartFactory::CreateCart(header.CartType);

	if(cart) {
		Gameboy* gb = new Gameboy();
		gb->Init(emu, cart, romData, header, sgbEnabled);
		return gb;
	}

	return nullptr;
}

void Gameboy::Init(Emulator* emu, GbCart* cart, std::vector<uint8_t>& romData, GameboyHeader& header, bool sgbEnabled)
{
	_emu = emu;
	_cart.reset(cart);

	_prgRomSize = (uint32_t)romData.size();
	_prgRom = new uint8_t[_prgRomSize];
	memcpy(_prgRom, romData.data(), romData.size());

	_cartRamSize = header.GetCartRamSize();
	_cartRam = new uint8_t[_cartRamSize];
	_hasBattery = header.HasBattery();

	shared_ptr<EmuSettings> settings = emu->GetSettings();
	GameboyConfig cfg = settings->GetGameboyConfig();
	GameboyModel model = cfg.Model;
	if(model == GameboyModel::Auto) {
		if((header.CgbFlag & 0x80) != 0) {
			model = GameboyModel::GameboyColor;
		} else {
			model = GameboyModel::SuperGameboy;
		}
	}

	if(!sgbEnabled && model == GameboyModel::SuperGameboy) {
		//SGB bios isn't available, use gameboy color mode instead
		model = GameboyModel::GameboyColor;
	}

	_model = model;

	bool cgbMode = _model == GameboyModel::GameboyColor;
	_workRamSize = cgbMode ? 0x8000 : 0x2000;
	_videoRamSize = cgbMode ? 0x4000 : 0x2000;

	_workRam = new uint8_t[_workRamSize];
	_videoRam = new uint8_t[_videoRamSize];
	_spriteRam = new uint8_t[Gameboy::SpriteRamSize];
	_highRam = new uint8_t[Gameboy::HighRamSize];

	_bootRomSize = 0;

	FirmwareType type = FirmwareType::Gameboy;
	if(_model == GameboyModel::SuperGameboy) {
		type = cfg.UseSgb2 ? FirmwareType::Sgb2GameboyCpu : FirmwareType::Sgb1GameboyCpu;
	} else if(_model == GameboyModel::GameboyColor) {
		type = FirmwareType::GameboyColor;
	}

	_bootRomSize = cgbMode ? 9 * 256 : 256;
	if(!FirmwareHelper::LoadGbBootRom(_emu, &_bootRom, type)) {
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

	settings->InitializeRam(_cartRam, _cartRamSize);
	settings->InitializeRam(_workRam, _workRamSize);
	settings->InitializeRam(_spriteRam, Gameboy::SpriteRamSize);
	settings->InitializeRam(_highRam, Gameboy::HighRamSize);
	settings->InitializeRam(_videoRam, _videoRamSize);

	LoadBattery();
}

Gameboy::Gameboy()
{
	_ppu.reset(new GbPpu());
	_apu.reset(new GbApu());
	_cpu.reset(new GbCpu());
	_memoryManager.reset(new GbMemoryManager());
	_timer.reset(new GbTimer());
	_dmaController.reset(new GbDmaController());
}

Gameboy::~Gameboy()
{
	SaveBattery();

	delete[] _cartRam;
	delete[] _prgRom;
	
	delete[] _spriteRam;
	delete[] _videoRam;

	delete[] _highRam;
	delete[] _workRam;

	delete[] _bootRom;
}

void Gameboy::PowerOn(SuperGameboy* superGameboy)
{
	_superGameboy = superGameboy;

	_timer->Init(_memoryManager.get(), _apu.get());
	_apu->Init(_emu, this);
	_cart->Init(this, _memoryManager.get());
	_memoryManager->Init(_emu, this, _cart.get(), _ppu.get(), _apu.get(), _timer.get(), _dmaController.get());
	_cpu->Init(_emu, this, _memoryManager.get());
	_ppu->Init(_emu, this, _memoryManager.get(), _dmaController.get(), _videoRam, _spriteRam);
	_dmaController->Init(_memoryManager.get(), _ppu.get(), _cpu.get());
}

void Gameboy::Exec()
{
	_cpu->Exec();
}

void Gameboy::Run(uint64_t runUntilClock)
{
	while(_memoryManager->GetCycleCount() < runUntilClock) {
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
	state.Dma = _dmaController->GetState();
	state.Timer = _timer->GetState();
	state.HasBattery = _hasBattery;
	return state;
}

uint32_t Gameboy::DebugGetMemorySize(SnesMemoryType type)
{
	switch(type) {
		case SnesMemoryType::GbPrgRom: return _prgRomSize;
		case SnesMemoryType::GbWorkRam: return _workRamSize;
		case SnesMemoryType::GbCartRam: return _cartRamSize;
		case SnesMemoryType::GbHighRam: return Gameboy::HighRamSize;
		case SnesMemoryType::GbBootRom: return _bootRomSize;
		case SnesMemoryType::GbVideoRam: return _videoRamSize;
		case SnesMemoryType::GbSpriteRam: return Gameboy::SpriteRamSize;
		default: return 0;
	}
}

uint8_t* Gameboy::DebugGetMemory(SnesMemoryType type)
{
	switch(type) {
		case SnesMemoryType::GbPrgRom: return _prgRom;
		case SnesMemoryType::GbWorkRam: return _workRam;
		case SnesMemoryType::GbCartRam: return _cartRam;
		case SnesMemoryType::GbHighRam: return _highRam;
		case SnesMemoryType::GbBootRom: return _bootRom;
		case SnesMemoryType::GbVideoRam: return _videoRam;
		case SnesMemoryType::GbSpriteRam: return _spriteRam;
		default: return nullptr;
	}
}

GbMemoryManager* Gameboy::GetMemoryManager()
{
	return _memoryManager.get();
}

GbPpu* Gameboy::GetPpu()
{
	return _ppu.get();
}

GbCpu* Gameboy::GetCpu()
{
	return _cpu.get();
}

void Gameboy::GetSoundSamples(int16_t* &samples, uint32_t& sampleCount)
{
	_apu->GetSoundSamples(samples, sampleCount);
}

AddressInfo Gameboy::GetAbsoluteAddress(uint16_t addr)
{
	AddressInfo addrInfo = { -1, SnesMemoryType::Register };

	if(addr >= 0xFF80 && addr <= 0xFFFE) {
		addrInfo.Address = addr & 0x7F;
		addrInfo.Type = SnesMemoryType::GbHighRam;
		return addrInfo;
	}

	uint8_t* ptr = _memoryManager->GetMappedBlock(addr);

	if(!ptr) {
		return addrInfo;
	}

	ptr += (addr & 0xFF);

	if(ptr >= _prgRom && ptr < _prgRom + _prgRomSize) {
		addrInfo.Address = (int32_t)(ptr - _prgRom);
		addrInfo.Type = SnesMemoryType::GbPrgRom;
	} else if(ptr >= _workRam && ptr < _workRam + _workRamSize) {
		addrInfo.Address = (int32_t)(ptr - _workRam);
		addrInfo.Type = SnesMemoryType::GbWorkRam;
	} else if(ptr >= _cartRam && ptr < _cartRam + _cartRamSize) {
		addrInfo.Address = (int32_t)(ptr - _cartRam);
		addrInfo.Type = SnesMemoryType::GbCartRam;
	} else if(ptr >= _bootRom && ptr < _bootRom + _bootRomSize) {
		addrInfo.Address = (int32_t)(ptr - _bootRom);
		addrInfo.Type = SnesMemoryType::GbBootRom;
	}
	return addrInfo;
}

int32_t Gameboy::GetRelativeAddress(AddressInfo& absAddress)
{
	if(absAddress.Type == SnesMemoryType::GbHighRam) {
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

GameboyHeader Gameboy::GetHeader()
{
	GameboyHeader header;
	memcpy(&header, _prgRom + Gameboy::HeaderOffset, sizeof(GameboyHeader));
	return header;
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
	return _memoryManager->GetCycleCount();
}

uint64_t Gameboy::GetApuCycleCount()
{
	return _memoryManager->GetApuCycleCount();
}

void Gameboy::Serialize(Serializer& s)
{
	s.Stream(_cpu.get());
	s.Stream(_ppu.get());
	s.Stream(_apu.get());
	s.Stream(_memoryManager.get());
	s.Stream(_cart.get());
	s.Stream(_timer.get());
	s.Stream(_dmaController.get());
	s.Stream(_hasBattery);

	s.StreamArray(_cartRam, _cartRamSize);
	s.StreamArray(_workRam, _workRamSize);
	s.StreamArray(_videoRam, _videoRamSize);
	s.StreamArray(_spriteRam, Gameboy::SpriteRamSize);
	s.StreamArray(_highRam, Gameboy::HighRamSize);
}
