#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Debugger/IExtModeMapperDebug.h"

class RainbowAudio;
class FlashS29;

class Rainbow : public BaseMapper, public IExtModeMapperDebug
{
private:
	struct NtControl
	{
		bool AttrExtMode;
		bool BgExtMode;
		uint8_t FpgaRamSrc;
		bool FillMode;
		uint8_t Source;

		uint8_t ToByte()
		{
			return (
				((uint8_t)AttrExtMode << 0) |
				((uint8_t)BgExtMode << 1) |
				(FpgaRamSrc << 2) |
				((uint8_t)FillMode << 5) |
				(Source << 6)
			);
		}
	};

	unique_ptr<RainbowAudio> _audio;
	unique_ptr<FlashS29> _prgFlash;
	unique_ptr<FlashS29> _chrFlash;

	vector<uint8_t> _orgPrgRom;
	vector<uint8_t> _orgChrRom;

	uint16_t _highBanks[8] = {};
	uint16_t _lowBanks[2] = {};
	uint16_t _chrBanks[16] = {};

	uint8_t _fpgaRamBank = 0;

	uint8_t _highMode = 0;
	uint8_t _lowMode = 0;
	uint8_t _chrMode = 0;
	uint8_t _chrSource = 0;
	bool _windowEnabled = false;
	bool _spriteExtMode = false;
	uint8_t _bgExtModeOffset = 0;

	uint8_t _ntBanks[4] = {};
	NtControl _ntControl[4] = {};

	uint8_t _fillModeTileIndex = 0;
	uint8_t _fillModeAttrIndex = 0;
	
	NtControl _windowControl = {};
	uint8_t _windowBank = 0;
	uint8_t _windowX1 = 0;
	uint8_t _windowX2 = 0;
	uint8_t _windowY1 = 0;
	uint8_t _windowY2 = 0;
	uint8_t _windowScrollX = 0;
	uint8_t _windowScrollY = 0;
	bool _inWindow = false;
	
	bool _slIrqEnabled = false;
	bool _slIrqPending = false;
	uint8_t _slIrqScanline = 0;
	uint8_t _slIrqOffset = 0;

	uint16_t _lastPpuReadAddr = 0;
	int16_t _scanlineCounter = 0;
	uint8_t _ppuIdleCounter = 0;
	uint8_t _ntReadCounter = 0;
	uint8_t _ppuReadCounter = 0;

	bool _inFrame = false;
	bool _inHBlank = false;
	uint8_t _jitterCounter = 0;

	uint16_t _cpuIrqCounter = 0;
	uint16_t _cpuIrqReloadValue = 0;
	bool _cpuIrqEnabled = false;
	bool _cpuIrqPending = false;
	bool _cpuIrqEnableAfterAck = false;
	bool _cpuIrqAckOn4011 = false;

	uint16_t _fpgaRamAddr = 0;
	uint8_t _fpgaRamInc = 0;

	bool _nmiVectorEnabled = false;
	bool _irqVectorEnabled = false;
	uint16_t _nmiVectorAddr = 0;
	uint16_t _irqVectorAddr = 0;

	bool _overrideTileFetch = false;
	uint8_t _extData = 0;
	uint8_t _ntFetchCounter = 0;

	uint8_t _spriteExtData[64] = {};
	uint8_t _oamPosY[64] = {};
	uint8_t _oamMappings[8] = {};
	uint8_t _spriteExtBank = 0;
	bool _largeSprites = false;
	uint8_t _oamAddr = 0;

	uint8_t _oamExtUpdatePage = 0;
	uint8_t _oamSlowUpdatePage = 0;
	uint8_t _oamCode[0x506] = {};
	bool _oamCodeLocked = false;

	bool _espEnabled = false;
	bool _wifiIrqEnabled = false;
	bool _wifiIrqPending = false;
	bool _dataSent = false;
	bool _dataReceived = false;
	bool _dataReady = false;
	uint8_t _sendSrcAddr = 0;
	uint8_t _recvDstAddr = 0;

	uint8_t ReadChr(uint32_t addr);
	void UpdateInWindowFlag();
	void UpdateIrqStatus();
	void AckCpuIrq();
	void ProcessSpriteEval();
	void DetectScanlineStart(uint16_t addr);

	void GenerateOamClear();
	void GenerateExtUpdate();
	void GenerateOamSlowUpdate();

	PrgMemoryType GetWorkRamType();
	void SelectHighBank(uint16_t start, uint16_t size, uint8_t reg);
	void SelectLowBank(uint16_t start, uint16_t size, uint8_t reg);
	void SelectChrBank(uint16_t start, uint16_t size, uint8_t reg);
	void UpdateState();

	void ApplySaveData();

protected:
	uint16_t GetPrgPageSize() override { return 0x1000; }
	uint16_t GetChrPageSize() override { return 0x200; }
	uint32_t GetMapperRamSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x4785; }
	bool AllowRegisterRead() override { return true; }
	bool EnableCpuClockHook() override { return true; }
	bool EnableCustomVramRead() override { return true; }

	void InitMapper() override;
	void SaveBattery() override;
	void Reset(bool softReset) override;
	void OnAfterResetPowerOn() override;

	uint8_t MapperReadVram(uint16_t addr, MemoryOperationType memoryOperationType) override;
	void MapperWriteVram(uint16_t addr, uint8_t value) override;

public:
	Rainbow();

	void WriteRam(uint16_t addr, uint8_t value) override;
	uint8_t ReadRam(uint16_t addr) override;
	
	void ProcessCpuClock() override;

	uint8_t ReadRegister(uint16_t addr) override;
	void WriteRegister(uint16_t addr, uint8_t value) override;

	//Debugger
	vector<MapperStateEntry> GetMapperStateEntries() override;
	bool HasExtendedAttributes(ExtModeConfig& cfg, uint8_t ntIndex) override;
	bool HasExtendedBackground(ExtModeConfig& cfg, uint8_t ntIndex) override;
	bool HasExtendedSprites(ExtModeConfig& cfg) override;
	uint8_t DebugReadChr(ExtModeConfig& cfg, uint32_t addr);
	uint8_t GetExAttributePalette(ExtModeConfig& cfg, uint8_t ntIndex, uint16_t ntOffset) override;
	uint8_t GetExBackgroundChrData(ExtModeConfig& cfg, uint8_t ntIndex, uint16_t ntOffset, uint16_t chrAddr) override;
	uint8_t GetExSpriteChrData(ExtModeConfig& cfg, uint8_t spriteIndex, uint16_t chrAddr) override;
	ExtModeConfig GetExModeConfig() override;

	void Serialize(Serializer& s) override;
};