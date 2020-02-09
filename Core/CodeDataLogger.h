#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class MemoryManager;

class CodeDataLogger
{
private:
	MemoryManager *_memoryManager;
	uint8_t *_cdlData = nullptr;
	uint32_t _prgSize = 0;
	uint32_t _codeSize = 0;
	uint32_t _dataSize = 0;
	
	void CalculateStats();

public:
	CodeDataLogger(uint32_t prgSize, MemoryManager* memoryManager);
	~CodeDataLogger();

	void Reset();

	bool LoadCdlFile(string cdlFilepath);
	bool SaveCdlFile(string cdlFilepath);

	void SetFlags(int32_t absoluteAddr, uint8_t flags);

	CdlRatios GetRatios();

	bool IsCode(uint32_t absoluteAddr);
	bool IsJumpTarget(uint32_t absoluteAddr);
	bool IsSubEntryPoint(uint32_t absoluteAddr);
	bool IsData(uint32_t absoluteAddr);
	uint8_t GetCpuFlags(uint32_t absoluteAddr);

	void SetCdlData(uint8_t *cdlData, uint32_t length);
	void GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t *cdlData);

	void MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags);
};