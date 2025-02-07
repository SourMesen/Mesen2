#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Loaders/FdsLoader.h"
#include "NES/Mappers/FDS/FdsInputButtons.h"

class FdsAudio;
class NesCpu;
class NesMemoryManager;

class Fds : public BaseMapper
{
private:
	static constexpr uint32_t NoDiskInserted = 0xFF;
	bool _disableAutoInsertDisk = false;

	unique_ptr<FdsAudio> _audio;
	shared_ptr<FdsInputButtons> _input;
	
	NesConfig* _settings = nullptr;
	NesCpu* _cpu = nullptr;
	NesMemoryManager* _memoryManager = nullptr;

	//Write registers
	uint16_t _irqReloadValue = 0;
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;
	bool _irqRepeatEnabled = false;

	bool _diskRegEnabled = true;
	bool _soundRegEnabled = true;

	uint8_t _writeDataReg = 0;
	
	bool _motorOn = false;
	bool _resetTransfer = false;
	bool _readMode = false;
	bool _crcControl = false;
	bool _diskReady = false;
	bool _diskIrqEnabled = false;

	int32_t _autoDiskEjectCounter = -1;
	int32_t _autoDiskSwitchCounter = -1;
	int32_t _restartAutoInsertCounter = -1;
	uint32_t _previousFrame = 0;
	int32_t _lastDiskCheckFrame = 0;
	int32_t _successiveChecks = 0;
	uint32_t _previousDiskNumber = Fds::NoDiskInserted;

	uint8_t _extConWriteReg = 0;

	//Read registers
	bool _badCrc = false;
	bool _endOfHead = false;
	bool _readWriteEnabled = false;

	uint8_t _readDataReg = 0;

	bool _diskWriteProtected = false;

	//Internal values
	uint32_t _diskNumber = Fds::NoDiskInserted;
	uint32_t _diskPosition = 0;
	uint32_t _delay = 0;	
	uint16_t _crcAccumulator = 0;
	bool _previousCrcControlFlag = false;
	bool _gapEnded = true;
	bool _scanningDisk = false;
	bool _transferComplete = false;
	bool _useQdFormat = false;
	
	vector<uint8_t> _fdsRawData;
	vector<vector<uint8_t>> _fdsDiskSides;
	vector<vector<uint8_t>> _fdsDiskHeaders;
	string _romFilepath;

	vector<vector<uint8_t>> _orgDiskSides;
	vector<vector<uint8_t>> _orgDiskHeaders;

	bool _gameStarted = false;
	bool _needSave = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x8000; }
	uint32_t GetWorkRamSize() override { return 0x8000; }
	uint16_t RegisterStartAddress() override { return 0x4020; }
	uint16_t RegisterEndAddress() override { return 0x4092; }
	bool AllowRegisterRead() override { return true; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override;
	void InitMapper(RomData &romData) override;
	void LoadDiskData(vector<uint8_t> ipsData = vector<uint8_t>());
	vector<uint8_t> CreateIpsPatch();
	void Reset(bool softReset) override;

	uint32_t GetFdsDiskSideSize(uint8_t side);
	uint8_t ReadFdsDisk();
	void WriteFdsDisk(uint8_t value);

	void ProcessAutoDiskInsert();

	void ClockIrq();
	
	void ProcessCpuClock() override;
	void UpdateCrc(uint8_t value);

	void WriteRegister(uint16_t addr, uint8_t value) override;
	uint8_t ReadRegister(uint16_t addr) override;

	uint8_t ReadRam(uint16_t addr) override;

	void Serialize(Serializer& s) override;
	vector<MapperStateEntry> GetMapperStateEntries() override;

public:
	~Fds();

	void SaveBattery() override;

	uint32_t GetSideCount();

	void EjectDisk();
	void InsertDisk(uint32_t diskNumber);
	uint32_t GetCurrentDisk();
	bool IsDiskInserted();

	bool IsAutoInsertDiskEnabled();
};