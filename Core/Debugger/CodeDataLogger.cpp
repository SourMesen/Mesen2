#include "stdafx.h"
#include "CodeDataLogger.h"
#include "Utilities/VirtualFile.h"

CodeDataLogger::CodeDataLogger(MemoryType prgMemType, uint32_t prgSize, CpuType cpuType)
{
	_prgMemType = prgMemType;
	_cpuType = cpuType;
	_prgSize = prgSize;
	_cdlData = new uint8_t[prgSize];
	Reset();
}

CodeDataLogger::~CodeDataLogger()
{
	delete[] _cdlData;
}

void CodeDataLogger::Reset()
{
	_codeSize = 0;
	_dataSize = 0;
	memset(_cdlData, 0, _prgSize);
}

uint32_t CodeDataLogger::GetPrgSize()
{
	return _prgSize;
}

MemoryType CodeDataLogger::GetPrgMemoryType()
{
	return _prgMemType;
}

bool CodeDataLogger::LoadCdlFile(string cdlFilepath, bool autoResetCdl, uint32_t romCrc)
{
	VirtualFile cdlFile = cdlFilepath;
	if(cdlFile.IsValid()) {
		uint32_t fileSize = (uint32_t)cdlFile.GetSize();
		vector<uint8_t> cdlData;
		cdlFile.ReadFile(cdlData);

		if(fileSize >= _prgSize) {
			Reset();

			constexpr int headerSize = 9; //"CDLv2" + 4-byte CRC32 value
			if(memcmp(cdlData.data(), "CDLv2", 5) == 0) {
				uint32_t savedCrc = cdlData[5] | (cdlData[6] << 8) | (cdlData[7] << 16) | (cdlData[8] << 24);
				if((autoResetCdl && savedCrc != romCrc) || fileSize < _prgSize + headerSize) {
					memset(_cdlData, 0, _prgSize);
				} else {
					memcpy(_cdlData, cdlData.data() + headerSize, _prgSize);
				}
			} else {
				//Older CRC-less CDL file, use as-is without checking CRC to avoid data loss
				memcpy(_cdlData, cdlData.data(), _prgSize);
			}

			CalculateStats();
			
			return true;
		}
	}
	return false;
}

bool CodeDataLogger::SaveCdlFile(string cdlFilepath, uint32_t romCrc)
{
	ofstream cdlFile(cdlFilepath, ios::out | ios::binary);
	if(cdlFile) {
		cdlFile.write("CDLv2", 5);
		cdlFile.put(romCrc & 0xFF);
		cdlFile.put((romCrc >> 8) & 0xFF);
		cdlFile.put((romCrc >> 16) & 0xFF);
		cdlFile.put((romCrc >> 24) & 0xFF);
		cdlFile.write((char*)_cdlData, _prgSize);
		cdlFile.close();
		return true;
	}
	return false;
}

void CodeDataLogger::CalculateStats()
{
	uint32_t codeSize = 0;
	uint32_t dataSize = 0;

	for(int i = 0, len = _prgSize; i < len; i++) {
		if(IsCode(i)) {
			codeSize++;
		} else if(IsData(i)) {
			dataSize++;
		}
	}

	_codeSize = codeSize;
	_dataSize = dataSize;
}

void CodeDataLogger::SetFlags(int32_t absoluteAddr, uint8_t flags)
{
	if((_cdlData[absoluteAddr] & flags) != flags) {
		if(flags & CdlFlags::Code) {
			_cdlData[absoluteAddr] = flags | (_cdlData[absoluteAddr] & ~(CdlFlags::Data | CdlFlags::IndexMode8 | CdlFlags::MemoryMode8));
		} else if(flags & CdlFlags::Data) {
			if(!IsCode(absoluteAddr)) {
				_cdlData[absoluteAddr] |= flags;
			}
		} else {
			_cdlData[absoluteAddr] |= flags;
		}
	}
}

CdlRatios CodeDataLogger::GetRatios()
{
	CalculateStats();

	CdlRatios ratios;
	ratios.CodeRatio = (float)_codeSize / (float)_prgSize;
	ratios.DataRatio = (float)_dataSize / (float)_prgSize;
	ratios.PrgRatio = (float)(_codeSize + _dataSize) / (float)_prgSize;
	return ratios;
}

bool CodeDataLogger::IsCode(uint32_t absoluteAddr)
{
	return (_cdlData[absoluteAddr] & CdlFlags::Code) != 0;
}

bool CodeDataLogger::IsJumpTarget(uint32_t absoluteAddr)
{
	return (_cdlData[absoluteAddr] & CdlFlags::JumpTarget) != 0;
}

bool CodeDataLogger::IsSubEntryPoint(uint32_t absoluteAddr)
{
	return (_cdlData[absoluteAddr] & CdlFlags::SubEntryPoint) != 0;
}

bool CodeDataLogger::IsData(uint32_t absoluteAddr)
{
	return (_cdlData[absoluteAddr] & CdlFlags::Data) != 0;
}

uint8_t CodeDataLogger::GetCpuFlags(uint32_t absoluteAddr)
{
	return _cdlData[absoluteAddr] & (CdlFlags::MemoryMode8 | CdlFlags::IndexMode8);
}

CpuType CodeDataLogger::GetCpuType(uint32_t absoluteAddr)
{
	//TODO?
	if(_cpuType == CpuType::Snes) {
		if(_cdlData[absoluteAddr] & CdlFlags::Gsu) {
			return CpuType::Gsu;
		} else if(_cdlData[absoluteAddr] & CdlFlags::Cx4) {
			return CpuType::Cx4;
		}
	}
	return _cpuType;
}

void CodeDataLogger::SetCdlData(uint8_t *cdlData, uint32_t length)
{
	if(length <= _prgSize) {
		memcpy(_cdlData, cdlData, length);
	}
}

void CodeDataLogger::GetCdlData(uint32_t offset, uint32_t length, uint8_t *cdlData)
{
	memcpy(cdlData, _cdlData + offset, length);
}

uint8_t CodeDataLogger::GetFlags(uint32_t addr)
{
	return _cdlData[addr];
}

void CodeDataLogger::MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags)
{
	for(uint32_t i = start; i <= end; i++) {
		_cdlData[i] = (_cdlData[i] & 0xFC) | (int)flags;
	}
}

void CodeDataLogger::StripData(uint8_t* romBuffer, CdlStripOption flag)
{
	if(flag == CdlStripOption::StripUnused) {
		for(uint32_t i = 0; i < _prgSize; i++) {
			if(_cdlData[i] == 0) {
				romBuffer[i] = 0;
			}
		}
	} else if(flag == CdlStripOption::StripUsed) {
		for(uint32_t i = 0; i < _prgSize; i++) {
			if(_cdlData[i] != 0) {
				romBuffer[i] = 0;
			}
		}
	}
}