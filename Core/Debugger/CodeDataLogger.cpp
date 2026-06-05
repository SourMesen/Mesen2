#include "pch.h"
#include "Debugger/Debugger.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/CdlManager.h"
#include "Debugger/Disassembler.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/FolderUtilities.h"

CodeDataLogger::CodeDataLogger(Debugger* debugger, MemoryType memType, uint32_t memSize, CpuType cpuType, uint32_t romCrc32)
{
	_memType = memType;
	_cpuType = cpuType;
	_memSize = memSize;
	_romCrc32 = romCrc32;
	_cdlData = new uint8_t[memSize];
	Reset();

	if(debugger && debugger->GetCdlManager()) {
		debugger->GetCdlManager()->RegisterCdl(memType, this);
	}
}

CodeDataLogger::~CodeDataLogger()
{
	delete[] _cdlData;
}

void CodeDataLogger::Reset()
{
	if(_cdlData) {
		memset(_cdlData, 0, _memSize);
	}
}

uint8_t* CodeDataLogger::GetRawData()
{
	return _cdlData;
}

uint32_t CodeDataLogger::GetSize()
{
	return _memSize;
}

MemoryType CodeDataLogger::GetMemoryType()
{
	return _memType;
}

/**
 * Read a CDL file and load contents into memory.
 *
 * Checks the 4-byte CRC32 value of the CDL file against the expected CRC32 for the current ROM. If checksums do not match, clears all CDL data and starts over from zero.
 *
 * @param cdlFilepath Valid path to the file to read.
 * @param autoResetCdl If false: ignore mismatched CRC and load CDL data even if checksums do not match.
 *
 * @return Returns true if the file was successfully read and loaded, and false otherwise.
 */
bool CodeDataLogger::LoadCdlFile(string cdlFilepath, bool autoResetCdl)
{
	VirtualFile cdlFile = cdlFilepath;
	if(cdlFile.IsValid()) {
		vector<uint8_t>& cdlData = cdlFile.GetData();
		uint32_t fileSize = (uint32_t)cdlData.size();

		if(fileSize >= _memSize && fileSize >= CodeDataLogger::HeaderSize) {
			Reset();

			if(memcmp(cdlData.data(), CodeDataLogger::CdlHeader.data(), CodeDataLogger::CdlHeader.length()) == 0) {
				uint32_t savedCrc = cdlData[5] | (cdlData[6] << 8) | (cdlData[7] << 16) | (cdlData[8] << 24);
				if((!autoResetCdl || savedCrc == _romCrc32) && fileSize >= _memSize + CodeDataLogger::HeaderSize) {
					memcpy(_cdlData, cdlData.data() + CodeDataLogger::HeaderSize, _memSize);
					InternalLoadCdlFile(cdlData.data() + CodeDataLogger::HeaderSize, (uint32_t)cdlData.size() - CodeDataLogger::HeaderSize);
					return true;
				}
			} else {
				MessageManager::Log("[Warning] CDL file doesn't contain header/CRC and may be incompatible.");

				//Older CRC-less CDL file, use as-is without checking CRC to avoid data loss
				memcpy(_cdlData, cdlData.data(), _memSize);
				InternalLoadCdlFile(cdlData.data(), (uint32_t)cdlData.size());
				return true;
			}
		}
	}
	return false;
}

bool CodeDataLogger::SaveCdlFile(string cdlFilepath)
{
	ofstream cdlFile(cdlFilepath, ios::out | ios::binary);
	if(cdlFile) {
		cdlFile.write(CodeDataLogger::CdlHeader.data(), CodeDataLogger::CdlHeader.length());
		cdlFile.put(_romCrc32 & 0xFF);
		cdlFile.put((_romCrc32 >> 8) & 0xFF);
		cdlFile.put((_romCrc32 >> 16) & 0xFF);
		cdlFile.put((_romCrc32 >> 24) & 0xFF);
		cdlFile.write((char*)_cdlData, _memSize);
		InternalSaveCdlFile(cdlFile);
		cdlFile.close();
		return true;
	}
	return false;
}

string CodeDataLogger::GetCdlFilePath(string romName)
{
	return FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(romName, false) + ".cdl");
}

CdlStatistics CodeDataLogger::GetStatistics()
{
	uint32_t codeSize = 0;
	uint32_t dataSize = 0;
	uint32_t bothSize = 0;

	for(uint32_t i = 0, len = _memSize; i < len; i++) {
		uint32_t isCode = (uint32_t)(_cdlData[i] & CdlFlags::Code);
		uint32_t isData = (uint32_t)(_cdlData[i] & CdlFlags::Data) >> 1;
		codeSize += isCode;
		dataSize += isData;
		bothSize += isCode & isData;
	}

	dataSize -= bothSize;

	CdlStatistics stats = {};
	stats.CodeBytes = codeSize;
	stats.DataBytes = dataSize;
	stats.TotalBytes = _memSize;
	return stats;
}

bool CodeDataLogger::IsCode(uint32_t absoluteAddr)
{
	if(absoluteAddr >= _memSize) {
		return false;
	}
	return (_cdlData[absoluteAddr] & CdlFlags::Code) != 0;
}

bool CodeDataLogger::IsJumpTarget(uint32_t absoluteAddr)
{
	if(absoluteAddr >= _memSize) {
		return false;
	}

	return (_cdlData[absoluteAddr] & CdlFlags::JumpTarget) != 0;
}

bool CodeDataLogger::IsSubEntryPoint(uint32_t absoluteAddr)
{
	if(absoluteAddr >= _memSize) {
		return false;
	}

	return (_cdlData[absoluteAddr] & CdlFlags::SubEntryPoint) != 0;
}

bool CodeDataLogger::IsData(uint32_t absoluteAddr)
{
	if(absoluteAddr >= _memSize) {
		return false;
	}
	return (_cdlData[absoluteAddr] & CdlFlags::Data) != 0;
}

void CodeDataLogger::SetCdlData(uint8_t *cdlData, uint32_t length)
{
	if(cdlData && length <= _memSize) {
		memcpy(_cdlData, cdlData, length);
	}
}

void CodeDataLogger::GetCdlData(uint32_t offset, uint32_t length, uint8_t *cdlData)
{
	if(cdlData && (offset + length <= _memSize)) {
		memcpy(cdlData, _cdlData + offset, length);
	}
}

uint8_t CodeDataLogger::GetFlags(uint32_t addr)
{
	if(addr >= _memSize) {
		return 0;
	}

	return _cdlData[addr];
}

uint32_t CodeDataLogger::GetFunctions(uint32_t functions[], uint32_t maxSize)
{
	if(!functions) {
		return 0;
	}

	uint32_t count = 0;
	for(int i = 0, len = _memSize; i < len; i++) {
		if(IsSubEntryPoint(i)) {
			functions[count] = i;
			count++;

			if(count == maxSize) {
				break;
			}
		}
	}
	return count;
}

void CodeDataLogger::MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags)
{
	if(end >= _memSize) {
		MessageManager::Log("[Warning] end >= _memSize; capping CDL bytes");
		end = _memSize - 1;
	}

	for(uint32_t i = start; i <= end; i++) {
		_cdlData[i] = (_cdlData[i] & 0xFC) | (int)flags;
	}
}

void CodeDataLogger::StripData(uint8_t* romBuffer, CdlStripOption flag)
{
	if(!romBuffer) {
		return;
	}

	if(flag == CdlStripOption::StripUnused) {
		for(uint32_t i = 0; i < _memSize; i++) {
			if(_cdlData[i] == 0) {
				romBuffer[i] = 0;
			}
		}
	} else if(flag == CdlStripOption::StripUsed) {
		for(uint32_t i = 0; i < _memSize; i++) {
			if(_cdlData[i] != 0) {
				romBuffer[i] = 0;
			}
		}
	}
}

void CodeDataLogger::RebuildPrgCache(Disassembler* dis)
{
	AddressInfo addrInfo;
	addrInfo.Type = _memType;
	for(uint32_t i = 0; i < _memSize; i++) {
		if(IsCode(i)) {
			addrInfo.Address = (int32_t)i;
			i += dis->BuildCache(addrInfo, 0, _cpuType) - 1;
		}
	}
}
