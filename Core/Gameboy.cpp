#include "stdafx.h"
#include "Console.h"
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
#include "../Utilities/VirtualFile.h"
#include "../Utilities/Serializer.h"

Gameboy* Gameboy::Create(Console* console, VirtualFile &romFile)
{
	vector<uint8_t> romData;
	romFile.ReadFile(romData);

	GameboyHeader header;
	memcpy(&header, romData.data() + 0x134, sizeof(GameboyHeader));

	MessageManager::Log("-----------------------------");
	MessageManager::Log("File: " + romFile.GetFileName());
	MessageManager::Log("Game: " + header.GetCartName());
	MessageManager::Log("Cart Type: " + std::to_string(header.CartType));
	MessageManager::Log("File size: " + std::to_string(romData.size() / 1024) + " KB");
	if(header.GetCartRamSize() > 0) {
		string sizeString = header.GetCartRamSize() > 1024 ? std::to_string(header.GetCartRamSize() / 1024) + " KB" : std::to_string(header.GetCartRamSize()) + " bytes";
		MessageManager::Log("Cart RAM size: " + sizeString + (header.HasBattery() ? " (with battery)" : ""));
	}
	MessageManager::Log("-----------------------------");

	GbCart* cart = GbCartFactory::CreateCart(header.CartType);

	if(cart) {
		Gameboy* gb = new Gameboy();
		gb->_console = console;
		gb->_cart.reset(cart);
		gb->_prgRomSize = (uint32_t)romData.size();
		gb->_prgRom = new uint8_t[gb->_prgRomSize];
		memcpy(gb->_prgRom, romData.data(), romData.size());

		gb->_cartRamSize = header.GetCartRamSize();
		gb->_cartRam = new uint8_t[gb->_cartRamSize];
		gb->_hasBattery = header.HasBattery();

		shared_ptr<EmuSettings> settings = console->GetSettings();
		EmulationConfig cfg = settings->GetEmulationConfig();
		switch(cfg.GbModel) {
			default:
			case GameboyModel::Auto: gb->_cgbMode = (header.CgbFlag & 0x80) != 0; break;

			case GameboyModel::Gameboy: gb->_cgbMode = false; break;
			case GameboyModel::GameboyColor: gb->_cgbMode = true; break;
		}

		gb->_workRamSize = gb->_cgbMode ? 0x8000 : 0x2000;
		gb->_videoRamSize = gb->_cgbMode ? 0x4000 : 0x2000;

		gb->_workRam = new uint8_t[gb->_workRamSize];
		gb->_videoRam = new uint8_t[gb->_videoRamSize];
		gb->_spriteRam = new uint8_t[Gameboy::SpriteRamSize];
		gb->_highRam = new uint8_t[Gameboy::HighRamSize];
		
		gb->_useBootRom = cfg.GbUseBootRom;
		gb->_bootRomSize = 0;
		if(gb->_useBootRom) {
			gb->_useBootRom = FirmwareHelper::LoadGbBootRom(console, &gb->_bootRom, gb->_cgbMode ? FirmwareType::GameboyColor : FirmwareType::Gameboy);
			if(gb->_useBootRom) {
				gb->_bootRomSize = gb->_cgbMode ? 9 * 256 : 256;
			}
		}

		return gb;
	}

	return nullptr;
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

void Gameboy::PowerOn()
{
	shared_ptr<EmuSettings> settings = _console->GetSettings();
	settings->InitializeRam(_cartRam, _cartRamSize);
	settings->InitializeRam(_workRam, _workRamSize);
	settings->InitializeRam(_spriteRam, Gameboy::SpriteRamSize);
	settings->InitializeRam(_highRam, Gameboy::HighRamSize);

	//VRAM is filled with 0s by the boot rom
	memset(_videoRam, 0, _videoRamSize);
	memset(_spriteRam, 0, Gameboy::SpriteRamSize);

	LoadBattery();

	_ppu.reset(new GbPpu());
	_apu.reset(new GbApu(_console, this));
	_memoryManager.reset(new GbMemoryManager());
	_timer.reset(new GbTimer(_memoryManager.get(), _apu.get()));
	_dmaController.reset(new GbDmaController(_memoryManager.get()));
	_cart->Init(this, _memoryManager.get());
	_memoryManager->Init(_console, this, _cart.get(), _ppu.get(), _apu.get(), _timer.get(), _dmaController.get());

	_cpu.reset(new GbCpu(_console, this, _memoryManager.get()));
	_ppu->Init(_console, this, _memoryManager.get(), _videoRam, _spriteRam);
}

void Gameboy::Exec()
{
	_cpu->Exec();
}

void Gameboy::Run(uint64_t masterClock)
{
	while(_memoryManager->GetCycleCount() < masterClock) {
		_cpu->Exec();
	}
}

void Gameboy::LoadBattery()
{
	if(_hasBattery) {
		_console->GetBatteryManager()->LoadBattery(".srm", _cartRam, _cartRamSize);
	}
}

void Gameboy::SaveBattery()
{
	if(_hasBattery) {
		_console->GetBatteryManager()->SaveBattery(".srm", _cartRam, _cartRamSize);
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

bool Gameboy::IsCgb()
{
	return _cgbMode;
}

bool Gameboy::UseBootRom()
{
	return _useBootRom;
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
