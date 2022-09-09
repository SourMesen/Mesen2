#include "pch.h"
#include "Debugger/CdlManager.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Disassembler.h"

void CdlManager::SetCdlData(MemoryType memType, uint8_t* cdlData, uint32_t length)
{
	DebugBreakHelper helper(_debugger);
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	if(cdl) {
		cdl->SetCdlData(cdlData, length);
		RefreshCodeCache();
	}
}

void CdlManager::MarkBytesAs(MemoryType memType, uint32_t start, uint32_t end, uint8_t flags)
{
	DebugBreakHelper helper(_debugger);
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	if(cdl) {
		cdl->MarkBytesAs(start, end, flags);
		RefreshCodeCache();
	}
}

CdlStatistics CdlManager::GetCdlStatistics(MemoryType memType)
{
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	return cdl ? cdl->GetStatistics() : CdlStatistics {};
}

uint32_t CdlManager::GetCdlFunctions(MemoryType memType, uint32_t functions[], uint32_t maxSize)
{
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	return cdl ? cdl->GetFunctions(functions, maxSize) : 0;
}

void CdlManager::ResetCdl(MemoryType memType)
{
	DebugBreakHelper helper(_debugger);
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	if(cdl) {
		cdl->Reset();
		RefreshCodeCache();
	}
}

void CdlManager::LoadCdlFile(MemoryType memType, char* cdlFile)
{
	DebugBreakHelper helper(_debugger);
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	if(cdl) {
		cdl->LoadCdlFile(cdlFile, false);
		RefreshCodeCache();
	}
}

void CdlManager::SaveCdlFile(MemoryType memType, char* cdlFile)
{
	DebugBreakHelper helper(_debugger);
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	if(cdl) {
		cdl->SaveCdlFile(cdlFile);
	}
}

void CdlManager::RegisterCdl(MemoryType memType, CodeDataLogger* cdl)
{
	_codeDataLoggers[(int)memType] = cdl;
}

void CdlManager::RefreshCodeCache()
{
	_disassembler->ResetPrgCache();

	for(CodeDataLogger* cdl : _codeDataLoggers) {
		if(cdl) {
			cdl->RebuildPrgCache(_disassembler);
		}
	}
}

CdlManager::CdlManager(Debugger* debugger, Disassembler* disassembler)
{
	_debugger = debugger;
	_disassembler = disassembler;
}

void CdlManager::GetCdlData(uint32_t offset, uint32_t length, MemoryType memoryType, uint8_t* cdlData)
{
	CodeDataLogger* cdl = GetCodeDataLogger(memoryType);
	if(cdl) {
		cdl->GetCdlData(offset, length, cdlData);
	} else {
		AddressInfo relAddress;
		relAddress.Type = memoryType;
		for(uint32_t i = 0; i < length; i++) {
			relAddress.Address = offset + i;
			AddressInfo info = _debugger->GetAbsoluteAddress(relAddress);
			if(info.Address >= 0) {
				cdl = GetCodeDataLogger(info.Type);
				cdlData[i] = cdl ? cdl->GetFlags(info.Address) : 0;
			} else {
				cdlData[i] = 0;
			}
		}
	}
}

int16_t CdlManager::GetCdlFlags(MemoryType memType, uint32_t addr)
{
	CodeDataLogger* cdl = GetCodeDataLogger(memType);
	if(cdl) {
		return cdl->GetFlags(addr);
	} else {
		if(DebugUtilities::IsRelativeMemory(memType)) {
			AddressInfo info = _debugger->GetAbsoluteAddress({ (int32_t)addr, memType });
			if(info.Address >= 0) {
				cdl = GetCodeDataLogger(info.Type);
				return cdl ? cdl->GetFlags(info.Address) : -1;
			}
		}
	}
	return -1;
}

CodeDataLogger* CdlManager::GetCodeDataLogger(MemoryType memType)
{
	return _codeDataLoggers[(int)memType];
}
