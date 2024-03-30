#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class Disassembler;
class Debugger;

class CodeDataLogger
{
protected:
	constexpr static int HeaderSize = 9; //"CDLv2" + 4-byte CRC32 value

	uint8_t* _cdlData = nullptr;
	CpuType _cpuType = CpuType::Snes;
	MemoryType _memType = {};
	uint32_t _memSize = 0;
	uint32_t _romCrc32 = 0;
	
	virtual void InternalLoadCdlFile(uint8_t* cdlData, uint32_t cdlSize) {}
	virtual void InternalSaveCdlFile(ofstream& cdlFile) {}

public:
	CodeDataLogger(Debugger* debugger, MemoryType memType, uint32_t memSize, CpuType cpuType, uint32_t romCrc32);
	virtual ~CodeDataLogger();

	virtual void Reset();
	uint8_t* GetRawData();
	uint32_t GetSize();
	MemoryType GetMemoryType();

	bool LoadCdlFile(string cdlFilepath, bool autoResetCdl);
	bool SaveCdlFile(string cdlFilepath);
	string GetCdlFilePath(string romName);

	template<uint8_t flags = 0, uint8_t accessWidth = 1>
	void SetCode(int32_t absoluteAddr)
	{
		for(int i = 0; i < accessWidth; i++) {
			_cdlData[absoluteAddr+i] |= CdlFlags::Code | flags;
		}
	}

	template<uint8_t accessWidth = 1>
	void SetCode(int32_t absoluteAddr, uint8_t flags)
	{
		_cdlData[absoluteAddr] |= CdlFlags::Code | flags; //only sets extra flags on first byte
		if constexpr(accessWidth > 1) {
			for(int i = 1; i < accessWidth; i++) {
				_cdlData[absoluteAddr+i] |= CdlFlags::Code;
			}
		}
	}

	template<uint8_t flags = 0, uint8_t accessWidth = 1>
	void SetData(int32_t absoluteAddr)
	{
		for(int i = 0; i < accessWidth; i++) {
			_cdlData[absoluteAddr+i] |= CdlFlags::Data | flags;
		}
	}

	virtual CdlStatistics GetStatistics();

	bool IsCode(uint32_t absoluteAddr);
	bool IsJumpTarget(uint32_t absoluteAddr);
	bool IsSubEntryPoint(uint32_t absoluteAddr);
	bool IsData(uint32_t absoluteAddr);

	void SetCdlData(uint8_t *cdlData, uint32_t length);
	void GetCdlData(uint32_t offset, uint32_t length, uint8_t *cdlData);
	uint8_t GetFlags(uint32_t addr);
	
	uint32_t GetFunctions(uint32_t functions[], uint32_t maxSize);

	void MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags);
	virtual void StripData(uint8_t* romBuffer, CdlStripOption flag);

	virtual void RebuildPrgCache(Disassembler* dis);
};