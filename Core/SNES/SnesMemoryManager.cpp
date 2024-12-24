#include "pch.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesDmaController.h"
#include "SNES/RegisterHandlerA.h"
#include "SNES/RegisterHandlerB.h"
#include "SNES/RamHandler.h"
#include "Debugger/DebugTypes.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/CheatManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/HexUtilities.h"
#include "Shared/MemoryOperationType.h"

void SnesMemoryManager::Initialize(SnesConsole *console)
{
	_masterClock = 0;
	_openBus = 0;
	
	_cpuSpeed = 8;
	UpdateExecCallbacks();

	_console = console;
	_emu = console->GetEmulator();
	_regs = console->GetInternalRegisters();
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_cart = console->GetCartridge();
	_cheatManager = _emu->GetCheatManager();

	_workRam = new uint8_t[SnesMemoryManager::WorkRamSize];
	_emu->RegisterMemory(MemoryType::SnesWorkRam, _workRam, SnesMemoryManager::WorkRamSize);
	_console->InitializeRam(_workRam, SnesMemoryManager::WorkRamSize);

	_registerHandlerA.reset(new RegisterHandlerA(
		console->GetDmaController(),
		console->GetInternalRegisters(),
		(SnesControlManager*)console->GetControlManager()
	));

	_registerHandlerB.reset(new RegisterHandlerB(
		_console,
		_ppu,
		console->GetSpc(),
		_workRam
	));

	for(uint32_t i = 0; i < 128 * 1024; i += 0x1000) {
		_workRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(_workRam, i, SnesMemoryManager::WorkRamSize, MemoryType::SnesWorkRam)));
	}

	_mappings.RegisterHandler(0x7E, 0x7F, 0x0000, 0xFFFF, _workRamHandlers);

	_mappings.RegisterHandler(0x00, 0x3F, 0x2000, 0x2FFF, _registerHandlerB.get());
	_mappings.RegisterHandler(0x80, 0xBF, 0x2000, 0x2FFF, _registerHandlerB.get());

	_mappings.RegisterHandler(0x00, 0x3F, 0x4000, 0x4FFF, _registerHandlerA.get());
	_mappings.RegisterHandler(0x80, 0xBF, 0x4000, 0x4FFF, _registerHandlerA.get());

	_mappings.RegisterHandler(0x00, 0x3F, 0x0000, 0x0FFF, _workRamHandlers[0].get());
	_mappings.RegisterHandler(0x80, 0xBF, 0x0000, 0x0FFF, _workRamHandlers[0].get());
	_mappings.RegisterHandler(0x00, 0x3F, 0x1000, 0x1FFF, _workRamHandlers[1].get());
	_mappings.RegisterHandler(0x80, 0xBF, 0x1000, 0x1FFF, _workRamHandlers[1].get());

	_cart->Init(_mappings);

	GenerateMasterClockTable();
	Reset();
}

SnesMemoryManager::~SnesMemoryManager()
{
	delete[] _workRam;
}

void SnesMemoryManager::Reset()
{
	_masterClock = 0;
	_hClock = 0;
	_dramRefreshPosition = 538 - (_masterClock & 0x07);
	_nextEventClock = _dramRefreshPosition;
	_nextEvent = SnesEventType::DramRefresh;
}

void SnesMemoryManager::GenerateMasterClockTable()
{
	for(int i = 0; i < 0x800; i++) {
		uint8_t bank = (i & 0x300) >> 8;
		if(bank == 1) {
			//Banks $40-$7F - Slow
			_masterClockTable[i] = 8;
		} else if(bank == 3) {
			//Banks $C0-$FF
			//Slow or fast (depending on register)
			_masterClockTable[i] = (i >= 0x400) ? 6 : 8;
		} else {
			//Banks $00-$3F and $80-$BF
			uint8_t page = (i & 0xFF);
			if(page <= 0x1F) {
				//Slow
				_masterClockTable[i] = 8;
			} else if(page >= 0x20 && page <= 0x3F) {
				//Fast
				_masterClockTable[i] = 6;
			} else if(page == 0x40 || page == 0x41) {
				//Extra slow
				_masterClockTable[i] = 12;
			} else if(page >= 0x42 && page <= 0x5F) {
				//Fast
				_masterClockTable[i] = 6;
			} else if(page >= 0x60 && page <= 0x7F) {
				//Slow
				_masterClockTable[i] = 8;
			} else if(bank == 0) {
				//Slow
				_masterClockTable[i] = 8;
			} else {
				//page >= $80
				//Slow or fast (depending on register)
				_masterClockTable[i] = (i >= 0x400) ? 6 : 8;
			}
		}
	}
}

template<uint8_t clocks>
void SnesMemoryManager::IncMasterClock()
{
	if constexpr(clocks == 2) {
		Exec();
	} else if constexpr(clocks == 4) {
		Exec();
		Exec();
	} else if constexpr(clocks == 6) {
		Exec();
		Exec();
		Exec();
	} else if constexpr(clocks == 8) {
		Exec();
		Exec();
		Exec();
		Exec();
	} else if constexpr(clocks == 12) {
		Exec();
		Exec();
		Exec();
		Exec();
		Exec();
		Exec();
	}
}

void SnesMemoryManager::IncMasterClock4()
{
	Exec();
	Exec();
}

void SnesMemoryManager::IncMasterClock6()
{
	Exec();
	Exec();
	Exec();
}

void SnesMemoryManager::IncMasterClock8()
{
	Exec();
	Exec();
	Exec();
	Exec();
}

void SnesMemoryManager::IncMasterClock40()
{
	Exec(); Exec(); Exec(); Exec(); Exec();
	Exec(); Exec(); Exec(); Exec(); Exec();
	Exec(); Exec(); Exec(); Exec(); Exec();
	Exec(); Exec(); Exec(); Exec(); Exec();
}

void SnesMemoryManager::IncMasterClockStartup()
{
	for(int i = 0; i < 186 / 2; i++) {
		Exec();
	}
}

void SnesMemoryManager::IncrementMasterClockValue(uint16_t cyclesToRun)
{
	switch(cyclesToRun) {
		case 12: Exec(); [[fallthrough]];
		case 10: Exec(); [[fallthrough]];
		case 8: Exec(); [[fallthrough]];
		case 6: Exec(); [[fallthrough]];
		case 4: Exec(); [[fallthrough]];
		case 2: Exec(); break;
	}
}

void SnesMemoryManager::Exec()
{
	_masterClock += 2;
	_hClock += 2;

	if(_hClock == _nextEventClock) {
		ProcessEvent();
	} 
	
	if((_hClock & 0x03) == 0) {
		_emu->ProcessPpuCycle<CpuType::Snes>();
	} else if(_hClock & 0x02) {
		_regs->ProcessIrqCounters();
	}

	_cart->SyncCoprocessors();
}

void SnesMemoryManager::ProcessEvent()
{
	switch(_nextEvent) {
		case SnesEventType::HdmaInit:
			_console->GetDmaController()->BeginHdmaInit();
			_nextEvent = SnesEventType::DramRefresh;
			_nextEventClock = _dramRefreshPosition;
			break;

		case SnesEventType::DramRefresh:
			IncMasterClock40();
			if(_ppu->GetScanline() < _ppu->GetVblankStart()) {
				_nextEvent = SnesEventType::HdmaStart;
				_nextEventClock = 276 * 4;
			} else {
				_nextEvent = SnesEventType::EndOfScanline;
				_nextEventClock = 1360;
			}
			break;

		case SnesEventType::HdmaStart:
			_console->GetDmaController()->BeginHdmaTransfer();
			_nextEvent = SnesEventType::EndOfScanline;
			_nextEventClock = 1360;
			break;

		case SnesEventType::EndOfScanline:
			if(_ppu->ProcessEndOfScanline(_hClock)) {
				_dramRefreshPosition = 538 - (_masterClock & 0x07);
				if(_ppu->GetScanline() == 0) {
					_nextEvent = SnesEventType::HdmaInit;
					_nextEventClock = 12 + (_masterClock & 0x07);
				} else {
					_nextEvent = SnesEventType::DramRefresh;
					_nextEventClock = _dramRefreshPosition;
				}
			} else {
				_nextEventClock += 2;
			}
			break;
	}
}

uint8_t SnesMemoryManager::Read(uint32_t addr, MemoryOperationType type)
{
	(this->*_execRead)();

	uint8_t value;
	IMemoryHandler *handler = _mappings.GetHandler(addr);
	if(handler) {
		value = handler->Read(addr);
		_memTypeBusA = handler->GetMemoryType();
		_openBus = value;
	} else {
		//open bus
		value = _openBus;
		LogDebug("[Debug] Read - missing handler: $" + HexUtilities::ToHex(addr));
	}
	if(_cheatManager->HasCheats<CpuType::Snes>()) {
		_cheatManager->ApplyCheat<CpuType::Snes>(addr, value);
	}
	IncMasterClock4();
	_emu->ProcessMemoryRead<CpuType::Snes>(addr, value, type);
	return value;
}

uint8_t SnesMemoryManager::ReadDma(uint32_t addr, bool forBusA)
{
	IncMasterClock4();

	uint8_t value;
	IMemoryHandler* handler = _mappings.GetHandler(addr);
	if(handler) {
		if(forBusA && handler == _registerHandlerB.get() && (addr & 0xFF00) == 0x2100) {
			//Trying to read from bus B using bus A returns open bus
			value = _openBus;
		} else if(handler == _registerHandlerA.get()) {
			uint16_t regAddr = addr & 0xFFFF;
			if(regAddr == 0x420B || regAddr == 0x420C || (regAddr >= 0x4300 && regAddr <= 0x437F)) {
				//Trying to read the DMA controller with DMA returns open bus
				value = _openBus;
			} else {
				value = handler->Read(addr);
			}
		} else {
			value = handler->Read(addr);
			if(handler != _registerHandlerB.get()) {
				_memTypeBusA = handler->GetMemoryType();
			}
		}
		_openBus = value;
	} else {
		//open bus
		value = _openBus;
		LogDebug("[Debug] Read - missing handler: $" + HexUtilities::ToHex(addr));
	}
	if(_cheatManager->HasCheats<CpuType::Snes>()) {
		_cheatManager->ApplyCheat<CpuType::Snes>(addr, value);
	}
	_emu->ProcessMemoryRead<CpuType::Snes>(addr, value, MemoryOperationType::DmaRead);
	return value;
}

uint8_t SnesMemoryManager::Peek(uint32_t addr)
{
	return _mappings.Peek(addr);
}

uint16_t SnesMemoryManager::PeekWord(uint32_t addr)
{
	return _mappings.PeekWord(addr);
}

void SnesMemoryManager::PeekBlock(uint32_t addr, uint8_t *dest)
{
	_mappings.PeekBlock(addr, dest);
}

void SnesMemoryManager::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	(this->*_execWrite)();

	if(_emu->ProcessMemoryWrite<CpuType::Snes>(addr, value, type)) {
		IMemoryHandler* handler = _mappings.GetHandler(addr);
		if(handler) {
			handler->Write(addr, value);
			_memTypeBusA = handler->GetMemoryType();
		} else {
			LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
		}
		_openBus = value;
	}
}

void SnesMemoryManager::WriteDma(uint32_t addr, uint8_t value, bool forBusA)
{
	IncMasterClock4();
	if(_emu->ProcessMemoryWrite<CpuType::Snes>(addr, value, MemoryOperationType::DmaWrite)) {
		IMemoryHandler* handler = _mappings.GetHandler(addr);
		if(handler) {
			if(forBusA && handler == _registerHandlerB.get() && (addr & 0xFF00) == 0x2100) {
				//Trying to write to bus B using bus A does nothing
			} else if(handler == _registerHandlerA.get()) {
				uint16_t regAddr = addr & 0xFFFF;
				if(regAddr == 0x420B || regAddr == 0x420C || (regAddr >= 0x4300 && regAddr <= 0x437F)) {
					//Trying to write to the DMA controller with DMA does nothing
				} else {
					handler->Write(addr, value);
				}
			} else {
				handler->Write(addr, value);
				if(handler != _registerHandlerB.get()) {
					_memTypeBusA = handler->GetMemoryType();
				}
			}
		} else {
			LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
		}
		_openBus = value;
	}
}

uint8_t SnesMemoryManager::GetOpenBus()
{
	return _openBus;
}

uint64_t SnesMemoryManager::GetMasterClock()
{
	return _masterClock;
}

uint16_t SnesMemoryManager::GetHClock()
{
	return _hClock;
}

uint8_t * SnesMemoryManager::DebugGetWorkRam()
{
	return _workRam;
}

MemoryMappings* SnesMemoryManager::GetMemoryMappings()
{
	return &_mappings;
}

uint8_t SnesMemoryManager::GetCpuSpeed(uint32_t addr)
{
	return _masterClockTable[(_regs->IsFastRomEnabled() << 10) | ((addr & 0xC00000) >> 14) | ((addr & 0xFF00) >> 8)];
}

uint8_t SnesMemoryManager::GetCpuSpeed()
{
	return _cpuSpeed;
}

void SnesMemoryManager::SetCpuSpeed(uint8_t speed)
{
	if(_cpuSpeed != speed) {
		_cpuSpeed = speed;
		UpdateExecCallbacks();
	}
}

void SnesMemoryManager::UpdateExecCallbacks()
{
	switch(_cpuSpeed) {
		case 6:
			_execRead = &SnesMemoryManager::IncMasterClock<2>;
			_execWrite = &SnesMemoryManager::IncMasterClock<6>;
			break;

		case 8:
			_execRead = &SnesMemoryManager::IncMasterClock<4>;
			_execWrite = &SnesMemoryManager::IncMasterClock<8>;
			break;

		case 12:
			_execRead = &SnesMemoryManager::IncMasterClock<8>;
			_execWrite = &SnesMemoryManager::IncMasterClock<12>;
			break;
	}
}

MemoryType SnesMemoryManager::GetMemoryTypeBusA()
{
	return _memTypeBusA;
}

bool SnesMemoryManager::IsRegister(uint32_t cpuAddress)
{
	IMemoryHandler* handler = _mappings.GetHandler(cpuAddress);
	return handler == _registerHandlerA.get() || handler == _registerHandlerB.get();
}

bool SnesMemoryManager::IsWorkRam(uint32_t cpuAddress)
{
	IMemoryHandler* handler = _mappings.GetHandler(cpuAddress);
	return handler && handler->GetMemoryType() == MemoryType::SnesWorkRam;
}

uint32_t SnesMemoryManager::GetWramPosition()
{
	return _registerHandlerB->GetWramPosition();
}

void SnesMemoryManager::Serialize(Serializer &s)
{
	SV(_masterClock); SV(_openBus); SV(_cpuSpeed); SV(_hClock); SV(_dramRefreshPosition);
	SV(_memTypeBusA); SV(_nextEvent); SV(_nextEventClock);
	SVArray(_workRam, SnesMemoryManager::WorkRamSize);
	SV(_registerHandlerB);

	if(!s.IsSaving()) {
		UpdateExecCallbacks();
	}
}
