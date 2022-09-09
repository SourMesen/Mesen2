#pragma once
#include "pch.h"
#include "Debugger/CodeDataLogger.h"

class NesCodeDataLogger : public CodeDataLogger
{
private:
	CodeDataLogger* _chrRomCdl = nullptr;

protected:
	void InternalLoadCdlFile(uint8_t* cdlData, uint32_t cdlSize) override
	{
		if(_chrRomCdl) {
			_chrRomCdl->SetCdlData(cdlData + _memSize, cdlSize - _memSize);
		}
	}

	void InternalSaveCdlFile(ofstream& cdlFile) override
	{
		if(_chrRomCdl) {
			cdlFile.write((char*)_chrRomCdl->GetRawData(), _chrRomCdl->GetSize());
		}
	}

public:
	NesCodeDataLogger(Debugger* debugger, MemoryType memType, uint32_t memSize, CpuType cpuType, uint32_t romCrc32, CodeDataLogger* chrRomCdl)
		: CodeDataLogger(debugger, memType, memSize, cpuType, romCrc32)
	{
		_chrRomCdl = chrRomCdl;
	}

	void Reset() override
	{
		CodeDataLogger::Reset();
		if(_chrRomCdl) {
			_chrRomCdl->Reset();
		}
	}

	CdlStatistics GetStatistics() override
	{
		CdlStatistics stats = CodeDataLogger::GetStatistics();
		if(_chrRomCdl) {
			CdlStatistics cdlStats = _chrRomCdl->GetStatistics();
			stats.DrawnChrBytes = cdlStats.CodeBytes;
			stats.TotalChrBytes = cdlStats.TotalBytes;
		}
		return stats;
	}

	void StripData(uint8_t* romBuffer, CdlStripOption flag) override
	{
		CodeDataLogger::StripData(romBuffer, flag);
		if(_chrRomCdl) {
			_chrRomCdl->StripData(romBuffer + _memSize, flag);
		}
	}
};
