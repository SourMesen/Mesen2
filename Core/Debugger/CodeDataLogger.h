#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class CodeDataLogger
{
private:
	uint8_t* _cdlData = nullptr;
	CpuType _cpuType = CpuType::Snes;
	SnesMemoryType _prgMemType;
	uint32_t _prgSize = 0;
	uint32_t _codeSize = 0;
	uint32_t _dataSize = 0;
	
	void CalculateStats();

public:
	CodeDataLogger(SnesMemoryType prgMemType, uint32_t prgSize, CpuType cpuType);
	~CodeDataLogger();

	void Reset();
	uint32_t GetPrgSize();
	SnesMemoryType GetPrgMemoryType();

	bool LoadCdlFile(string cdlFilepath, bool autoResetCdl, uint32_t romCrc);
	bool SaveCdlFile(string cdlFilepath, uint32_t romCrc);

	void SetFlags(int32_t absoluteAddr, uint8_t flags);

	CdlRatios GetRatios();

	bool IsCode(uint32_t absoluteAddr);
	bool IsJumpTarget(uint32_t absoluteAddr);
	bool IsSubEntryPoint(uint32_t absoluteAddr);
	bool IsData(uint32_t absoluteAddr);
	uint8_t GetCpuFlags(uint32_t absoluteAddr);
	CpuType GetCpuType(uint32_t absoluteAddr);

	void SetCdlData(uint8_t *cdlData, uint32_t length);
	void GetCdlData(uint32_t offset, uint32_t length, uint8_t *cdlData);
	uint8_t GetFlags(uint32_t addr);

	void MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags);
	void StripData(uint8_t* romBuffer, CdlStripOption flag);
};