#include "stdafx.h"
#include "../../Utilities/Serializer.h"
#include "NesMemoryManager.h"
#include "BaseMapper.h"
//#include "Debugger.h"
#include "CheatManager.h"
#include "Emulator.h"
#include "NesConsole.h"
#include "MemoryOperationType.h"

NesMemoryManager::NesMemoryManager(shared_ptr<NesConsole> console)
{
	_console = console;
	_emu = console->GetEmulator();
	_cheatManager = _emu->GetCheatManager();
	_internalRAM = new uint8_t[InternalRAMSize];
	_emu->RegisterMemory(SnesMemoryType::NesInternalRam, _internalRAM, InternalRAMSize);
	_internalRamHandler.SetInternalRam(_internalRAM);

	_ramReadHandlers = new INesMemoryHandler*[RAMSize];
	_ramWriteHandlers = new INesMemoryHandler*[RAMSize];

	for(int i = 0; i < RAMSize; i++) {
		_ramReadHandlers[i] = &_openBusHandler;
		_ramWriteHandlers[i] = &_openBusHandler;
	}

	RegisterIODevice(&_internalRamHandler);	
}

NesMemoryManager::~NesMemoryManager()
{
	delete[] _internalRAM;

	delete[] _ramReadHandlers;
	delete[] _ramWriteHandlers;
}

void NesMemoryManager::SetMapper(shared_ptr<BaseMapper> mapper)
{
	_mapper = mapper;
}

void NesMemoryManager::Reset(bool softReset)
{
	if(!softReset) {
		_console->InitializeRam(_internalRAM, InternalRAMSize);
	}

	_mapper->Reset(softReset);
}

void NesMemoryManager::InitializeMemoryHandlers(INesMemoryHandler** memoryHandlers, INesMemoryHandler* handler, vector<uint16_t> *addresses, bool allowOverride)
{
	for(uint16_t address : *addresses) {
		if(!allowOverride && memoryHandlers[address] != &_openBusHandler && memoryHandlers[address] != handler) {
			throw std::runtime_error("Not supported");
		}
		memoryHandlers[address] = handler;
	}
}

void NesMemoryManager::RegisterIODevice(INesMemoryHandler*handler)
{
	MemoryRanges ranges;
	handler->GetMemoryRanges(ranges);

	InitializeMemoryHandlers(_ramReadHandlers, handler, ranges.GetRAMReadAddresses(), ranges.GetAllowOverride());
	InitializeMemoryHandlers(_ramWriteHandlers, handler, ranges.GetRAMWriteAddresses(), ranges.GetAllowOverride());
}

void NesMemoryManager::RegisterWriteHandler(INesMemoryHandler* handler, uint32_t start, uint32_t end)
{
	for(uint32_t i = start; i < end; i++) {
		_ramWriteHandlers[i] = handler;
	}
}

void NesMemoryManager::UnregisterIODevice(INesMemoryHandler*handler)
{
	MemoryRanges ranges;
	handler->GetMemoryRanges(ranges);

	for(uint16_t address : *ranges.GetRAMReadAddresses()) {
		_ramReadHandlers[address] = &_openBusHandler;
	}

	for(uint16_t address : *ranges.GetRAMWriteAddresses()) {
		_ramWriteHandlers[address] = &_openBusHandler;
	}
}

uint8_t* NesMemoryManager::GetInternalRAM()
{
	return _internalRAM;
}

uint8_t NesMemoryManager::DebugRead(uint16_t addr, bool disableSideEffects)
{
	uint8_t value = 0x00;
	if(addr <= 0x1FFF) {
		value = _ramReadHandlers[addr]->ReadRam(addr);
	} else {
		INesMemoryHandler* handler = _ramReadHandlers[addr];
		if(handler) {
			if(disableSideEffects) {
				value = handler->PeekRam(addr);
			} else {
				value = handler->ReadRam(addr);
			}
		} else {
			//Fake open bus
			value = addr >> 8;
		}
	}

	_cheatManager->ApplyCheat(addr, value);

	return value;
}

uint16_t NesMemoryManager::DebugReadWord(uint16_t addr)
{
	return DebugRead(addr) | (DebugRead(addr + 1) << 8);
}

uint8_t NesMemoryManager::Read(uint16_t addr, MemoryOperationType operationType)
{
	uint8_t value = _ramReadHandlers[addr]->ReadRam(addr);
	_cheatManager->ApplyCheat(addr, value);
	_emu->ProcessMemoryRead<CpuType::Nes>(addr, value, operationType);

	_openBusHandler.SetOpenBus(value);

	return value;
}

void NesMemoryManager::Write(uint16_t addr, uint8_t value, MemoryOperationType operationType)
{
	_emu->ProcessMemoryWrite<CpuType::Nes>(addr, value, operationType);
	//TODO
	//if(_console->DebugProcessRamOperation(operationType, addr, value)) {
		_ramWriteHandlers[addr]->WriteRam(addr, value);
	//}
}

void NesMemoryManager::DebugWrite(uint16_t addr, uint8_t value, bool disableSideEffects)
{
	if(addr <= 0x1FFF) {
		_ramWriteHandlers[addr]->WriteRam(addr, value);
	} else {
		INesMemoryHandler* handler = _ramReadHandlers[addr];
		if(handler) {
			if(disableSideEffects) {
				if(handler == _mapper.get()) {
					//Only allow writes to prg/chr ram/rom (e.g not ppu, apu, mapper registers, etc.)
					((BaseMapper*)handler)->DebugWriteRAM(addr, value);
				}
			} else {
				handler->WriteRam(addr, value);
			}
		}
	}
}

void NesMemoryManager::Serialize(Serializer &s)
{
	ArrayInfo<uint8_t> internalRam = { _internalRAM, NesMemoryManager::InternalRAMSize };
	s.Stream(internalRam);
}

uint8_t NesMemoryManager::GetOpenBus(uint8_t mask)
{
	return _openBusHandler.GetOpenBus() & mask;
}
