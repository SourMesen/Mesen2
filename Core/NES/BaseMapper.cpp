#include "pch.h"
#include <random>
#include <assert.h>
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesTypes.h"
#include "NES/NesMemoryManager.h"
#include "NES/RomData.h"
#include "NES/Epsm.h"
#include "Debugger/DebugTypes.h"
#include "Shared/MessageManager.h"
#include "Shared/CheatManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/RomInfo.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryType.h"
#include "Shared/MemoryOperationType.h"
#include "Shared/FirmwareHelper.h"

void BaseMapper::WriteRegister(uint16_t addr, uint8_t value) { }
uint8_t BaseMapper::ReadRegister(uint16_t addr) { return 0; }
void BaseMapper::InitMapper(RomData &romData) { }
void BaseMapper::Reset(bool softReset) { }

//Make sure the page size is no bigger than the size of the ROM itself
//Otherwise we will end up reading from unallocated memory
uint16_t BaseMapper::InternalGetPrgPageSize() { return std::min((uint32_t)GetPrgPageSize(), _prgSize); }
uint16_t BaseMapper::InternalGetSaveRamPageSize() { return std::min((uint32_t)GetSaveRamPageSize(), _saveRamSize); }
uint16_t BaseMapper::InternalGetWorkRamPageSize() { return std::min((uint32_t)GetWorkRamPageSize(), _workRamSize); }
uint16_t BaseMapper::InternalGetChrRomPageSize() { return std::min((uint32_t)GetChrPageSize(), _chrRomSize); }
uint16_t BaseMapper::InternalGetChrRamPageSize() { return std::min((uint32_t)GetChrRamPageSize(), _chrRamSize); }
	
bool BaseMapper::ValidateAddressRange(uint16_t startAddr, uint16_t endAddr)
{
	if((startAddr & 0xFF) || (endAddr & 0xFF) != 0xFF) {
		#ifdef _DEBUG
			throw new std::runtime_error("Start/End address must be multiples of 256/0x100");
		#else
			//Ignore this request in release mode - granularity smaller than 256 bytes is not supported
			return false;
		#endif
	}
	return true;
}

void BaseMapper::SetCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, int16_t pageNumber, PrgMemoryType type, int8_t accessType)
{
	if(!ValidateAddressRange(startAddr, endAddr) || startAddr > 0xFF00 || endAddr <= startAddr) {
		return;
	}

	uint32_t pageCount;
	uint32_t pageSize;
	uint8_t defaultAccessType = MemoryAccessType::Read;
	switch(type) {
		case PrgMemoryType::PrgRom:
			pageCount = GetPrgPageCount();
			pageSize = InternalGetPrgPageSize();
			break;
		case PrgMemoryType::SaveRam:
			pageSize = InternalGetSaveRamPageSize();
			if(pageSize == 0) {
				#ifdef _DEBUG
				MessageManager::DisplayMessage("Debug", "Tried to map undefined save ram.");
				#endif
				return;
			}
			pageCount = _saveRamSize / pageSize;
			
			defaultAccessType |= MemoryAccessType::Write;
			break;
		case PrgMemoryType::WorkRam:
			pageSize = InternalGetWorkRamPageSize();
			if(pageSize == 0) {
				#ifdef _DEBUG
				MessageManager::DisplayMessage("Debug", "Tried to map undefined work ram.");
				#endif
				return;
			}

			pageCount = _workRamSize / pageSize;
			
			defaultAccessType |= MemoryAccessType::Write;
			break;
		default:
			throw new std::runtime_error("Invalid parameter");
	}

	if(pageCount == 0) {
		#ifdef _DEBUG
		MessageManager::DisplayMessage("Debug", "Tried to map undefined save/work ram.");
		#endif
		return;
	}

	auto wrapPageNumber = [=](int16_t &page) -> void {
		if(page < 0) {
			//Can't use modulo for negative number because pageCount is sometimes not a power of 2.  (Fixes some Mapper 191 games)
			page = pageCount + page;
		} else {
			page = page % pageCount;
		}
	};
	wrapPageNumber(pageNumber);
	
	accessType = accessType != -1 ? accessType : defaultAccessType;
	
	if((uint16_t)(endAddr - startAddr) >= pageSize) {
		#ifdef _DEBUG
		uint16_t gap = endAddr - startAddr + 1;
		if(gap % pageSize != 0) {
			MessageManager::DisplayMessage("Debug", "Tried to map undefined prg - page size too small for selected range.");
		}
		#endif
		
		//If range is bigger than a single page, keep going until we reach the last page
		uint32_t addr = startAddr;
		while(addr <= endAddr - pageSize + 1) {
			SetCpuMemoryMapping(addr, addr + pageSize - 1, type, pageNumber * pageSize, accessType);
			addr += pageSize;
			pageNumber++;
			wrapPageNumber(pageNumber);
		}
	} else {
		SetCpuMemoryMapping(startAddr, endAddr, type, pageNumber * pageSize, accessType);
	}
}

void BaseMapper::SetCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, PrgMemoryType type, uint32_t sourceOffset, int8_t accessType)
{
	uint8_t* source = nullptr;
	uint32_t sourceSize = 0;
	switch(type) {
		default:
		case PrgMemoryType::PrgRom: source = _prgRom; sourceSize = _prgSize; break;
		case PrgMemoryType::SaveRam: source = _saveRam; sourceSize = _saveRamSize; break;
		case PrgMemoryType::WorkRam: source = _workRam; sourceSize = _workRamSize; break;
		case PrgMemoryType::MapperRam: source = _mapperRam; sourceSize = _mapperRamSize; break;
	}

	int firstSlot = startAddr >> 8;
	int slotCount = (endAddr - startAddr + 1) >> 8;
	for(int i = 0; i < slotCount; i++) {
		if(sourceSize == 0) {
			_prgPages[i] = nullptr;
			_prgMemoryAccess[i] = MemoryAccessType::NoAccess;
		} else {
			while(sourceOffset >= sourceSize) {
				sourceOffset -= sourceSize;
			}
			_prgPages[firstSlot + i] = source + sourceOffset;
			_prgMemoryOffset[firstSlot + i] = sourceOffset + i * 0x100;
			_prgMemoryType[firstSlot + i] = type;
			_prgMemoryAccess[firstSlot + i] = (MemoryAccessType)accessType;
		}
	}

	SetCpuMemoryMapping(startAddr, endAddr, source, sourceOffset, sourceSize, accessType);
}

void BaseMapper::SetCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, uint8_t *source, uint32_t sourceOffset, uint32_t sourceSize, int8_t accessType)
{
	if(!ValidateAddressRange(startAddr, endAddr)) {
		return;
	}

	startAddr >>= 8;
	endAddr >>= 8;
	for(uint16_t i = startAddr; i <= endAddr; i++) {
		if(source && sourceSize > 0 && sourceOffset <= sourceSize - 0x100) {
			_prgPages[i] = source + sourceOffset;
			_prgMemoryAccess[i] = accessType != -1 ? (MemoryAccessType)accessType : MemoryAccessType::ReadWrite;
		} else {
			_prgPages[i] = nullptr;
			_prgMemoryAccess[i] = MemoryAccessType::NoAccess;
		}

		sourceOffset += 0x100;
	}
}

void BaseMapper::RemoveCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr)
{
	//Unmap this section of memory (causing open bus behavior)
	int firstSlot = startAddr >> 8;
	int slotCount = (endAddr - startAddr + 1) >> 8;
	for(int i = 0; i < slotCount; i++) {
		_prgMemoryOffset[firstSlot + i] = -1;
		_prgMemoryType[firstSlot + i] = PrgMemoryType::PrgRom;
		_prgMemoryAccess[firstSlot + i] = MemoryAccessType::NoAccess;
	}

	SetCpuMemoryMapping(startAddr, endAddr, nullptr, 0, 0, MemoryAccessType::NoAccess);
}

void BaseMapper::SetPpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, uint16_t pageNumber, ChrMemoryType type, int8_t accessType)
{
	if(!ValidateAddressRange(startAddr, endAddr) || startAddr > 0x3F00 || endAddr > 0x3FFF || endAddr <= startAddr) {
		return;
	}

	uint32_t pageCount = 0;
	uint32_t pageSize = 0;
	uint8_t defaultAccessType = MemoryAccessType::Read;
	if(type == ChrMemoryType::Default) {
		if(_chrRomSize > 0) {
			type = ChrMemoryType::ChrRom;
		} else {
			type = ChrMemoryType::ChrRam;
		}
	}

	switch(type) {
		case ChrMemoryType::Default:
		case ChrMemoryType::ChrRom:
			pageSize = InternalGetChrRomPageSize();
			if(pageSize == 0) {
				#ifdef _DEBUG
				MessageManager::DisplayMessage("Debug", "Tried to map undefined chr rom.");
				#endif
				return;
			}
			pageCount = GetChrRomPageCount();
			break;

		case ChrMemoryType::ChrRam:
			pageSize = InternalGetChrRamPageSize();
			if(pageSize == 0) {
				#ifdef _DEBUG
				MessageManager::DisplayMessage("Debug", "Tried to map undefined chr ram.");
				#endif
				return;
			}
			pageCount = _chrRamSize / pageSize;
			defaultAccessType |= MemoryAccessType::Write;
			break;

		case ChrMemoryType::NametableRam:
			pageSize = BaseMapper::NametableSize;
			pageCount = _nametableCount;
			defaultAccessType |= MemoryAccessType::Write;
			break;

		default:
			throw new std::runtime_error("Invalid parameter");
	}

	if(pageCount == 0) {
		#ifdef _DEBUG
		MessageManager::DisplayMessage("Debug", "Tried to map undefined chr ram.");
		#endif
		return;
	}

	pageNumber = pageNumber % pageCount;

	if((uint16_t)(endAddr - startAddr) >= pageSize) {
		#ifdef _DEBUG
		MessageManager::DisplayMessage("Debug", "Tried to map undefined chr - page size too small for selected range.");
		#endif

		uint32_t addr = startAddr;
		while(addr <= endAddr - pageSize + 1) {
			SetPpuMemoryMapping(addr, addr + pageSize - 1, type, pageNumber * pageSize, accessType);
			addr += pageSize;
			pageNumber = (pageNumber + 1) % pageCount;
		}
	} else {
		SetPpuMemoryMapping(startAddr, endAddr, type, pageNumber * pageSize, accessType == -1 ? defaultAccessType : accessType);
	}
}

void BaseMapper::SetPpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, ChrMemoryType type, uint32_t sourceOffset, int8_t accessType)
{
	uint8_t* sourceMemory = nullptr;

	if(type == ChrMemoryType::Default) {
		type = _chrRomSize > 0 ? ChrMemoryType::ChrRom : ChrMemoryType::ChrRam;
	}

	uint32_t sourceSize = 0;
	switch(type) {
		case ChrMemoryType::Default: 
		case ChrMemoryType::ChrRom:
			sourceMemory = _chrRom;
			sourceSize = _chrRomSize;
			break;

		case ChrMemoryType::ChrRam:
			sourceMemory = _chrRam;
			sourceSize = _chrRamSize;
			break;

		case ChrMemoryType::NametableRam:
			sourceMemory = _nametableRam;
			sourceSize = _ntRamSize;
			break;

		case ChrMemoryType::MapperRam:
			sourceMemory = _mapperRam;
			sourceSize = _mapperRamSize;
			break;
	}

	int firstSlot = startAddr >> 8;
	int slotCount = (endAddr - startAddr + 1) >> 8;
	for(int i = 0; i < slotCount; i++) {
		if(sourceSize == 0) {
			_chrPages[i] = nullptr;
			_chrMemoryAccess[i] = MemoryAccessType::NoAccess;
		} else {
			while(sourceOffset >= sourceSize) {
				sourceOffset -= sourceSize;
			}
			_chrMemoryOffset[firstSlot + i] = sourceOffset + i * 256;
			_chrMemoryType[firstSlot + i] = type;
			_chrMemoryAccess[firstSlot + i] = (MemoryAccessType)accessType;
		}
	}

	SetPpuMemoryMapping(startAddr, endAddr, sourceMemory, sourceOffset, sourceSize, accessType);
}

void BaseMapper::SetPpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, uint8_t* sourceMemory, uint32_t sourceOffset, uint32_t sourceSize, int8_t accessType)
{
	if(!ValidateAddressRange(startAddr, endAddr)) {
		return;
	}

	startAddr >>= 8;
	endAddr >>= 8;
	for(uint16_t i = startAddr; i <= endAddr; i++) {
		if(sourceMemory && sourceSize > 0 && sourceOffset <= sourceSize - 0x100) {
			_chrPages[i] = sourceMemory + sourceOffset;
			_chrMemoryAccess[i] = accessType != -1 ? (MemoryAccessType)accessType : MemoryAccessType::ReadWrite;
		} else {
			_chrPages[i] = nullptr;
			_chrMemoryAccess[i] = MemoryAccessType::NoAccess;
		}

		sourceOffset += 0x100;
	}
}

void BaseMapper::RemovePpuMemoryMapping(uint16_t startAddr, uint16_t endAddr)
{
	//Unmap this section of memory (causing open bus behavior)
	int firstSlot = startAddr >> 8;
	int slotCount = (endAddr - startAddr + 1) >> 8;
	for(int i = 0; i < slotCount; i++) {
		_chrMemoryOffset[firstSlot + i] = -1;
		_chrMemoryType[firstSlot + i] = ChrMemoryType::Default;
		_chrMemoryAccess[firstSlot + i] = MemoryAccessType::NoAccess;
	}

	SetPpuMemoryMapping(startAddr, endAddr, nullptr, 0, 0, MemoryAccessType::NoAccess);
}

uint8_t BaseMapper::InternalReadRam(uint16_t addr)
{
	return _prgPages[addr >> 8] ? _prgPages[addr >> 8][(uint8_t)addr] : 0;
}

void BaseMapper::SelectPrgPage4x(uint16_t slot, uint16_t page, PrgMemoryType memoryType)
{
	BaseMapper::SelectPrgPage2x(slot*2, page, memoryType);
	BaseMapper::SelectPrgPage2x(slot*2+1, page+2, memoryType);
}

void BaseMapper::SelectPrgPage2x(uint16_t slot, uint16_t page, PrgMemoryType memoryType)
{
	BaseMapper::SelectPrgPage(slot*2, page, memoryType);
	BaseMapper::SelectPrgPage(slot*2+1, page+1, memoryType);
}

void BaseMapper::SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType)
{
	if(_prgSize < 0x8000 && GetPrgPageSize() > _prgSize) {
		//Total PRG size is smaller than available memory range, map the entire PRG to all slots
		//i.e same logic as NROM (mapper 0) when PRG is 16kb
		//Needed by "Pyramid" (mapper 79)
		#ifdef _DEBUG
			MessageManager::DisplayMessage("Debug", "PrgSizeWarning");
		#endif

		for(slot = 0; slot < 0x8000 / _prgSize; slot++) {
			uint16_t startAddr = 0x8000 + slot * _prgSize;
			uint16_t endAddr = startAddr + _prgSize - 1;
			SetCpuMemoryMapping(startAddr, endAddr, 0, memoryType);
		}
	} else {
		uint16_t startAddr = 0x8000 + slot * InternalGetPrgPageSize();
		uint16_t endAddr = startAddr + InternalGetPrgPageSize() - 1;
		SetCpuMemoryMapping(startAddr, endAddr, page, memoryType);
	}
}

void BaseMapper::SelectChrPage8x(uint16_t slot, uint16_t page, ChrMemoryType memoryType)
{
	BaseMapper::SelectChrPage4x(slot, page, memoryType);
	BaseMapper::SelectChrPage4x(slot*2+1, page+4, memoryType);
}

void BaseMapper::SelectChrPage4x(uint16_t slot, uint16_t page, ChrMemoryType memoryType)
{
	BaseMapper::SelectChrPage2x(slot*2, page, memoryType);
	BaseMapper::SelectChrPage2x(slot*2+1, page+2, memoryType);
}

void BaseMapper::SelectChrPage2x(uint16_t slot, uint16_t page, ChrMemoryType memoryType)
{
	BaseMapper::SelectChrPage(slot*2, page, memoryType);
	BaseMapper::SelectChrPage(slot*2+1, page+1, memoryType);
}

void BaseMapper::SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType)
{
	uint16_t pageSize;
	if(memoryType == ChrMemoryType::NametableRam) {
		pageSize = BaseMapper::NametableSize;
	} else {
		if(memoryType == ChrMemoryType::Default) {
			memoryType = _chrRomSize > 0 ? ChrMemoryType::ChrRom : ChrMemoryType::ChrRam;
		}
		pageSize = memoryType == ChrMemoryType::ChrRam ? InternalGetChrRamPageSize() : InternalGetChrRomPageSize();
	}

	uint16_t startAddr = slot * pageSize;
	uint16_t endAddr = startAddr + pageSize - 1;
	SetPpuMemoryMapping(startAddr, endAddr, page, memoryType);
}

uint8_t BaseMapper::GetPowerOnByte(uint8_t defaultValue)
{
	if(_console->GetNesConfig().RandomizeMapperPowerOnState) {
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<> dist(0, 255);
		return dist(mt);
	} else {
		return defaultValue;
	}
}

uint32_t BaseMapper::GetDipSwitches()
{
	uint32_t mask = (1 << GetDipSwitchCount()) - 1;
	return _emu->GetSettings()->GetGameConfig().DipSwitches & mask;
}
		
bool BaseMapper::HasBattery()
{
	return _romInfo.HasBattery;
}

void BaseMapper::LoadBattery()
{
	if(HasBattery() && _saveRamSize > 0) {
		_emu->GetBatteryManager()->LoadBattery(".sav", _saveRam, _saveRamSize);
	}

	if(_hasChrBattery && _chrRamSize > 0) {
		_emu->GetBatteryManager()->LoadBattery(".chr.sav", _chrRam, _chrRamSize);
	}
}

void BaseMapper::SaveBattery()
{
	if(HasBattery() && _saveRamSize > 0) {
		_emu->GetBatteryManager()->SaveBattery(".sav", _saveRam, _saveRamSize);
	}

	if(_hasChrBattery && _chrRamSize > 0) {
		_emu->GetBatteryManager()->SaveBattery(".chr.sav", _chrRam, _chrRamSize);
	}
}

uint32_t BaseMapper::GetPrgPageCount()
{
	uint16_t pageSize = InternalGetPrgPageSize();
	return pageSize ? (_prgSize / pageSize) : 0;
}

uint32_t BaseMapper::GetChrRomPageCount()
{
	uint16_t pageSize = InternalGetChrRomPageSize();
	return pageSize ? (_chrRomSize / pageSize) : 0;
}

string BaseMapper::GetBatteryFilename()
{
	return FolderUtilities::CombinePath(FolderUtilities::GetSaveFolder(), FolderUtilities::GetFilename(_romInfo.RomName, false) + ".sav");
}

void BaseMapper::InitializeChrRam(int32_t chrRamSize)
{
	uint32_t defaultRamSize = GetChrRamSize() ? GetChrRamSize() : 0x2000;
	_chrRamSize = chrRamSize >= 0 ? chrRamSize : defaultRamSize;
	if(_chrRamSize > 0) {
		_chrRam = new uint8_t[_chrRamSize];
		_emu->RegisterMemory(MemoryType::NesChrRam, _chrRam, _chrRamSize);
		_console->InitializeRam(_chrRam, _chrRamSize);
	}
}

bool BaseMapper::HasDefaultWorkRam()
{
	return _hasDefaultWorkRam;
}

void BaseMapper::SetupDefaultWorkRam()
{
	//Setup a default work/save ram in 0x6000-0x7FFF space
	if(HasBattery() && _saveRamSize > 0) {
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::SaveRam);
	} else if(_workRamSize > 0) {
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::WorkRam);
	}
}

bool BaseMapper::HasChrRam()
{
	return _chrRamSize > 0;
}

bool BaseMapper::HasChrRom()
{
	return _chrRomSize > 0;
}

void BaseMapper::AddRegisterRange(uint16_t startAddr, uint16_t endAddr, MemoryOperation operation)
{
	for(int i = startAddr; i <= endAddr; i++) {
		if((int)operation & (int)MemoryOperation::Read) {
			_isReadRegisterAddr[i] = true;
		}
		if((int)operation & (int)MemoryOperation::Write) {
			_isWriteRegisterAddr[i] = true;
		}
	}
}

void BaseMapper::RemoveRegisterRange(uint16_t startAddr, uint16_t endAddr, MemoryOperation operation)
{
	for(int i = startAddr; i <= endAddr; i++) {
		if((int)operation & (int)MemoryOperation::Read) {
			_isReadRegisterAddr[i] = false;
		}
		if((int)operation & (int)MemoryOperation::Write) {
			_isWriteRegisterAddr[i] = false;
		}
	}
}

void BaseMapper::Serialize(Serializer& s)
{
	SVArray(_chrRam, _chrRamSize);
	SVArray(_workRam, _workRamSize);
	SVArray(_saveRam, _saveRamSize);
	SVArray(_mapperRam, _mapperRamSize);
	SVArray(_nametableRam, _ntRamSize);

	SVArray(_prgMemoryOffset, 0x100);
	SVArray(_chrMemoryOffset, 0x40);
	SVArray(_prgMemoryType, 0x100);
	SVArray(_chrMemoryType, 0x40);
	SVArray(_prgMemoryAccess, 0x100);
	SVArray(_chrMemoryAccess, 0x40);

	SV(_mirroringType);

	if(_epsm) {
		SV(_epsm);
	}

	if(!s.IsSaving()) {
		RestorePrgChrState();
	}
}

void BaseMapper::RestorePrgChrState()
{
	for(uint16_t i = 0; i < 0x100; i++) {
		uint16_t startAddr = i << 8;
		if(_prgMemoryAccess[i] != MemoryAccessType::NoAccess) {
			SetCpuMemoryMapping(startAddr, startAddr + 0xFF, _prgMemoryType[i], _prgMemoryOffset[i], _prgMemoryAccess[i]);
		} else {
			RemoveCpuMemoryMapping(startAddr, startAddr + 0xFF);
		}
	}

	for(uint16_t i = 0; i < 0x40; i++) {
		uint16_t startAddr = i << 8;
		if(_chrMemoryAccess[i] != MemoryAccessType::NoAccess) {
			SetPpuMemoryMapping(startAddr, startAddr + 0xFF, _chrMemoryType[i], _chrMemoryOffset[i], _chrMemoryAccess[i]);
		} else {
			RemovePpuMemoryMapping(startAddr, startAddr + 0xFF);
		}
	}
}

void BaseMapper::Initialize(NesConsole* console, RomData& romData)
{
	_console = console;
	_emu = console->GetEmulator();

	_romInfo = romData.Info;
	_internalRamMask = GetInternalRamSize() - 1;

	_batteryFilename = GetBatteryFilename();

	if(romData.SaveRamSize == -1) {
		_saveRamSize = HasBattery() ? GetSaveRamSize() : 0;
		_hasDefaultWorkRam = _saveRamSize > 0;
	} else if(ForceSaveRamSize()) {
		_saveRamSize = GetSaveRamSize();
	} else {
		_saveRamSize = romData.SaveRamSize;
	}

	if(romData.WorkRamSize == -1) {
		_workRamSize = HasBattery() ? 0 : GetWorkRamSize();
		_hasDefaultWorkRam = _workRamSize > 0;
	} else if(ForceWorkRamSize()) {
		_workRamSize = GetWorkRamSize();
	} else {
		_workRamSize = romData.WorkRamSize;
	}

	_allowRegisterRead = AllowRegisterRead();
	_hasCpuClockHook = EnableCpuClockHook();
	_hasCustomReadVram = EnableCustomVramRead();
	_hasVramAddressHook = EnableVramAddressHook();

	memset(_isReadRegisterAddr, 0, sizeof(_isReadRegisterAddr));
	memset(_isWriteRegisterAddr, 0, sizeof(_isWriteRegisterAddr));
	AddRegisterRange(RegisterStartAddress(), RegisterEndAddress(), MemoryOperation::Any);

	_prgSize = (uint32_t)romData.PrgRom.size();
	_chrRomSize = (uint32_t)romData.ChrRom.size();
	_originalPrgRom = romData.PrgRom;
	_originalChrRom = romData.ChrRom;

	_prgRom = new uint8_t[_prgSize];
	_emu->RegisterMemory(MemoryType::NesPrgRom, _prgRom, _prgSize);

	_chrRom = new uint8_t[_chrRomSize];
	_emu->RegisterMemory(MemoryType::NesChrRom, _chrRom, _chrRomSize);

	memcpy(_prgRom, romData.PrgRom.data(), _prgSize);
	if(_chrRomSize > 0) {
		memcpy(_chrRom, romData.ChrRom.data(), _chrRomSize);
	}

	_hasChrBattery = romData.SaveChrRamSize > 0 || ForceChrBattery();

	switch(romData.Info.BusConflicts) {
		case BusConflictType::Default: _hasBusConflicts = HasBusConflicts(); break;
		case BusConflictType::Yes: _hasBusConflicts = true; break;
		case BusConflictType::No: _hasBusConflicts = false; break;
	}	

	if(_hasBusConflicts) {
		MessageManager::Log("[iNes] Bus conflicts enabled");
	}

	_saveRam = new uint8_t[_saveRamSize];

	_emu->RegisterMemory(MemoryType::NesSaveRam, _saveRam, _saveRamSize);
	
	_workRam = new uint8_t[_workRamSize];
	_emu->RegisterMemory(MemoryType::NesWorkRam, _workRam, _workRamSize);

	_mapperRamSize = GetMapperRamSize();
	_mapperRam = new uint8_t[_mapperRamSize];
	_emu->RegisterMemory(MemoryType::NesMapperRam, _mapperRam, _mapperRamSize);

	_console->InitializeRam(_saveRam, _saveRamSize);
	_console->InitializeRam(_workRam, _workRamSize);
	_console->InitializeRam(_mapperRam, _mapperRamSize);

	_nametableCount = GetNametableCount();
	if(_nametableCount == 0) {
		_nametableCount = romData.Info.Mirroring == MirroringType::FourScreens ? 4 : 2;
	}
	
	_ntRamSize = _nametableCount * BaseMapper::NametableSize;
	_nametableRam = new uint8_t[_ntRamSize];
	_console->InitializeRam(_nametableRam, _ntRamSize);
	_emu->RegisterMemory(MemoryType::NesNametableRam, _nametableRam, _ntRamSize);

	for(int i = 0; i < 0x100; i++) {
		//Allow us to map a different page every 256 bytes
		_prgPages[i] = nullptr;
		_prgMemoryOffset[i] = -1;
		_prgMemoryType[i] = PrgMemoryType::PrgRom;
		_prgMemoryAccess[i] = MemoryAccessType::NoAccess;

		_chrPages[i] = nullptr;
		_chrMemoryOffset[i] = -1;
		_chrMemoryType[i] = ChrMemoryType::Default;
		_chrMemoryAccess[i] = MemoryAccessType::NoAccess;
	}

	if(_chrRomSize == 0) {
		//Assume there is CHR RAM if no CHR ROM exists
		InitializeChrRam(romData.ChrRamSize);

		//Map CHR RAM to 0x0000-0x1FFF by default when no CHR ROM exists
		SetPpuMemoryMapping(0x0000, 0x1FFF, 0, ChrMemoryType::ChrRam);
	} else if(romData.ChrRamSize >= 0) {
		InitializeChrRam(romData.ChrRamSize);
	} else if(GetChrRamSize()) {
		InitializeChrRam();
	}

	if(romData.Info.HasTrainer) {
		if(_workRamSize >= 0x2000) {
			memcpy(_workRam + 0x1000, romData.TrainerData.data(), 512);
		} else if(_saveRamSize >= 0x2000) {
			memcpy(_saveRam + 0x1000, romData.TrainerData.data(), 512);
		}
	}

	SetupDefaultWorkRam();

	SetMirroringType(romData.Info.Mirroring);


	//Load battery data if present
	LoadBattery();

	_romInfo.HasChrRam = HasChrRam();

	if(_romInfo.HasEpsm) {
		vector<uint8_t> adpcmRom;
		FirmwareHelper::LoadYmf288AdpcmRom(_emu, adpcmRom);
		_epsm.reset(new Epsm(_emu, _console, adpcmRom));
		_hasCpuClockHook = true;
	}
}

void BaseMapper::InitSpecificMapper(RomData& romData)
{
	InitMapper();
	InitMapper(romData);
}

BaseMapper::BaseMapper()
{
}

BaseMapper::~BaseMapper()
{
	delete[] _chrRam;
	delete[] _chrRom;
	delete[] _prgRom;
	delete[] _saveRam;
	delete[] _workRam;
	delete[] _mapperRam;
	delete[] _nametableRam;
}

void BaseMapper::GetMemoryRanges(MemoryRanges &ranges)
{
	if(_romInfo.System == GameSystem::VsSystem) {
		ranges.AddHandler(MemoryOperation::Read, 0x6000, 0xFFFF);
		ranges.AddHandler(MemoryOperation::Write, 0x6000, 0xFFFF);
	} else {
		ranges.AddHandler(MemoryOperation::Read, 0x4020, 0xFFFF);
		ranges.AddHandler(MemoryOperation::Write, 0x4020, 0xFFFF);
	}
}

uint8_t* BaseMapper::GetNametable(uint8_t nametableIndex)
{
	if(nametableIndex >= _nametableCount) {
		#ifdef _DEBUG
		MessageManager::Log("Invalid nametable index");
		#endif
		return _nametableRam;
	}

	return _nametableRam + (nametableIndex * BaseMapper::NametableSize);
}

void BaseMapper::SetNametable(uint8_t index, uint8_t nametableIndex)
{
	if(nametableIndex >= _nametableCount) {
		#ifdef _DEBUG
		MessageManager::Log("Invalid nametable index");
		#endif
		return;
	}

	SetPpuMemoryMapping(0x2000 + index * 0x400, 0x2000 + (index + 1) * 0x400 - 1, nametableIndex, ChrMemoryType::NametableRam);
	
	//Mirror $2000-$2FFF to $3000-$3FFF, while keeping a distinction between the addresses
	//Previously, $3000-$3FFF was being "redirected" to $2000-$2FFF to avoid MMC3 IRQ issues (which is incorrect)
	//More info here: https://forums.nesdev.com/viewtopic.php?p=132145#p132145
	SetPpuMemoryMapping(0x3000 + index * 0x400, 0x3000 + (index + 1) * 0x400 - 1, nametableIndex, ChrMemoryType::NametableRam);
}

void BaseMapper::SetNametables(uint8_t nametable1Index, uint8_t nametable2Index, uint8_t nametable3Index, uint8_t nametable4Index)
{
	SetNametable(0, nametable1Index);
	SetNametable(1, nametable2Index);
	SetNametable(2, nametable3Index);
	SetNametable(3, nametable4Index);
}

void BaseMapper::SetMirroringType(MirroringType type)
{
	_mirroringType = type;
	switch(type) {
		case MirroringType::Vertical: SetNametables(0, 1, 0, 1); break;
		case MirroringType::Horizontal: SetNametables(0, 0, 1, 1); break;
		case MirroringType::FourScreens:	SetNametables(0, 1, 2, 3); break;
		case MirroringType::ScreenAOnly: SetNametables(0, 0, 0, 0);	break;
		case MirroringType::ScreenBOnly: SetNametables(1, 1, 1, 1);	break;
	}
}

GameSystem BaseMapper::GetGameSystem()
{
	return _romInfo.System;
}

PpuModel BaseMapper::GetPpuModel()
{
	return _romInfo.VsPpuModel;
}

NesRomInfo BaseMapper::GetRomInfo()
{
	NesRomInfo romInfo = _romInfo;
	romInfo.BusConflicts = _hasBusConflicts ? BusConflictType::Yes : BusConflictType::No;
	return romInfo;
}

uint32_t BaseMapper::GetMapperDipSwitchCount()
{
	return GetDipSwitchCount();
}

MirroringType BaseMapper::GetMirroringType()
{
	return _mirroringType;
}
	
uint8_t BaseMapper::ReadRam(uint16_t addr)
{
	if(_allowRegisterRead && _isReadRegisterAddr[addr]) {
		return ReadRegister(addr);
	} else if(_prgMemoryAccess[addr >> 8] & MemoryAccessType::Read) {
		return _prgPages[addr >> 8][(uint8_t)addr];
	} else {
		//assert(false);
	}
	return _console->GetMemoryManager()->GetOpenBus();
}

uint8_t BaseMapper::PeekRam(uint16_t addr)
{
	return DebugReadRam(addr);
}

uint8_t BaseMapper::DebugReadRam(uint16_t addr)
{
	if(_prgMemoryAccess[addr >> 8] & MemoryAccessType::Read) {
		uint8_t* page = _prgPages[addr >> 8];
		return page[(uint8_t)addr];
	} else {
		//assert(false);
	}
	
	//Fake open bus
	return addr >> 8;
}

void BaseMapper::WriteRam(uint16_t addr, uint8_t value)
{
	if(_isWriteRegisterAddr[addr]) {
		if(_hasBusConflicts) {
			uint8_t prgValue = _prgPages[addr >> 8][(uint8_t)addr];
			if(value != prgValue) {
				_emu->BreakIfDebugging(CpuType::Nes, BreakSource::NesBusConflict);
			}
			value &= prgValue;
		}
		WriteRegister(addr, value);
	} else {
		WritePrgRam(addr, value);
	}
}

void BaseMapper::DebugWriteRam(uint16_t addr, uint8_t value)
{
	if(_isWriteRegisterAddr[addr]) {
		//not supported
	} else {
		if(_prgMemoryAccess[addr >> 8] & MemoryAccessType::Write) {
			uint8_t* page = _prgPages[addr >> 8];
			page[(uint8_t)addr] = value;
		}
	}
}

void BaseMapper::WritePrgRam(uint16_t addr, uint8_t value)
{
	if(_prgMemoryAccess[addr >> 8] & MemoryAccessType::Write) {
		_prgPages[addr >> 8][(uint8_t)addr] = value;
	}
}

void BaseMapper::SetRegion(ConsoleRegion region)
{
	if(_epsm) {
		_epsm->OnRegionChanged();
	}
}

void BaseMapper::BaseProcessCpuClock()
{
	if(_epsm) {
		_epsm->Exec();
	}
}

void BaseMapper::ProcessCpuClock()
{
	BaseProcessCpuClock();
}

void BaseMapper::NotifyVramAddressChange(uint16_t addr)
{
	//This is called when the VRAM addr on the PPU memory bus changes
	//Used by MMC3/MMC5/etc
}

uint8_t BaseMapper::DebugReadVram(uint16_t addr, bool disableSideEffects)
{
	addr &= 0x3FFF;
	if(!disableSideEffects) {
		NotifyVramAddressChange(addr);
	}
	return InternalReadVram(addr);
}

uint8_t BaseMapper::MapperReadVram(uint16_t addr, MemoryOperationType operationType)
{
	return InternalReadVram(addr);
}

void BaseMapper::DebugWriteVram(uint16_t addr, uint8_t value, bool disableSideEffects)
{
	addr &= 0x3FFF;
	if(disableSideEffects) {
		if(_chrPages[addr >> 8]) {
			//Always allow writes when side-effects are disabled
			_chrPages[addr >> 8][(uint8_t)addr] = value;
		}
	} else {
		NotifyVramAddressChange(addr);
		if(_chrMemoryAccess[addr >> 8] & MemoryAccessType::Write) {
			_chrPages[addr >> 8][(uint8_t)addr] = value;
		}
	}
}

void BaseMapper::WriteVram(uint16_t addr, uint8_t value)
{
	_emu->ProcessPpuWrite<CpuType::Nes>(addr, value, MemoryType::NesPpuMemory);
	MapperWriteVram(addr, value);
}

void BaseMapper::MapperWriteVram(uint16_t addr, uint8_t value)
{
	InternalWriteVram(addr, value);
}

void BaseMapper::InternalWriteVram(uint16_t addr, uint8_t value)
{
	if(_chrMemoryAccess[addr >> 8] & MemoryAccessType::Write) {
		_chrPages[addr >> 8][(uint8_t)addr] = value;
	}
}

bool BaseMapper::IsNes20()
{
	return _romInfo.Header.GetRomHeaderVersion() == RomHeaderVersion::Nes2_0;
}

//Debugger Helper Functions
void BaseMapper::CopyChrTile(uint32_t address, uint8_t *dest)
{
	if(_chrRamSize > 0 && address <= _chrRamSize - 16) {
		memcpy(dest, _chrRam + address, 16);
	} else if(_chrRomSize > 0 && address <= _chrRomSize - 16) {
		memcpy(dest, _chrRom + address, 16);
	}
}

AddressInfo BaseMapper::GetAbsoluteAddress(uint16_t relativeAddr)
{
	AddressInfo info;
	if(relativeAddr < 0x2000) {
		info.Address = relativeAddr & _internalRamMask;
		info.Type = MemoryType::NesInternalRam;
	} else {
		uint8_t *addr = _prgPages[relativeAddr >> 8] + (uint8_t)relativeAddr;
		if(addr >= _prgRom && addr < _prgRom + _prgSize) {
			info.Address = (uint32_t)(addr - _prgRom);
			info.Type = MemoryType::NesPrgRom;
		} else if(addr >= _workRam && addr < _workRam + _workRamSize) {
			info.Address = (uint32_t)(addr - _workRam);
			info.Type = MemoryType::NesWorkRam;
		} else if(addr >= _saveRam && addr < _saveRam + _saveRamSize) {
			info.Address = (uint32_t)(addr - _saveRam);
			info.Type = MemoryType::NesSaveRam;
		} else if(addr >= _mapperRam && addr < _mapperRam + _mapperRamSize) {
			info.Address = (uint32_t)(addr - _mapperRam);
			info.Type = MemoryType::NesMapperRam;
		} else {
			info.Address = -1;
			info.Type = MemoryType::None;
		}
	}
	return info;
}

void BaseMapper::GetPpuAbsoluteAddress(uint16_t relativeAddr, AddressInfo& info)
{
	if(relativeAddr >= 0x3F00) {
		info.Address = relativeAddr & 0x1F;
		info.Type = MemoryType::NesPaletteRam;
	} else {
		uint8_t* addr = _chrPages[relativeAddr >> 8] + (uint8_t)relativeAddr;
		if(addr >= _chrRom && addr < _chrRom + _chrRomSize) {
			info.Address = (uint32_t)(addr - _chrRom);
			info.Type = MemoryType::NesChrRom;
		} else if(addr >= _chrRam && addr < _chrRam + _chrRamSize) {
			info.Address = (uint32_t)(addr - _chrRam);
			info.Type = MemoryType::NesChrRam;
		} else if(addr >= _mapperRam && addr < _mapperRam + _mapperRamSize) {
			info.Address = (uint32_t)(addr - _mapperRam);
			info.Type = MemoryType::NesMapperRam;
		} else if(addr >= _nametableRam && addr < _nametableRam + _nametableCount * BaseMapper::NametableSize) {
			info.Address = (uint32_t)(addr - _nametableRam);
			info.Type = MemoryType::NesNametableRam;
		} else {
			info.Address = -1;
			info.Type = MemoryType::None;
		}
	}
}

AddressInfo BaseMapper::GetPpuAbsoluteAddress(uint32_t relativeAddr)
{
	AddressInfo info;
	GetPpuAbsoluteAddress(relativeAddr, info);
	return info;
}

AddressInfo BaseMapper::GetRelativeAddress(AddressInfo& addr)
{
	uint8_t* ptrAddress;

	switch(addr.Type) {
		case MemoryType::NesPrgRom: ptrAddress = _prgRom; break;
		case MemoryType::NesWorkRam: ptrAddress = _workRam; break;
		case MemoryType::NesSaveRam: ptrAddress = _saveRam; break;
		case MemoryType::NesMapperRam: ptrAddress = _mapperRam; break;
		case MemoryType::NesInternalRam: return { (int32_t)(addr.Address & _internalRamMask), MemoryType::NesMemory };
		default: return { GetPpuRelativeAddress(addr), MemoryType::NesPpuMemory };
	}
	ptrAddress += addr.Address;

	for(int i = 0; i < 256; i++) {
		uint8_t* pageAddress = _prgPages[i];
		if(pageAddress != nullptr && ptrAddress >= pageAddress && ptrAddress <= pageAddress + 0xFF) {
			return { (int)((i << 8) + (uint32_t)(ptrAddress - pageAddress)), MemoryType::NesMemory };
		}
	}

	//Address is currently not mapped
	return { -1, MemoryType::None };
}

int32_t BaseMapper::GetPpuRelativeAddress(AddressInfo& addr)
{
	uint8_t* ptrAddress;

	switch(addr.Type) {
		case MemoryType::NesChrRom: ptrAddress = _chrRom; break;
		case MemoryType::NesChrRam: ptrAddress = _chrRam; break;
		case MemoryType::NesNametableRam: ptrAddress = _nametableRam; break;
		case MemoryType::NesMapperRam: ptrAddress = _mapperRam; break;
		case MemoryType::NesPaletteRam: return 0x3F00 | (addr.Address & 0x1F); break;
		default: return -1;
	}
	ptrAddress += addr.Address;

	for(int i = 0; i < 0x40; i++) {
		uint8_t* pageAddress = _chrPages[i];
		if(pageAddress != nullptr && ptrAddress >= pageAddress && ptrAddress <= pageAddress + 0xFF) {
			return (i << 8) + (uint32_t)(ptrAddress - pageAddress);
		}
	}

	//Address is currently not mapped
	return -1;
}

bool BaseMapper::IsWriteRegister(uint16_t addr)
{
	return _isWriteRegisterAddr[addr];
}

bool BaseMapper::IsReadRegister(uint16_t addr)
{
	return _allowRegisterRead && _isReadRegisterAddr[addr];
}

CartridgeState BaseMapper::GetState()
{
	CartridgeState state;

	state.Mirroring = _mirroringType;
	state.HasBattery = _romInfo.HasBattery;

	state.PrgRomSize = _prgSize;
	state.ChrRomSize = _chrRomSize;
	state.ChrRamSize = _chrRamSize;

	state.PrgPageCount = GetPrgPageCount();
	state.PrgPageSize = InternalGetPrgPageSize();
	state.ChrPageCount = GetChrRomPageCount();
	state.ChrPageSize = InternalGetChrRomPageSize();
	state.ChrRamPageSize = InternalGetChrRamPageSize();
	for(int i = 0; i < 0x100; i++) {
		state.PrgMemoryOffset[i] = _prgMemoryOffset[i];
		state.PrgType[i] = _prgMemoryType[i];
		state.PrgMemoryAccess[i] = _prgMemoryAccess[i];
	}
	for(int i = 0; i < 0x40; i++) {
		state.ChrMemoryOffset[i] = _chrMemoryOffset[i];
		state.ChrType[i] = _chrMemoryType[i];
		state.ChrMemoryAccess[i] = _chrMemoryAccess[i];
	}

	state.WorkRamPageSize = GetWorkRamPageSize();
	state.SaveRamPageSize = GetSaveRamPageSize();

	vector<MapperStateEntry> entries = GetMapperStateEntries();
	assert(entries.size() <= 200);
	state.CustomEntryCount = (uint32_t)entries.size();
	for(int i = 0; i < entries.size(); i++) {
		state.CustomEntries[i] = entries[i];
	}
	
	return state;
}

void BaseMapper::GetRomFileData(vector<uint8_t> &out, bool asIpsFile, uint8_t* header)
{
	if(header) {
		//Get original rom with edited header
		vector<uint8_t> originalFile;
		_emu->GetRomInfo().RomFile.ReadFile(originalFile);

		out.insert(out.end(), header, header+sizeof(NesHeader));
		if(_romInfo.IsHeaderlessRom) {
			out.insert(out.end(), originalFile.begin(), originalFile.end());
		} else {
			out.insert(out.end(), originalFile.begin() + sizeof(NesHeader), originalFile.end());
		}
	} else {
		vector<uint8_t> newFile;
		newFile.insert(newFile.end(), (uint8_t*)&_romInfo.Header, ((uint8_t*)&_romInfo.Header) + sizeof(NesHeader));
		newFile.insert(newFile.end(), _prgRom, _prgRom + _prgSize);
		if(HasChrRom()) {
			newFile.insert(newFile.end(), _chrRom, _chrRom + _chrRomSize);
		}
		
		//Get edited rom
		if(asIpsFile) {
			vector<uint8_t> originalFile;
			_emu->GetRomInfo().RomFile.ReadFile(originalFile);

			vector<uint8_t> patchData = IpsPatcher::CreatePatch(originalFile, newFile);
			out.insert(out.end(), patchData.begin(), patchData.end());
		} else {
			out.insert(out.end(), newFile.begin(), newFile.end());
		}
	}
}

vector<uint8_t> BaseMapper::GetPrgChrCopy()
{
	vector<uint8_t> data;
	data.resize(_prgSize + _chrRomSize);
	memcpy(data.data(), _prgRom, _prgSize);
	memcpy(data.data() + _prgSize, _chrRom, _chrRomSize);
	return data;
}

void BaseMapper::RestorePrgChrBackup(vector<uint8_t> &backupData)
{
	memcpy(_prgRom, backupData.data(), _prgSize);
	memcpy(_chrRom, backupData.data() + _prgSize, _chrRomSize);
}

void BaseMapper::RevertPrgChrChanges()
{
	memcpy(_prgRom, _originalPrgRom.data(), _originalPrgRom.size());
	if(_chrRom) {
		memcpy(_chrRom, _originalChrRom.data(), _originalChrRom.size());
	}
}

bool BaseMapper::HasPrgChrChanges()
{
	if(memcmp(_prgRom, _originalPrgRom.data(), _originalPrgRom.size()) != 0) {
		return true;
	}
	if(_chrRom) {
		if(memcmp(_chrRom, _originalChrRom.data(), _originalChrRom.size()) != 0) {
			return true;
		}
	}
	return false;
}

void BaseMapper::CopyPrgChrRom(BaseMapper* mapper)
{
	if(_prgSize == mapper->_prgSize && _chrRomSize == mapper->_chrRomSize) {
		memcpy(_prgRom, mapper->_prgRom, _prgSize);
		memcpy(_chrRom, mapper->_chrRom, _chrRomSize);
	}
}

void BaseMapper::SwapMemoryAccess(BaseMapper* sub, bool mainHasAccess)
{
	//Used by VS Dualsystem to share memory access between 2 consoles
	if(_saveRamSize == 0 && _workRamSize == 0) {
		return;
	}

	uint8_t* memory = HasBattery() ? _saveRam : _workRam;
	uint32_t size = HasBattery() ? _saveRamSize : _workRamSize;
	SetCpuMemoryMapping(0x6000, 0x7FFF, memory, 0, size, mainHasAccess ? MemoryAccessType::ReadWrite : MemoryAccessType::NoAccess);
	sub->SetCpuMemoryMapping(0x6000, 0x7FFF, memory, 0, size, mainHasAccess ? MemoryAccessType::NoAccess : MemoryAccessType::ReadWrite);
}
