#include "stdafx.h"
#include "MemoryManager.h"
#include "Console.h"
#include "BaseCartridge.h"
#include "Cpu.h"
#include "Ppu.h"
#include "DmaController.h"
#include "RegisterHandlerA.h"
#include "RegisterHandlerB.h"
#include "RamHandler.h"
#include "MessageManager.h"
#include "DebugTypes.h"
#include "EmuSettings.h"
#include "Sa1.h"
#include "Gsu.h"
#include "Cx4.h"
#include "BaseCoprocessor.h"
#include "CheatManager.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/HexUtilities.h"

void MemoryManager::Initialize(Console *console)
{
	_masterClock = 0;
	_openBus = 0;
	_cpuSpeed = 8;
	_console = console;
	_regs = console->GetInternalRegisters().get();
	_cpu = console->GetCpu().get();
	_ppu = console->GetPpu().get();
	_cart = console->GetCartridge().get();
	_cheatManager = console->GetCheatManager().get();

	_workRam = new uint8_t[MemoryManager::WorkRamSize];
	_console->GetSettings()->InitializeRam(_workRam, MemoryManager::WorkRamSize);

	_registerHandlerA.reset(new RegisterHandlerA(
		console->GetDmaController().get(),
		console->GetInternalRegisters().get(),
		console->GetControlManager().get()
	));

	_registerHandlerB.reset(new RegisterHandlerB(
		_console,
		_ppu,
		console->GetSpc().get(),
		_workRam
	));

	memset(_hasEvent, 0, sizeof(_hasEvent));
	_hasEvent[276 * 4] = true;
	_hasEvent[285 * 4] = true;
	_hasEvent[1360] = true;
	_hasEvent[1364] = true;
	_hasEvent[1368] = true;

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
	UpdateEvents();
}

MemoryManager::~MemoryManager()
{
	delete[] _workRam;
}

void MemoryManager::Reset()
{
	_masterClock = 0;
	_hClock = 0;
	UpdateEvents();
}

void MemoryManager::GenerateMasterClockTable()
{
	for(int j = 0; j < 2; j++) {
		for(int i = 0; i < 0x10000; i++) {
			uint8_t bank = (i & 0xFF00) >> 8;
			if(bank >= 0x40 && bank <= 0x7F) {
				//Slow
				_masterClockTable[j][i] = 8;
			} else if(bank >= 0xC0) {
				//Banks $C0-$FF
				//Slow or fast (depending on register)
				_masterClockTable[j][i] = j == 1 ? 6 : 8;
			} else {
				//Banks $00-$3F and $80-$BF
				uint8_t page = (i & 0xFF);
				if(page <= 0x1F) {
					//Slow
					_masterClockTable[j][i] = 8;
				} else if(page >= 0x20 && page <= 0x3F) {
					//Fast
					_masterClockTable[j][i] = 6;
				} else if(page == 0x40 || page == 0x41) {
					//Extra slow
					_masterClockTable[j][i] = 12;
				} else if(page >= 0x42 && page <= 0x5F) {
					//Fast
					_masterClockTable[j][i] = 6;
				} else if(page >= 0x60 && page <= 0x7F) {
					//Slow
					_masterClockTable[j][i] = 8;
				} else if(bank <= 0x3F) {
					//Slow
					_masterClockTable[j][i] = 8;
				} else {
					//page >= $80
					//Slow or fast (depending on register)
					_masterClockTable[j][i] = j == 1 ? 6 : 8;
				}
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

void MemoryManager::UpdateEvents()
{
	_hasEvent[_hdmaInitPosition] = false;
	_hasEvent[_dramRefreshPosition] = false;

	if(_ppu->GetScanline() == 0) {
		_hdmaInitPosition = 12 + (_masterClock & 0x07);
		_hasEvent[_hdmaInitPosition] = true;
	}

	_dramRefreshPosition = 538 - (_masterClock & 0x07);
	_hasEvent[_dramRefreshPosition] = true;
}

void MemoryManager::SyncCoprocessors()
{
	if(_cart->GetCoprocessor()) {
		if(_cart->GetGsu()) {
			_cart->GetGsu()->Run();
		} else if(_cart->GetSa1()) {
			_cart->GetSa1()->Run();
		} else if(_cart->GetCx4()) {
			_cart->GetCx4()->Run();
		}
	}
}

void MemoryManager::Exec()
{
	_masterClock += 2;
	_hClock += 2;

	if(_hasEvent[_hClock]) {
		if(_hClock >= 1360 && _ppu->ProcessEndOfScanline(_hClock)) {
			_hClock = 0;
			UpdateEvents();
		}

		if((_hClock & 0x03) == 0) {
			_console->ProcessPpuCycle();
			_regs->ProcessIrqCounters();

			if(_hClock == 276 * 4 && _ppu->GetScanline() < _ppu->GetVblankStart()) {
				_console->GetDmaController()->BeginHdmaTransfer();
			}
		}

		 if(_hClock == _hdmaInitPosition && _ppu->GetScanline() == 0) {
			_console->GetDmaController()->BeginHdmaInit();
		}

		if(_hClock == _dramRefreshPosition) {
			IncMasterClock40();
			_cpu->IncreaseCycleCount<5>();
		}
	} else if((_hClock & 0x03) == 0) {
		_console->ProcessPpuCycle();
		_regs->ProcessIrqCounters();
	}

	SyncCoprocessors();
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
	_console->ProcessMemoryRead<CpuType::Cpu>(addr, value, type);

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
	_console->ProcessMemoryRead<CpuType::Cpu>(addr, value, MemoryOperationType::DmaRead);
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

	_console->ProcessMemoryWrite<CpuType::Cpu>(addr, value, type);
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
	_console->ProcessMemoryWrite<CpuType::Cpu>(addr, value, MemoryOperationType::DmaWrite);

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
	return _masterClockTable[(uint8_t)_regs->IsFastRomEnabled()][addr >> 8];
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
	s.Stream(_masterClock, _openBus, _cpuSpeed, _hClock, _dramRefreshPosition, _hdmaInitPosition);
	s.StreamArray(_workRam, MemoryManager::WorkRamSize);
	s.StreamArray(_hasEvent, sizeof(_hasEvent));
	s.Stream(_registerHandlerB.get());
}
