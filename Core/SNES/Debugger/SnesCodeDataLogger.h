#pragma once
#include "Debugger/CodeDataLogger.h"
#include "Debugger/Disassembler.h"

namespace SnesCdlFlags
{
	enum SnesCdlFlags : uint8_t
	{
		IndexMode8 = 0x10,
		MemoryMode8 = 0x20,
		Gsu = 0x40,
		Cx4 = 0x80
	};
}

class SnesCodeDataLogger final : public CodeDataLogger
{
private:
	uint8_t GetCpuFlags(uint32_t absoluteAddr)
	{
		return _cdlData[absoluteAddr] & (SnesCdlFlags::MemoryMode8 | SnesCdlFlags::IndexMode8);
	}

	CpuType GetCpuType(uint32_t absoluteAddr)
	{
		if(_cdlData[absoluteAddr] & SnesCdlFlags::Gsu) {
			return CpuType::Gsu;
		} else if(_cdlData[absoluteAddr] & SnesCdlFlags::Cx4) {
			return CpuType::Cx4;
		}
		return CpuType::Snes;
	}

public:
	using CodeDataLogger::CodeDataLogger;

	void SetSnesCode(int32_t absoluteAddr, uint8_t flags)
	{
		_cdlData[absoluteAddr] |= CdlFlags::Code | flags;
	}

	template<uint8_t flags>
	void SetSnesCode(int32_t absoluteAddr)
	{
		_cdlData[absoluteAddr] |= CdlFlags::Code | flags;
	}

	void RebuildPrgCache(Disassembler* dis) override
	{
		//TODO GSU flags
		AddressInfo addrInfo;
		addrInfo.Type = _memType;
		for(uint32_t i = 0; i < _memSize; i++) {
			if(IsCode(i)) {
				addrInfo.Address = (int32_t)i;
				i += dis->BuildCache(addrInfo, GetCpuFlags(i), GetCpuType(i)) - 1;
			}
		}
	}
};