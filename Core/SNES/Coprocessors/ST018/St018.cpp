#include "pch.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/MemoryMappings.h"
#include "SNES/Coprocessors/ST018/St018.h"
#include "SNES/Coprocessors/ST018/ArmV3Cpu.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "SNES/Debugger/DummyArmV3Cpu.h"
#include "Shared/Emulator.h"
#include "Shared/FirmwareHelper.h"
#include "GBA/GbaCpu.h"

St018::St018(SnesConsole* console)
{
	_emu = console->GetEmulator();
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_memoryManager->GetMemoryMappings()->RegisterHandler(0x00, 0x3F, 0x3000, 0x3FFF, this);
	_memoryManager->GetMemoryMappings()->RegisterHandler(0x80, 0xBF, 0x3000, 0x3FFF, this);

	ArmV3Cpu::StaticInit();
	DummyArmV3Cpu::StaticInit();
	GbaCpu::StaticInit(); //todo remove

	_prgRom = new uint8_t[St018::PrgRomSize];
	_dataRom = new uint8_t[St018::DataRomSize];
	
	_workRam = new uint8_t[St018::WorkRamSize];
	_console->InitializeRam(_workRam, St018::WorkRamSize);

	vector<uint8_t> firmwareData;
	FirmwareHelper::LoadSt018Firmware(_emu, firmwareData);
	if(firmwareData.size() == 0x28000) {
		memcpy(_prgRom, firmwareData.data(), St018::PrgRomSize);
		memcpy(_dataRom, firmwareData.data() + St018::PrgRomSize, St018::DataRomSize);
	}

	_emu->RegisterMemory(MemoryType::St018PrgRom, _prgRom, St018::PrgRomSize);
	_emu->RegisterMemory(MemoryType::St018DataRom, _dataRom, St018::DataRomSize);
	_emu->RegisterMemory(MemoryType::St018WorkRam, _workRam, St018::WorkRamSize);

	_cpu.reset(new ArmV3Cpu());
	_cpu->Init(_emu, this);
	_cpu->PowerOn(false);
}

St018::~St018()
{
	delete[] _prgRom;
	delete[] _dataRom;
	delete[] _workRam;
}

void St018::Reset()
{
}

void St018::Run()
{
	ArmV3CpuState& state = _cpu->GetState();

	uint64_t targetCycle = _memoryManager->GetMasterClock();

	if(_state.ArmReset) {
		state.CycleCount = targetCycle;
	}

	while(state.CycleCount < targetCycle) {
		_cpu->Exec();
	}
}

void St018::ProcessEndOfFrame()
{
	Run();
}

uint8_t St018::Read(uint32_t addr)
{
	Run();

	switch(addr & 0xFF06) {
		case 0x3800:
			_state.HasDataForSnes = false;
			return _state.DataSnes;

		case 0x3802:
			_state.Ack = false;
			break;

		case 0x3804:
			return GetStatus();
	}

	return _memoryManager->GetOpenBus();
}

uint8_t St018::Peek(uint32_t addr)
{
	switch(addr & 0xFF06) {
		case 0x3800: return _state.DataSnes;
		case 0x3802: break;
		case 0x3804: return GetStatus();
	}

	return _memoryManager->GetOpenBus();
}

void St018::PeekBlock(uint32_t addr, uint8_t* output)
{
	for(int i = 0; i < 0x1000; i++) {
		output[i] = Peek(i);
	}
}

void St018::Write(uint32_t addr, uint8_t value)
{
	Run();

	switch(addr & 0xFF06) {
		case 0x3802: 
			_state.DataArm = value;
			_state.HasDataForArm = true;
			break;

		case 0x3804:
			if(_state.ArmReset && !value) {
				_cpu->PowerOn(true);
			}
			_state.ArmReset = value != 0;
			break;
	}
}

AddressInfo St018::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

AddressInfo St018::GetArmAbsoluteAddress(uint32_t addr)
{
	switch(addr >> 28) {
		case 0x00: return { (int)(addr & 0x1FFFF), MemoryType::St018PrgRom };
		case 0x0A: return { (int)(addr & 0x7FFF), MemoryType::St018DataRom};
		case 0x0E: return { (int)(addr & 0x3FFF), MemoryType::St018WorkRam };
	}

	return { -1, MemoryType::None };
}

int St018::GetArmRelativeAddress(AddressInfo& absoluteAddr)
{
	switch(absoluteAddr.Type) {
		case MemoryType::St018PrgRom: return absoluteAddr.Address;
		case MemoryType::St018DataRom: return 0xA0000000 | absoluteAddr.Address;
		case MemoryType::St018WorkRam: return 0xE0000000 | absoluteAddr.Address;
	}

	return -1;
}

void St018::LoadBattery()
{
}

void St018::SaveBattery()
{
}

St018State& St018::GetState()
{
	return _state;
}

uint32_t St018::ReadCpu(ArmV3AccessModeVal mode, uint32_t addr)
{
	_cpu->GetState().CycleCount++;

	uint32_t value;
	if(mode & ArmV3AccessMode::Word) {
		uint8_t b0 = ReadCpuByte(addr & ~0x03);
		uint8_t b1 = ReadCpuByte((addr & ~0x03) | 1);
		uint8_t b2 = ReadCpuByte((addr & ~0x03) | 2);
		uint8_t b3 = ReadCpuByte(addr | 3);
		value = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
		_emu->ProcessMemoryRead<CpuType::St018, 4>(addr & ~0x03, value, mode & ArmV3AccessMode::Prefetch ? MemoryOperationType::ExecOpCode : MemoryOperationType::Read);
	} else {
		value = ReadCpuByte(addr);
		_emu->ProcessMemoryRead<CpuType::St018, 1>(addr, value, MemoryOperationType::Read);
	}
	return value;
}

uint8_t St018::ReadCpuByte(uint32_t addr)
{
	switch(addr >> 28) {
		case 0x00: return _prgRom[addr & 0x1FFFF];
		case 0x02: return 0;
		case 0x04:
			switch(addr & 0x3F) {
				case 0x10:
					_state.HasDataForArm = false;
					return _state.DataArm;

				case 0x20: return GetStatus();

			}
			return 0;

		case 0x06: return 0;
		case 0x08: return 0;
		case 0x0A: return _dataRom[addr & 0x7FFF];
		case 0x0C: return 0;
		case 0x0E: return _workRam[addr & 0x3FFF];
	}

	return 0;
}

uint32_t St018::DebugCpuRead(ArmV3AccessModeVal mode, uint32_t addr)
{
	if(mode & ArmV3AccessMode::Byte) {
		return DebugRead(addr);
	} else {
		uint8_t b0 = DebugRead(addr & ~0x03);
		uint8_t b1 = DebugRead((addr & ~0x03) | 1);
		uint8_t b2 = DebugRead((addr & ~0x03) | 2);
		uint8_t b3 = DebugRead(addr | 3);
		return  b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
	}
}

uint8_t St018::DebugRead(uint32_t addr)
{
	switch(addr >> 28) {
		case 0x00: return _prgRom[addr & 0x1FFFF];
		case 0x02: return 0;
		case 0x04: return 0;
		case 0x06: return 0;
		case 0x08: return 0;
		case 0x0A: return _dataRom[addr & 0x7FFF];
		case 0x0C: return 0;
		case 0x0E: return _workRam[addr & 0x3FFF];
	}

	return 0;
}

void St018::DebugWrite(uint32_t addr, uint8_t value)
{
	switch(addr >> 28) {
		case 0x00: _prgRom[addr & 0x1FFFF] = value; break;
		case 0x0A: _dataRom[addr & 0x7FFF] = value; break;
		case 0x0E: _workRam[addr & 0x3FFF] = value; break;
	}
}

void St018::WriteCpu(ArmV3AccessModeVal mode, uint32_t addr, uint32_t value)
{
	_cpu->GetState().CycleCount++;
	
	if(mode & ArmV3AccessMode::Word) {
		if(_emu->ProcessMemoryWrite<CpuType::St018, 4>(addr & ~0x03, value, MemoryOperationType::Write)) {
			WriteCpuByte((addr & ~0x03), (uint8_t)value);
			WriteCpuByte((addr & ~0x03) | 0x01, (uint8_t)(value >> 8));
			WriteCpuByte((addr & ~0x03) | 0x02, (uint8_t)(value >> 16));
			WriteCpuByte((addr & ~0x03) | 0x03, (uint8_t)(value >> 24));
		}
	} else {
		if(_emu->ProcessMemoryWrite<CpuType::St018, 1>(addr, value, MemoryOperationType::Write)) {
			WriteCpuByte(addr, value);
		}
	}
}

void St018::WriteCpuByte(uint32_t addr, uint8_t value)
{
	switch(addr >> 28) {
		case 0x00: break;
		case 0x02: break;
		case 0x04:
			switch(addr & 0x3F) {
				case 0x00:
					_state.HasDataForSnes = true;
					_state.DataSnes = value;
					break;

				case 0x10:
					_state.Ack = true;
					break;

				case 0x20:
				case 0x24:
				case 0x28:
				case 0x2A:
					//??
					break;
			}
			break;

		case 0x06: break;
		case 0x08: break;
		case 0x0A: break;
		case 0x0C: break;
		case 0x0E: _workRam[addr & 0x3FFF] = value; break;
	}
}

void St018::ProcessIdleCycle()
{
	_cpu->GetState().CycleCount++;
}

uint8_t St018::GetStatus()
{
	return (
		(_state.HasDataForSnes ? 0x01 : 0) |
		(_state.Ack ? 0x04 : 0) |
		(_state.HasDataForArm ? 0x08 : 0) |
		(!_state.ArmReset ? 0x80 : 0)
	);
}

void St018::Serialize(Serializer& s)
{
	SV(_state.Ack);
	SV(_state.ArmReset);
	SV(_state.DataArm);
	SV(_state.DataSnes);
	SV(_state.HasDataForArm);
	SV(_state.HasDataForSnes);
	SV(_cpu);
}
