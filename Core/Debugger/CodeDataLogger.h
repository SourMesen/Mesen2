#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Disassembler;
class Debugger;

class CodeDataLogger
{
protected:
	uint8_t* _cdlData = nullptr;
	CpuType _cpuType = CpuType::Snes;
	MemoryType _memType = {};
	uint32_t _memSize = 0;
	uint32_t _romCrc32 = 0;
	
public:
	CodeDataLogger(Debugger* debugger, MemoryType memType, uint32_t memSize, CpuType cpuType, uint32_t romCrc32);
	virtual ~CodeDataLogger();

	void Reset();
	uint32_t GetSize();
	MemoryType GetMemoryType();

	bool LoadCdlFile(string cdlFilepath, bool autoResetCdl);
	bool SaveCdlFile(string cdlFilepath);
	string GetCdlFilePath(string romName);

	template<uint8_t flags = 0>
	void SetCode(int32_t absoluteAddr)
	{
		_cdlData[absoluteAddr] |= CdlFlags::Code | flags;
	}

	template<uint8_t flags = 0>
	void SetData(int32_t absoluteAddr)
	{
		_cdlData[absoluteAddr] |= CdlFlags::Data | flags;
	}

	CdlRatios GetRatios();

	bool IsCode(uint32_t absoluteAddr);
	bool IsJumpTarget(uint32_t absoluteAddr);
	bool IsSubEntryPoint(uint32_t absoluteAddr);
	bool IsData(uint32_t absoluteAddr);

	void SetCdlData(uint8_t *cdlData, uint32_t length);
	void GetCdlData(uint32_t offset, uint32_t length, uint8_t *cdlData);
	uint8_t GetFlags(uint32_t addr);

	void MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags);
	void StripData(uint8_t* romBuffer, CdlStripOption flag);

	virtual void RebuildPrgCache(Disassembler* dis);
};