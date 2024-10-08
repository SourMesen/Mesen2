#include "pch.h"
#include "NES/NesMemoryManager.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "Shared/CheatManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryOperationType.h"

NesMemoryManager::NesMemoryManager(NesConsole* console, BaseMapper* mapper)
{
	_console = console;
	_emu = console->GetEmulator();
	_cheatManager = _emu->GetCheatManager();
	_mapper = mapper;

	_internalRamSize = mapper->GetInternalRamSize();
	_internalRam = new uint8_t[_internalRamSize];
	_emu->RegisterMemory(MemoryType::NesInternalRam, _internalRam, _internalRamSize);
	if(_internalRamSize == NesMemoryManager::NesInternalRamSize) {
		_internalRamHandler.reset(new InternalRamHandler<0x7FF>());
		((InternalRamHandler<0x7FF>*)_internalRamHandler.get())->SetInternalRam(_internalRam);
	} else if(_internalRamSize == NesMemoryManager::FamicomBoxInternalRamSize) {
		_internalRamHandler.reset(new InternalRamHandler<0x1FFF>());
		((InternalRamHandler<0x1FFF>*)_internalRamHandler.get())->SetInternalRam(_internalRam);
	} else {
		throw std::runtime_error("unsupported memory size");
	}

	_ramReadHandlers = new INesMemoryHandler*[NesMemoryManager::CpuMemorySize];
	_ramWriteHandlers = new INesMemoryHandler*[NesMemoryManager::CpuMemorySize];

	for(int i = 0; i < NesMemoryManager::CpuMemorySize; i++) {
		_ramReadHandlers[i] = &_openBusHandler;
		_ramWriteHandlers[i] = &_openBusHandler;
	}

	RegisterIODevice(_internalRamHandler.get());	
}

NesMemoryManager::~NesMemoryManager()
{
	delete[] _internalRam;

	delete[] _ramReadHandlers;
	delete[] _ramWriteHandlers;
}

void NesMemoryManager::Reset(bool softReset)
{
	if(!softReset) {
		_console->InitializeRam(_internalRam, _internalRamSize);
	}

	_mapper->Reset(softReset);
}

void NesMemoryManager::InitializeMemoryHandlers(INesMemoryHandler** memoryHandlers, INesMemoryHandler* handler, vector<uint16_t> *addresses, bool allowOverride)
{
	for(uint16_t address : *addresses) {
		if(!allowOverride && memoryHandlers[address] != &_openBusHandler && memoryHandlers[address] != handler) {
			throw std::runtime_error("Can't override existing mapping");
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
	for(uint32_t i = start; i <= end; i++) {
		_ramWriteHandlers[i] = handler;
	}
}

void NesMemoryManager::RegisterReadHandler(INesMemoryHandler* handler, uint32_t start, uint32_t end)
{
	for(uint32_t i = start; i <= end; i++) {
		_ramReadHandlers[i] = handler;
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

uint8_t* NesMemoryManager::GetInternalRam()
{
	return _internalRam;
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

	_openBusHandler.SetOpenBus(value, addr == 0x4015);

	return value;
}

void NesMemoryManager::Write(uint16_t addr, uint8_t value, MemoryOperationType operationType)
{
	if(_emu->ProcessMemoryWrite<CpuType::Nes>(addr, value, operationType)) {
		_ramWriteHandlers[addr]->WriteRam(addr, value);
		_openBusHandler.SetOpenBus(value, false);
	}
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
					((BaseMapper*)handler)->DebugWriteRam(addr, value);
				}
			} else {
				handler->WriteRam(addr, value);
			}
		}
	}
}

void NesMemoryManager::Serialize(Serializer &s)
{
	SVArray(_internalRam, _internalRamSize);
	SV(_openBusHandler);
}

uint8_t NesMemoryManager::GetOpenBus(uint8_t mask)
{
	return _openBusHandler.GetOpenBus() & mask;
}

uint8_t NesMemoryManager::GetInternalOpenBus(uint8_t mask)
{
	return _openBusHandler.GetInternalOpenBus() & mask;
}
