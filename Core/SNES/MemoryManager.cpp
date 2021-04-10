#include "stdafx.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Cpu.h"
#include "SNES/Ppu.h"
#include "SNES/DmaController.h"
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
#include "MemoryOperationType.h"

void MemoryManager::Initialize(Console *console)
{
	_masterClock = 0;
	_openBus = 0;
	_cpuSpeed = 8;
	_console = console;
	_emu = console->GetEmulator();
	_regs = console->GetInternalRegisters().get();
	_cpu = console->GetCpu().get();
	_ppu = console->GetPpu().get();
	_cart = console->GetCartridge().get();
	_cheatManager = _emu->GetCheatManager().get();

	_workRam = new uint8_t[MemoryManager::WorkRamSize];
	_emu->RegisterMemory(SnesMemoryType::WorkRam, _workRam, MemoryManager::WorkRamSize);
	_emu->GetSettings()->InitializeRam(_workRam, MemoryManager::WorkRamSize);

	_registerHandlerA.reset(new RegisterHandlerA(
		console->GetDmaController().get(),
		console->GetInternalRegisters().get(),
		(ControlManager*)console->GetControlManager().get()
	));

	_registerHandlerB.reset(new RegisterHandlerB(
		_console,
		_ppu,
		console->GetSpc().get(),
		_workRam
	));

	for(uint32_t i = 0; i < 128 * 1024; i += 0x1000) {
		_workRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(_workRam, i, MemoryManager::WorkRamSize, SnesMemoryType::WorkRam)));
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

MemoryManager::~MemoryManager()
{
	delete[] _workRam;
}

void MemoryManager::Reset()
{
	_masterClock = 0;
	_hClock = 0;
	_dramRefreshPosition = 538 - (_masterClock & 0x07);
	_nextEventClock = _dramRefreshPosition;
	_nextEvent = SnesEventType::DramRefresh;
}

void MemoryManager::GenerateMasterClockTable()
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

void MemoryManager::IncMasterClock4()
{
	Exec();
	Exec();
}

void MemoryManager::IncMasterClock6()
{
	Exec();
	Exec();
	Exec();
}

void MemoryManager::IncMasterClock8()
{
	Exec();
	Exec();
	Exec();
	Exec();
}

void MemoryManager::IncMasterClock40()
{
	Exec(); Exec(); Exec(); Exec(); Exec();
	Exec(); Exec(); Exec(); Exec(); Exec();
	Exec(); Exec(); Exec(); Exec(); Exec();
	Exec(); Exec(); Exec(); Exec(); Exec();
}

void MemoryManager::IncMasterClockStartup()
{
	for(int i = 0; i < 182 / 2; i++) {
		Exec();
	}
}

void MemoryManager::IncrementMasterClockValue(uint16_t cyclesToRun)
{
	switch(cyclesToRun) {
		case 12: Exec();
		case 10: Exec();
		case 8: Exec();
		case 6: Exec();
		case 4: Exec();
		case 2: Exec();
	}
}

void MemoryManager::Exec()
{
	_masterClock += 2;
	_hClock += 2;

	if(_hClock == _nextEventClock) {
		ProcessEvent();
	} 
	
	if((_hClock & 0x03) == 0) {
		_emu->ProcessPpuCycle<CpuType::Cpu>();
		_regs->ProcessIrqCounters();
	}

	_cart->SyncCoprocessors();
}

void MemoryManager::ProcessEvent()
{
	switch(_nextEvent) {
		case SnesEventType::HdmaInit:
			_console->GetDmaController()->BeginHdmaInit();
			_nextEvent = SnesEventType::DramRefresh;
			_nextEventClock = _dramRefreshPosition;
			break;

		case SnesEventType::DramRefresh:
			IncMasterClock40();
			_cpu->IncreaseCycleCount<5>();

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
				_hClock = 0;

				if(_ppu->GetScanline() == 0) {
					_nextEvent = SnesEventType::HdmaInit;
					_nextEventClock = 12 + (_masterClock & 0x07);
				} else {
					_dramRefreshPosition = 538 - (_masterClock & 0x07);
					_nextEvent = SnesEventType::DramRefresh;
					_nextEventClock = _dramRefreshPosition;
				}
			} else {
				_nextEventClock += 2;
			}
			break;
	}
}

uint8_t MemoryManager::Read(uint32_t addr, MemoryOperationType type)
{
	IncrementMasterClockValue(_cpuSpeed - 4);

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
	_cheatManager->ApplyCheat(addr, value);
	_emu->ProcessMemoryRead<CpuType::Cpu>(addr, value, type);

	IncMasterClock4();
	return value;
}

uint8_t MemoryManager::ReadDma(uint32_t addr, bool forBusA)
{
	_cpu->DetectNmiSignalEdge();
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
	_cheatManager->ApplyCheat(addr, value);
	_emu->ProcessMemoryRead<CpuType::Cpu>(addr, value, MemoryOperationType::DmaRead);
	return value;
}

uint8_t MemoryManager::Peek(uint32_t addr)
{
	return _mappings.Peek(addr);
}

uint16_t MemoryManager::PeekWord(uint32_t addr)
{
	return _mappings.PeekWord(addr);
}

void MemoryManager::PeekBlock(uint32_t addr, uint8_t *dest)
{
	_mappings.PeekBlock(addr, dest);
}

void MemoryManager::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	IncrementMasterClockValue(_cpuSpeed);

	_emu->ProcessMemoryWrite<CpuType::Cpu>(addr, value, type);
	IMemoryHandler* handler = _mappings.GetHandler(addr);
	if(handler) {
		handler->Write(addr, value);
		_memTypeBusA = handler->GetMemoryType();
	} else {
		LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
	}
}

void MemoryManager::WriteDma(uint32_t addr, uint8_t value, bool forBusA)
{
	_cpu->DetectNmiSignalEdge();
	IncMasterClock4();
	_emu->ProcessMemoryWrite<CpuType::Cpu>(addr, value, MemoryOperationType::DmaWrite);

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
}

uint8_t MemoryManager::GetOpenBus()
{
	return _openBus;
}

uint64_t MemoryManager::GetMasterClock()
{
	return _masterClock;
}

uint16_t MemoryManager::GetHClock()
{
	return _hClock;
}

uint8_t * MemoryManager::DebugGetWorkRam()
{
	return _workRam;
}

MemoryMappings* MemoryManager::GetMemoryMappings()
{
	return &_mappings;
}

uint8_t MemoryManager::GetCpuSpeed(uint32_t addr)
{
	return _masterClockTable[(_regs->IsFastRomEnabled() << 10) | ((addr & 0xC00000) >> 14) | ((addr & 0xFF00) >> 8)];
}

uint8_t MemoryManager::GetCpuSpeed()
{
	return _cpuSpeed;
}

void MemoryManager::SetCpuSpeed(uint8_t speed)
{
	_cpuSpeed = speed;
}

SnesMemoryType MemoryManager::GetMemoryTypeBusA()
{
	return _memTypeBusA;
}

bool MemoryManager::IsRegister(uint32_t cpuAddress)
{
	IMemoryHandler* handler = _mappings.GetHandler(cpuAddress);
	return handler == _registerHandlerA.get() || handler == _registerHandlerB.get();
}

bool MemoryManager::IsWorkRam(uint32_t cpuAddress)
{
	IMemoryHandler* handler = _mappings.GetHandler(cpuAddress);
	return handler && handler->GetMemoryType() == SnesMemoryType::WorkRam;
}

void MemoryManager::Serialize(Serializer &s)
{
	s.Stream(_masterClock, _openBus, _cpuSpeed, _hClock, _dramRefreshPosition);
	s.StreamArray(_workRam, MemoryManager::WorkRamSize);

	if(s.GetVersion() < 8) {
		bool unusedHasEvent[1369];
		s.StreamArray(unusedHasEvent, sizeof(unusedHasEvent));
	}

	s.Stream(_registerHandlerB.get());
}
