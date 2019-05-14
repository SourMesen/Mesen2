#include "stdafx.h"
#include "CodeDataLogger.h"
#include "MemoryManager.h"

CodeDataLogger::CodeDataLogger(uint32_t prgSize, MemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
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

bool CodeDataLogger::LoadCdlFile(string cdlFilepath)
{
	ifstream cdlFile(cdlFilepath, ios::in | ios::binary);
	if(cdlFile) {
		cdlFile.seekg(0, std::ios::end);
		size_t fileSize = (size_t)cdlFile.tellg();
		cdlFile.seekg(0, std::ios::beg);

		if(fileSize == _prgSize) {
			Reset();

			cdlFile.read((char*)_cdlData, _prgSize);
			cdlFile.close();

			CalculateStats();
			
			return true;
		}
	}
	return false;
}

void CodeDataLogger::CalculateStats()
{
	_codeSize = 0;
	_dataSize = 0;

	for(int i = 0, len = _prgSize; i < len; i++) {
		if(IsCode(i)) {
			_codeSize++;
		} else if(IsData(i)) {
			_dataSize++;
		}
	}
}

bool CodeDataLogger::SaveCdlFile(string cdlFilepath)
{
	ofstream cdlFile(cdlFilepath, ios::out | ios::binary);
	if(cdlFile) {
		cdlFile.write((char*)_cdlData, _prgSize);
		cdlFile.close();
		return true;
	}
	return false;
}

void CodeDataLogger::SetFlags(int32_t absoluteAddr, uint8_t flags)
{
	if(absoluteAddr >= 0 && absoluteAddr < (int32_t)_prgSize) {
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

void CodeDataLogger::SetCdlData(uint8_t *cdlData, uint32_t length)
{
	if(length <= _prgSize) {
		memcpy(_cdlData, cdlData, length);
		CalculateStats();
	}
}

void CodeDataLogger::GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t *cdlData)
{
	if(memoryType == SnesMemoryType::PrgRom) {
		memcpy(cdlData, _cdlData + offset, length);
	} else if(memoryType == SnesMemoryType::CpuMemory) {
		for(uint32_t i = 0; i < length; i++) {
			AddressInfo info = _memoryManager->GetAbsoluteAddress(offset + i);
			cdlData[i] = (info.Type == SnesMemoryType::PrgRom && info.Address >= 0) ? _cdlData[info.Address] : 0;
		}
	}
}