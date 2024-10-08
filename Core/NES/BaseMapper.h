#pragma once

#include "pch.h"
#include "NES/INesMemoryHandler.h"
#include "NES/NesTypes.h"
#include "NES/RomData.h"
#include "Debugger/DebugTypes.h"
#include "Shared/Emulator.h"
#include "Shared/MemoryOperationType.h"
#include "Utilities/ISerializable.h"

class NesConsole;
class Epsm;
enum class MemoryType;
struct MapperStateEntry;

class BaseMapper : public INesMemoryHandler, public ISerializable
{
private:
	unique_ptr<Epsm> _epsm;

	MirroringType _mirroringType = {};
	string _batteryFilename;

	uint16_t InternalGetPrgPageSize();
	uint16_t InternalGetSaveRamPageSize();
	uint16_t InternalGetWorkRamPageSize();
	uint16_t InternalGetChrRomPageSize();
	uint16_t InternalGetChrRamPageSize();
	bool ValidateAddressRange(uint16_t startAddr, uint16_t endAddr);

	uint8_t *_nametableRam = nullptr;
	uint8_t _nametableCount = 2;
	uint32_t _ntRamSize = 0;

	uint32_t _internalRamMask = 0x7FF;

	bool _hasBusConflicts = false;
	bool _hasDefaultWorkRam = false;
	
	bool _hasCustomReadVram = false;
	bool _hasCpuClockHook = false;
	bool _hasVramAddressHook = false;

	bool _allowRegisterRead = false;
	bool _isReadRegisterAddr[0x10000] = {};
	bool _isWriteRegisterAddr[0x10000] = {};

	MemoryAccessType _prgMemoryAccess[0x100] = {};
	uint8_t* _prgPages[0x100] = {};

	MemoryAccessType _chrMemoryAccess[0x100] = {};
	uint8_t* _chrPages[0x100] = {};

	int32_t _prgMemoryOffset[0x100] = {};
	PrgMemoryType _prgMemoryType[0x100] = {};

	int32_t _chrMemoryOffset[0x100] = {};
	ChrMemoryType _chrMemoryType[0x100] = {};

	vector<uint8_t> _originalPrgRom;
	vector<uint8_t> _originalChrRom;

protected:
	NesRomInfo _romInfo = {};

	NesConsole* _console = nullptr;
	Emulator* _emu = nullptr;

	uint8_t* _prgRom = nullptr;
	uint8_t* _chrRom = nullptr;
	uint8_t* _chrRam = nullptr;
	uint32_t _prgSize = 0;
	uint32_t _chrRomSize = 0;
	uint32_t _chrRamSize = 0;

	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;
	uint32_t _workRamSize = 0;
	uint8_t* _workRam = nullptr;
	bool _hasChrBattery = false;
	int16_t _vramOpenBusValue = -1;

	uint8_t* _mapperRam = nullptr;
	uint32_t _mapperRamSize = 0;

	virtual void InitMapper() = 0;
	virtual void InitMapper(RomData &romData);
	virtual uint16_t GetPrgPageSize() = 0;
	virtual uint16_t GetChrPageSize() = 0;

	bool IsNes20();

	virtual uint16_t GetChrRamPageSize() { return GetChrPageSize(); }

	//Save ram is battery backed and saved to disk
	virtual uint32_t GetSaveRamSize() { return 0x2000; }
	virtual uint32_t GetSaveRamPageSize() { return 0x2000; }
	virtual bool ForceChrBattery() { return false; }
	
	virtual bool ForceSaveRamSize() { return false; }
	virtual bool ForceWorkRamSize() { return false; }

	virtual uint32_t GetChrRamSize() { return 0x0000; }

	//Work ram is NOT saved - aka Expansion ram, etc.
	virtual uint32_t GetWorkRamSize() { return 0x2000; }
	virtual uint32_t GetWorkRamPageSize() { return 0x2000; }
	
	virtual uint32_t GetMapperRamSize() { return 0; }

	virtual uint16_t RegisterStartAddress() { return 0x8000; }
	virtual uint16_t RegisterEndAddress() { return 0xFFFF; }
	virtual bool AllowRegisterRead() { return false; }

	virtual bool EnableCpuClockHook() { return false; }
	virtual bool EnableCustomVramRead() { return false; }
	virtual bool EnableVramAddressHook() { return false; }

	virtual uint32_t GetDipSwitchCount() { return 0; }
	virtual uint32_t GetNametableCount() { return 0; }
	
	virtual bool HasBusConflicts() { return false; }

	uint8_t InternalReadRam(uint16_t addr);

	virtual void WriteRegister(uint16_t addr, uint8_t value);
	virtual uint8_t ReadRegister(uint16_t addr);

	void SelectPrgPage4x(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom);
	void SelectPrgPage2x(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom);
	virtual void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom);
	void SetCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, int16_t pageNumber, PrgMemoryType type, int8_t accessType = -1);
	void SetCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, PrgMemoryType type, uint32_t sourceOffset, int8_t accessType);
	void SetCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, uint8_t *source, uint32_t sourceOffset, uint32_t sourceSize, int8_t accessType = -1);
	void RemoveCpuMemoryMapping(uint16_t startAddr, uint16_t endAddr);

	virtual void SelectChrPage8x(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default);
	virtual void SelectChrPage4x(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default);
	virtual void SelectChrPage2x(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default);
	virtual void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default);
	void SetPpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, uint16_t pageNumber, ChrMemoryType type = ChrMemoryType::Default, int8_t accessType = -1);
	void SetPpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, ChrMemoryType type, uint32_t sourceOffset, int8_t accessType);
	void SetPpuMemoryMapping(uint16_t startAddr, uint16_t endAddr, uint8_t* sourceMemory, uint32_t sourceOffset, uint32_t sourceSize, int8_t accessType = -1);
	void RemovePpuMemoryMapping(uint16_t startAddr, uint16_t endAddr);

	bool HasBattery();
	virtual void LoadBattery();
	string GetBatteryFilename();

	uint32_t GetPrgPageCount();
	uint32_t GetChrRomPageCount();

	uint8_t GetPowerOnByte(uint8_t defaultValue = 0);
	uint32_t GetDipSwitches();

	void SetupDefaultWorkRam();

	void InitializeChrRam(int32_t chrRamSize = -1);

	void AddRegisterRange(uint16_t startAddr, uint16_t endAddr, MemoryOperation operation = MemoryOperation::Any);
	void RemoveRegisterRange(uint16_t startAddr, uint16_t endAddr, MemoryOperation operation = MemoryOperation::Any);

	void Serialize(Serializer& s) override;

	void RestorePrgChrState();

	void BaseProcessCpuClock();

	uint8_t* GetNametable(uint8_t nametableIndex);
	void SetNametable(uint8_t index, uint8_t nametableIndex);
	void SetNametables(uint8_t nametable1Index, uint8_t nametable2Index, uint8_t nametable3Index, uint8_t nametable4Index);
	void SetMirroringType(MirroringType type);
	MirroringType GetMirroringType();

	void InternalWriteVram(uint16_t addr, uint8_t value);

	__forceinline uint8_t InternalReadVram(uint16_t addr)
	{
		if(_chrMemoryAccess[addr >> 8] & MemoryAccessType::Read) {
			return _chrPages[addr >> 8][(uint8_t)addr];
		}

		//Open bus - "When CHR is disabled, the pattern tables are open bus. Theoretically, this should return the LSB of the address read, but real-world behavior varies."
		return _vramOpenBusValue >= 0 ? _vramOpenBusValue : addr;
	}

	virtual vector<MapperStateEntry> GetMapperStateEntries() { return {}; }

public:
	static constexpr uint32_t NametableSize = 0x400;
	
	void Initialize(NesConsole* console, RomData &romData);
	void InitSpecificMapper(RomData& romData);

	BaseMapper();
	virtual ~BaseMapper();
	virtual void Reset(bool softReset);
	virtual void OnAfterResetPowerOn() {}

	GameSystem GetGameSystem();
	PpuModel GetPpuModel();
	
	Epsm* GetEpsm() { return _epsm.get(); }
	
	bool HasDefaultWorkRam();

	void SetRegion(ConsoleRegion region);

	__forceinline bool HasCpuClockHook() { return _hasCpuClockHook; }
	virtual void ProcessCpuClock();
	
	__forceinline bool HasVramAddressHook() { return _hasVramAddressHook; }
	virtual void NotifyVramAddressChange(uint16_t addr);

	virtual void GetMemoryRanges(MemoryRanges &ranges) override;
	virtual uint32_t GetInternalRamSize() { return 0x800; }

	virtual void SaveBattery();

	NesRomInfo GetRomInfo();
	uint32_t GetMapperDipSwitchCount();

	uint8_t ReadRam(uint16_t addr) override;
	uint8_t PeekRam(uint16_t addr) override;
	uint8_t DebugReadRam(uint16_t addr);
	void WriteRam(uint16_t addr, uint8_t value) override;
	void DebugWriteRam(uint16_t addr, uint8_t value);
	void WritePrgRam(uint16_t addr, uint8_t value);

	virtual uint8_t MapperReadVram(uint16_t addr, MemoryOperationType operationType);
	virtual void MapperWriteVram(uint16_t addr, uint8_t value);

	__forceinline uint8_t ReadVram(uint16_t addr, MemoryOperationType type = MemoryOperationType::PpuRenderingRead)
	{
		uint8_t value;
		if(!_hasCustomReadVram) {
			value = InternalReadVram(addr);
		} else {
			value = MapperReadVram(addr, type);
		}
		_emu->ProcessPpuRead<CpuType::Nes>(addr, value, MemoryType::NesPpuMemory, type);
		return value;
	}

	void DebugWriteVram(uint16_t addr, uint8_t value, bool disableSideEffects = true);
	void WriteVram(uint16_t addr, uint8_t value);

	uint8_t DebugReadVram(uint16_t addr, bool disableSideEffects = true);

	void CopyChrTile(uint32_t address, uint8_t *dest);

	//Debugger Helper Functions
	bool HasChrRam();
	bool HasChrRom();
	uint32_t GetChrRomSize() { return _chrRomSize; }

	CartridgeState GetState();
	
	AddressInfo GetAbsoluteAddress(uint16_t relativeAddr);
	void GetPpuAbsoluteAddress(uint16_t relativeAddr, AddressInfo& info);
	AddressInfo GetPpuAbsoluteAddress(uint32_t relativeAddr);
	AddressInfo GetRelativeAddress(AddressInfo& addr);
	int32_t GetPpuRelativeAddress(AddressInfo& addr);

	bool IsWriteRegister(uint16_t addr);
	bool IsReadRegister(uint16_t addr);

	void GetRomFileData(vector<uint8_t> &out, bool asIpsFile, uint8_t* header);

	vector<uint8_t> GetPrgChrCopy();
	void RestorePrgChrBackup(vector<uint8_t>& backupData);
	void RevertPrgChrChanges();
	bool HasPrgChrChanges();
	void CopyPrgChrRom(BaseMapper* mapper);
	void SwapMemoryAccess(BaseMapper* sub, bool mainHasAccess);
};