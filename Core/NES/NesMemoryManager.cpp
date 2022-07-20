#include "stdafx.h"
#include "NES/NesMemoryManager.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "Shared/CheatManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"
#include "MemoryOperationType.h"

NesMemoryManager::NesMemoryManager(NesConsole* console)
{
	_console = console;
	_emu = console->GetEmulator();
	_cheatManager = _emu->GetCheatManager();
	_internalRAM = new uint8_t[InternalRAMSize];
	_emu->RegisterMemory(MemoryType::NesInternalRam, _internalRAM, InternalRAMSize);
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

void NesMemoryManager::SetMapper(BaseMapper* mapper)
{
	_mapper = mapper;
}

void NesMemoryManager::Reset(bool softReset)
{
	if(!softReset) {
		_emu->GetSettings()->InitializeRam(_internalRAM, InternalRAMSize);
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

uint8_t NesMemoryManager::DebugRead(uint16_t addr)
{
	uint8_t value = _ramReadHandlers[addr]->PeekRam(addr);
	if(_cheatManager->HasCheats<CpuType::Nes>()) {
		_cheatManager->ApplyCheat<CpuType::Nes>(addr, value);
	}
	return value;
}

uint16_t NesMemoryManager::DebugReadWord(uint16_t addr)
{
	return DebugRead(addr) | (DebugRead(addr + 1) << 8);
}

uint8_t NesMemoryManager::Read(uint16_t addr, MemoryOperationType operationType)
{
	uint8_t value = _ramReadHandlers[addr]->ReadRam(addr);
	if(_cheatManager->HasCheats<CpuType::Nes>()) {
		_cheatManager->ApplyCheat<CpuType::Nes>(addr, value);
	}
	_emu->ProcessMemoryRead<CpuType::Nes>(addr, value, operationType);

	_openBusHandler.SetOpenBus(value);

	return value;
}

void NesMemoryManager::Write(uint16_t addr, uint8_t value, MemoryOperationType operationType)
{
	_emu->ProcessMemoryWrite<CpuType::Nes>(addr, value, operationType);
	_ramWriteHandlers[addr]->WriteRam(addr, value);
}

void NesMemoryManager::DebugWrite(uint16_t addr, uint8_t value, bool disableSideEffects)
{
	if(addr <= 0x1FFF) {
		_ramWriteHandlers[addr]->WriteRam(addr, value);
	} else {
		INesMemoryHandler* handler = _ramReadHandlers[addr];
		if(handler) {
			if(disableSideEffects) {
				if(handler == _mapper) {
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
	SVArray(_internalRAM, NesMemoryManager::InternalRAMSize);
}

uint8_t NesMemoryManager::GetOpenBus(uint8_t mask)
{
	return _openBusHandler.GetOpenBus() & mask;
}
