#pragma once
#include "Debugger/CodeDataLogger.h"
#include "Debugger/Disassembler.h"

namespace GbaCdlFlags
{
	enum GbaCdlFlags : uint8_t
	{
		Thumb = 0x20,
	};
}

class GbaCodeDataLogger final : public CodeDataLogger
{
private:
	uint8_t GetCpuFlags(uint32_t absoluteAddr)
	{
		return _cdlData[absoluteAddr] & (GbaCdlFlags::Thumb);
	}

public:
	using CodeDataLogger::CodeDataLogger;

	void RebuildPrgCache(Disassembler* dis) override
	{
		AddressInfo addrInfo;
		addrInfo.Type = _memType;
		for(uint32_t i = 0; i < _memSize; i++) {
			if(IsCode(i)) {
				addrInfo.Address = (int32_t)i;
				i += dis->BuildCache(addrInfo, GetCpuFlags(i), CpuType::Gba) - 1;
			}
		}
	}
};