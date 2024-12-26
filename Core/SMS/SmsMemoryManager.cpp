#include "pch.h"
#include "SMS/SmsMemoryManager.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsPsg.h"
#include "SMS/SmsFmAudio.h"
#include "SMS/SmsBiosMapper.h"
#include "SMS/Carts/SmsCart.h"
#include "SMS/SmsControlManager.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/CheatManager.h"
#include "Shared/KeyManager.h"
#include "Utilities/CRC32.h"

SmsMemoryManager::SmsMemoryManager()
{
}

SmsMemoryManager::~SmsMemoryManager()
{
	delete[] _workRam;
	delete[] _cartRam;
	delete[] _prgRom;
	delete[] _originalCartRam;
}

void SmsMemoryManager::Init(Emulator* emu, SmsConsole* console, vector<uint8_t>& romData, vector<uint8_t>& biosRom, SmsVdp* vdp, SmsControlManager* controlManager, SmsCart* cart, SmsPsg* psg, SmsFmAudio* fmAudio)
{
	_emu = emu;
	_console = console;
	_vdp = vdp;
	_psg = psg;
	_controlManager = controlManager;
	_cart = cart;
	_fmAudio = fmAudio;

	_prgRom = new uint8_t[romData.size()];
	_prgRomSize = (uint32_t)romData.size();
	memcpy(_prgRom, romData.data(), _prgRomSize);
	_emu->RegisterMemory(MemoryType::SmsPrgRom, _prgRom, _prgRomSize);

	_model = _console->GetModel();
	bool isSg1000 = _model == SmsModel::Sg;
	bool isCv = _model == SmsModel::ColecoVision;

	_workRamSize = isCv ? SmsMemoryManager::CvWorkRamSize : SmsMemoryManager::SmsWorkRamSize;

	_workRam = new uint8_t[_workRamSize];
	console->InitializeRam(_workRam, _workRamSize);
	_emu->RegisterMemory(MemoryType::SmsWorkRam, _workRam, _workRamSize);

	if(biosRom.size() >= 0x400) {
		_biosRom = new uint8_t[biosRom.size()];
		_biosRomSize = (uint32_t)biosRom.size();
		memcpy(_biosRom, biosRom.data(), _biosRomSize);
		_state.BiosEnabled = true;
		_emu->RegisterMemory(MemoryType::SmsBootRom, _biosRom, _biosRomSize);
		if(!isCv) {
			_biosMapper.reset(new SmsBiosMapper(this));
		}
	} else {
		if(isSg1000) {
			_state.CardEnabled = true;
		} else {
			_state.CartridgeEnabled = true;
		}
		memset(_workRam, 0, _workRamSize);

		//default value for $3E that some games expect after bios runs
		_workRam[0] = _model == SmsModel::GameGear ? 0xA8 : 0xAB;
	}

	if(_model == SmsModel::GameGear) {
		_state.CartridgeEnabled = true;
	}

	if(isSg1000) {
		_cartRamSize = DetectSgCartRam(romData);
	} else if(isCv) {
		_cartRamSize = 0;
	} else {
		_cartRamSize = SmsMemoryManager::CartRamMaxSize;
	}

	_cartRam = new uint8_t[_cartRamSize];
	console->InitializeRam(_cartRam, _cartRamSize);
	_emu->RegisterMemory(MemoryType::SmsCartRam, _cartRam, _cartRamSize);

	if(!isSg1000 && !isCv) {
		LoadBattery();
		_originalCartRam = new uint8_t[_cartRamSize];
		memcpy(_originalCartRam, _cartRam, _cartRamSize);
	}

	_state.WorkRamEnabled = true;
	_state.IoEnabled = true;

	//Power on values unverified, probably incorrect?
	_state.GgExtConfig = 0x7F;
	_state.GgExtData = 0xFF;

	RefreshMappings();
}

void SmsMemoryManager::RefreshMappings()
{
	Unmap(0, 0xFFFF);
	MapRegisters(0, 0xFFFF, SmsRegisterAccess::None);
	if(_model == SmsModel::ColecoVision) {
		Map(0x0000, 0x1FFF, MemoryType::SmsBootRom, 0, true);
		for(int i = 0; i < 8; i++) {
			Map(0x6000+i*0x400, 0x63FF+i*0x400, MemoryType::SmsWorkRam, 0, false);
		}
		
		//Don't mirror rom (Sammy Lightfoot breaks if mirrored)
		Map(0x8000, std::min<int>(0x8000 + _prgRomSize - 1, 0xFFFF), MemoryType::SmsPrgRom, 0, true);
	} else {
		if(_state.CartridgeEnabled && _model != SmsModel::Sg) {
			_cart->RefreshMappings();
		}

		if(_state.CardEnabled && _model == SmsModel::Sg) {
			Map(0x0000, 0xBFFF, MemoryType::SmsPrgRom, 0, true);
			if(_sgRamMapAddress >= 0 && _cartRamSize > 0) {
				Map(_sgRamMapAddress, _sgRamMapAddress + _cartRamSize - 1, MemoryType::SmsCartRam, 0, false);
			}
		}

		if(_state.WorkRamEnabled) {
			Map(0xC000, 0xFFFF, MemoryType::SmsWorkRam, 0, false);
		}

		if(_biosMapper && _state.BiosEnabled && (_model == SmsModel::GameGear || !_state.CartridgeEnabled)) {
			bool enableCart = _model == SmsModel::GameGear;
			_biosMapper->RefreshMappings(enableCart);
		}
	}
}

bool SmsMemoryManager::HasBios()
{
	return _biosMapper != nullptr;
}

void SmsMemoryManager::LoadBattery()
{
	_emu->GetBatteryManager()->LoadBattery(".sav", _cartRam, SmsMemoryManager::CartRamMaxSize);
}

void SmsMemoryManager::SaveBattery()
{
	if(_originalCartRam == nullptr) {
		//SG-1000 with ext ram adapter (Taiwanese games)
		return;
	}

	if(memcmp(_cartRam, _originalCartRam, SmsMemoryManager::CartRamMaxSize) != 0) {
		uint32_t batterySize = 0;
		//Select size of the .sav based on which portions of the 32kb buffer were changed
		if(memcmp(_cartRam + 0x4000, _originalCartRam + 0x4000, 0x4000) != 0) {
			batterySize = 0x8000;
		} else if(memcmp(_cartRam + 0x2000, _originalCartRam + 0x2000, 0x2000) != 0) {
			batterySize = 0x4000;
		} else if(memcmp(_cartRam, _originalCartRam, 0x2000) != 0) {
			batterySize = 0x2000;
		}
		batterySize = std::max(batterySize, _emu->GetBatteryManager()->GetBatteryFileSize(".sav"));
		if(batterySize > 0) {
			_emu->GetBatteryManager()->SaveBattery(".sav", _cartRam, batterySize);
		}
	}
}

AddressInfo SmsMemoryManager::GetAbsoluteAddress(uint16_t addr)
{
	AddressInfo addrInfo = { -1, MemoryType::None };

	uint8_t* ptr = _reads[addr >> 8];
	if(!ptr) {
		return addrInfo;
	}

	ptr += (addr & 0xFF);

	if(ptr >= _prgRom && ptr < _prgRom + _prgRomSize) {
		addrInfo.Address = (int32_t)(ptr - _prgRom);
		addrInfo.Type = MemoryType::SmsPrgRom;
	} else if(ptr >= _workRam && ptr < _workRam + _workRamSize) {
		addrInfo.Address = (int32_t)(ptr - _workRam);
		addrInfo.Type = MemoryType::SmsWorkRam;
	} else if(ptr >= _cartRam && ptr < _cartRam + SmsMemoryManager::CartRamMaxSize) {
		addrInfo.Address = (int32_t)(ptr - _cartRam);
		addrInfo.Type = MemoryType::SmsCartRam;
	} else if(_biosRom && ptr >= _biosRom && ptr < _biosRom + _biosRomSize) {
		addrInfo.Address = (int32_t)(ptr - _biosRom);
		addrInfo.Type = MemoryType::SmsBootRom;
	}
	return addrInfo;
}

int32_t SmsMemoryManager::GetRelativeAddress(AddressInfo& absAddress)
{
	for(int32_t i = 0; i < 0x10000; i += 0x100) {
		AddressInfo blockAddr = GetAbsoluteAddress(i);
		if(blockAddr.Type == absAddress.Type && (blockAddr.Address & ~0xFF) == (absAddress.Address & ~0xFF)) {
			return i | (absAddress.Address & 0xFF);
		}
	}
	return -1;
}

void SmsMemoryManager::Map(uint16_t start, uint16_t end, MemoryType type, uint32_t offset, bool readonly)
{
	uint8_t* src = (uint8_t*)_emu->GetMemory(type).Memory;
	uint32_t size = _emu->GetMemory(type).Size;

	if(size > 0) {
		while(offset >= size) {
			offset -= size;
		}

		src += offset;
		for(int i = start; i < end; i += 0x100) {
			_reads[i >> 8] = src;
			_writes[i >> 8] = readonly ? nullptr : src;

			if(src) {
				src += 0x100;
				offset = (offset + 0x100);
				if(offset >= size) {
					offset = 0;
					src = (uint8_t*)_emu->GetMemory(type).Memory;
				}
			}
		}
	} else {
		Unmap(start, end);
	}
}

void SmsMemoryManager::Unmap(uint16_t start, uint16_t end)
{
	for(int i = start; i < end; i += 0x100) {
		_reads[i >> 8] = nullptr;
		_writes[i >> 8] = nullptr;
	}
}

void SmsMemoryManager::MapRegisters(uint16_t start, uint16_t end, SmsRegisterAccess access)
{
	for(int i = start; i <= end; i += 0x100) {
		_state.IsReadRegister[i >> 8] = ((int)access & (int)SmsRegisterAccess::Read) != 0;
		_state.IsWriteRegister[i >> 8] = ((int)access & (int)SmsRegisterAccess::Write) != 0;
	}
}

uint8_t SmsMemoryManager::GetOpenBus()
{
	return _console->GetRevision() == SmsRevision::Sms1 ? _state.OpenBus : 0xFF;
}

uint8_t SmsMemoryManager::DebugRead(uint16_t addr)
{
	if(_state.IsReadRegister[addr >> 8]) {
		return _cart->PeekRegister(addr);
	}

	uint8_t* data = _reads[addr >> 8];
	return data ? data[(uint8_t)addr] : GetOpenBus();
}

void SmsMemoryManager::Write(uint16_t addr, uint8_t value)
{
	if(_emu->ProcessMemoryWrite<CpuType::Sms>(addr, value, MemoryOperationType::Write)) {
		if(_state.IsWriteRegister[addr >> 8]) {
			if(_state.CartridgeEnabled) {
				_cart->WriteRegister(addr, value);
			}
			if(_biosMapper && _state.BiosEnabled) {
				_biosMapper->WriteRegister(addr, value);
			}
		}
		if(_writes[addr >> 8]) {
			_writes[addr >> 8][(uint8_t)addr] = value;
		}
		_state.OpenBus = value;
	}
}

void SmsMemoryManager::DebugWrite(uint16_t addr, uint8_t value)
{
	//TODOSMS - allow side-effects for debugger
	uint8_t* data = _writes[addr >> 8];
	if(data) {
		data[(uint8_t)addr] = value;
	}
}

uint8_t SmsMemoryManager::DebugReadPort(uint8_t port)
{
	return InternalReadPort<true>(port);
}

uint8_t SmsMemoryManager::ReadPort(uint8_t port)
{
	return InternalReadPort<false>(port);
}

template<bool isPeek>
uint8_t SmsMemoryManager::InternalReadPort(uint8_t port)
{
	uint8_t value;
	switch(_model) {
		case SmsModel::ColecoVision: value = ReadColecoVisionPort<isPeek>(port); break;
		case SmsModel::GameGear: value = ReadGameGearPort<isPeek>(port); break;
		default: value = ReadSmsPort<isPeek>(port);
	}

	if constexpr(!isPeek) {
		_emu->ProcessMemoryAccess<CpuType::Sms, MemoryType::SmsPort, MemoryOperationType::Read>(port, value);
	}
	return value;
}

void SmsMemoryManager::WritePort(uint8_t port, uint8_t value)
{
	_emu->ProcessMemoryAccess<CpuType::Sms, MemoryType::SmsPort, MemoryOperationType::Write>(port, value);
	switch(_model) {
		case SmsModel::ColecoVision: WriteColecoVisionPort(port, value); break;
		case SmsModel::GameGear: WriteGameGearPort(port, value); break;
		default: WriteSmsPort(port, value); break;
	}
}

void SmsMemoryManager::WriteSmsPort(uint8_t port, uint8_t value)
{
	bool isGameGear = _model == SmsModel::GameGear;

	switch(port & 0xC1) {
		case 0x00: {
			//Port 3E (Memory Control)
			bool isSms1 = _console->GetRevision() == SmsRevision::Sms1;
			_state.ExpEnabled = (value & 0x80) == 0 && !isSms1; //TODOSMS not implemented
			_state.CartridgeEnabled = (value & 0x40) == 0 || isGameGear;
			_state.CardEnabled = (value & 0x20) == 0 && !isSms1;
			_state.WorkRamEnabled = (value & 0x10) == 0;
			_state.BiosEnabled = (value & 0x08) == 0;
			_state.IoEnabled = (value & 0x04) == 0 || isGameGear;
			RefreshMappings();
			break;
		}

		case 0x01:
			//Port 3F (IO Port Control)
			_controlManager->WriteControlPort(value);
			break;

		case 0x40: case 0x41:
			_psg->Write(value);
			break;

		case 0x80: case 0x81:
			//Ports: BE (VDP data), BF (VDP control)
			_vdp->WritePort(port, value);
			break;

		case 0xC0:
		case 0xC1:
			if(_emu->GetSettings()->GetSmsConfig().EnableFmAudio) {
				_fmAudio->Write(port, value);
			}
			break;
	}
}

void SmsMemoryManager::WriteColecoVisionPort(uint8_t port, uint8_t value)
{
	switch(port & 0xE0) {
		case 0x80: _controlManager->WriteControlPort(0); break;
		case 0xC0: _controlManager->WriteControlPort(1); break;
		case 0xE0: _psg->Write(value); break;
		case 0xA0: _vdp->WritePort(port, value); break;
	}
}

void SmsMemoryManager::WriteGameGearPort(uint8_t port, uint8_t value)
{
	switch(port) {
		case 0: break; //read-only

		//TODOSMS GG - input/output ext port
		case 1: _state.GgExtData = value & 0x7F; break;
		case 2: _state.GgExtConfig = value; break;
		case 3: _state.GgSendData = value; break;
		case 4: break; //read-only
		case 5: _state.GgSerialConfig = value; break;

		//Sound panning - write-only
		case 6:
			_psg->WritePanningReg(value);
			break;

		default:
			WriteSmsPort(port, value);
			break;
	}
}

template<bool isPeek>
uint8_t SmsMemoryManager::ReadSmsPort(uint8_t port)
{
	switch(port & 0xC1) {
		case 0x00: return GetOpenBus(); //Port 3E (Memory Control)

		case 0x40: case 0x41: case 0x80: case 0x81:
			//Ports: 7E (V counter), 7F (H counter), BE (VDP data), BF (VDP control)
			return isPeek ? _vdp->PeekPort(port) : _vdp->ReadPort(port);

		case 0xC0:
			//Port DC (IO Port A/B)
			if(_state.IoEnabled) {
				return _controlManager->ReadPort(0);
			} else if(port == 0xF2) {
				if(_emu->GetSettings()->GetSmsConfig().EnableFmAudio) {
					return _fmAudio->Read();
				} else {
					return GetOpenBus();
				}
			}
			return GetOpenBus();

		case 0xC1: return _state.IoEnabled ? _controlManager->ReadPort(1) : GetOpenBus(); //Port DD (IO Port B/Misc)

		default: return GetOpenBus();
	}
}

template<bool isPeek>
uint8_t SmsMemoryManager::ReadColecoVisionPort(uint8_t port)
{
	switch(port & 0xE0) {
		case 0xA0: return isPeek ? _vdp->PeekPort(port) : _vdp->ReadPort(port);
		case 0xE0: return _controlManager->ReadPort((port >> 1) & 0x01);
		default: return 0xFF;
	}
}

template<bool isPeek>
uint8_t SmsMemoryManager::ReadGameGearPort(uint8_t port)
{
	switch(port) {
		case 0: {
			//start button, region, pal/ntsc
			ConsoleRegion region = _console->GetRegion();
			return (
				(_controlManager->IsPausePressed() ? 0x00 : 0x80) |
				(region == ConsoleRegion::NtscJapan ? 0x00 : 0x40) |
				(region == ConsoleRegion::Pal ? 0x20 : 0x00)
			);
		}

		//TODOSMS GG - input/output ext port
		case 1: return _state.GgExtData;
		case 2: return _state.GgExtConfig;
		case 3: return _state.GgSendData;
		case 4: return 0xFF;
		case 5: return _state.GgSerialConfig;

		//Sound panning - write-only
		case 6: return 0xFF;

		default:
			if(port < 0x40) {
				return 0xFF;
			} else if(port < 0xC0) {
				return isPeek ? _vdp->PeekPort(port) : _vdp->ReadPort(port);
			} else {
				switch(port) {
					case 0xC0: case 0xDC:
						return _controlManager->ReadPort(0);

					case 0xC1: case 0xDD:
						return _controlManager->ReadPort(1);
				}
			}
	}

	return 0xFF;
}

uint32_t SmsMemoryManager::DetectSgCartRam(vector<uint8_t>& romData)
{
	//Used by Taiwanese games that require 8kb of ram at 2000-3FFF
	//These roms have 2000-3FFF filled with FF, this is used to detect them
	bool isUnlicensedRom = true;
	if(romData.size() >= 0x4000) {
		for(int i = 0; i < 0x2000; i++) {
			if(romData[0x2000 + i] != 0xFF) {
				isUnlicensedRom = false;
			}
		}
	}
	
	if(isUnlicensedRom) {
		_sgRamMapAddress = 0x2000;
		return 0x2000;
	} else {
		//SG-1000 games that had extra ram in cart
		uint32_t crc = CRC32::GetCRC(romData);
		if(crc == 0x092F29D6) {
			//The Castle, 8kb at $8000
			_sgRamMapAddress = 0x8000;
			return 0x2000;
		} else if(crc == 0xAF4F14BC) {
			//Othello, 2kb at $8000
			_sgRamMapAddress = 0x8000;
			return 0x800;
		}
	}

	return 0;
}

void SmsMemoryManager::Serialize(Serializer& s)
{
	SV(_masterClock);
	if(_biosMapper) {
		SV(_biosMapper);
	}
	
	SV(_state.ExpEnabled);
	SV(_state.CartridgeEnabled);
	SV(_state.CardEnabled);
	SV(_state.WorkRamEnabled);
	SV(_state.BiosEnabled);
	SV(_state.IoEnabled);

	SV(_state.GgExtData);
	SV(_state.GgExtConfig);
	SV(_state.GgSendData);
	SV(_state.GgSerialConfig);

	if(_cartRamSize > 0) {
		SVArray(_cartRam, _cartRamSize);
	}
	SVArray(_workRam, _workRamSize);

	if(!s.IsSaving()) {
		RefreshMappings();
	}
}
