#include "stdafx.h"
#include <algorithm>
#include "Disassembler.h"
#include "DisassemblyInfo.h"
#include "Cpu.h"
#include "MemoryManager.h"
#include "CpuTypes.h"
#include "Console.h"
#include "CodeDataLogger.h"
#include "BaseCartridge.h"
#include "../Utilities/HexUtilities.h"

Disassembler::Disassembler(shared_ptr<Console> console, shared_ptr<CodeDataLogger> cdl)
{
	_cdl = cdl;
	_console = console.get();
	_memoryManager = console->GetMemoryManager().get();

	_prgRom = console->GetCartridge()->DebugGetPrgRom();
	_prgRomSize = console->GetCartridge()->DebugGetPrgRomSize();
	_sram = console->GetCartridge()->DebugGetSaveRam();
	_sramSize = console->GetCartridge()->DebugGetSaveRamSize();
	_wram = _memoryManager->DebugGetWorkRam();
	_wramSize = MemoryManager::WorkRamSize;
	
	_prgCache = vector<shared_ptr<DisassemblyInfo>>(_prgRomSize);
	_sramCache = vector<shared_ptr<DisassemblyInfo>>(_sramSize);
	_wramCache = vector<shared_ptr<DisassemblyInfo>>(_wramSize);
}

void Disassembler::GetSource(AddressInfo &info, uint8_t **source, uint32_t &size, vector<shared_ptr<DisassemblyInfo>> **cache)
{
	switch(info.Type) {
		case SnesMemoryType::PrgRom:
			*source = _prgRom;
			*cache = &_prgCache;
			size = _prgRomSize;
			break;

		case SnesMemoryType::WorkRam:
			*source = _wram;
			*cache = &_wramCache;
			size = MemoryManager::WorkRamSize;
			break;

		case SnesMemoryType::SaveRam:
			*source = _sram;
			*cache = &_sramCache;
			size = _sramSize;
			break;

		default:
			throw std::runtime_error("Disassembler::GetSource() invalid memory type");
	}
}

uint32_t Disassembler::BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags)
{
	uint8_t *source;
	uint32_t sourceLength;
	vector<shared_ptr<DisassemblyInfo>> *cache;
	GetSource(addrInfo, &source, sourceLength, &cache);

	if(addrInfo.Address >= 0) {
		DisassemblyInfo *disInfo = (*cache)[addrInfo.Address].get();
		if(!disInfo) {
			shared_ptr<DisassemblyInfo> disassemblyInfo(new DisassemblyInfo(source+addrInfo.Address, cpuFlags));
			(*cache)[addrInfo.Address] = disassemblyInfo;
			_needDisassemble = true;
			disInfo = disassemblyInfo.get();
		}
		return disInfo->GetOperandSize() + 1;
	}
	return 0;
}

void Disassembler::InvalidateCache(AddressInfo addrInfo)
{
	uint8_t *source;
	uint32_t sourceLength;
	vector<shared_ptr<DisassemblyInfo>> *cache;
	GetSource(addrInfo, &source, sourceLength, &cache);

	if(addrInfo.Address >= 0) {
		for(int i = 0; i < 4; i++) {
			if(addrInfo.Address >= i) {
				if((*cache)[addrInfo.Address - i]) {
					_needDisassemble = true;
					(*cache)[addrInfo.Address - i].reset();
				}
			}
		}
	}
}

void Disassembler::Disassemble()
{
	if(!_needDisassemble) {
		return;
	}

	_needDisassemble = false;

	auto lock = _disassemblyLock.AcquireSafe(); 
		
	_disassembly.clear();

	uint8_t *source;
	uint32_t sourceLength;
	vector<shared_ptr<DisassemblyInfo>> *cache;

	bool disassembleAll = false;
	bool inUnknownBlock = false;
	shared_ptr<DisassemblyInfo> disassemblyInfo;
	shared_ptr<DisassemblyInfo> tmpInfo(new DisassemblyInfo());

	AddressInfo addrInfo = {};
	AddressInfo prevAddrInfo = {};
	for(int32_t i = 0; i <= 0xFFFFFF; i++) {
		prevAddrInfo = addrInfo;
		addrInfo = _memoryManager->GetAbsoluteAddress(i);

		if(addrInfo.Address < 0) {
			continue;
		}

		GetSource(addrInfo, &source, sourceLength, &cache);

		DisassemblyInfo *disassemblyInfo = (*cache)[addrInfo.Address].get();
		
		uint8_t opSize = 0;
		uint8_t opCode = (source + addrInfo.Address)[0];
		bool needRealign = true;
		if(!disassemblyInfo && disassembleAll) {
			opSize = DisassemblyInfo::GetOperandSize(opCode, 0);
		} else if(disassemblyInfo) {
			opSize = disassemblyInfo->GetOperandSize();
			needRealign = false;
		}

		if(disassemblyInfo || disassembleAll) {
			if(inUnknownBlock) {
				_disassembly.push_back(DisassemblyResult(prevAddrInfo, i - 1, LineFlags::BlockEnd));
				inUnknownBlock = false;
			}

			if(addrInfo.Type == SnesMemoryType::PrgRom && _cdl->IsSubEntryPoint(addrInfo.Address)) {
				_disassembly.push_back(DisassemblyResult(-1, LineFlags::SubStart | LineFlags::BlockStart | LineFlags::VerifiedCode));
			}

			_disassembly.push_back(DisassemblyResult(addrInfo, i));
			if(needRealign) {
				for(int j = 1; j < opSize; j++) {
					if((*cache)[addrInfo.Address + j]) {
						break;
					}
					i++;
				}
			} else {
				i += opSize;
			}

			if(opCode == 0x60 || opCode == 0x6B) {
				//End of function
				_disassembly.push_back(DisassemblyResult(-1, LineFlags::VerifiedCode | LineFlags::BlockEnd));
			} 
		} else {
			if(!inUnknownBlock) {
				inUnknownBlock = true;
				_disassembly.push_back(DisassemblyResult(addrInfo, i, LineFlags::BlockStart));
				_disassembly.push_back(DisassemblyResult(-1, LineFlags::None));
			}
		}
	}

	if(inUnknownBlock) {
		_disassembly.push_back(DisassemblyResult(addrInfo, 0xFFFFFF, LineFlags::BlockEnd));
	}
}

DisassemblyInfo Disassembler::GetDisassemblyInfo(AddressInfo &info)
{
	DisassemblyInfo* disassemblyInfo = nullptr;
	switch(info.Type) {
		case SnesMemoryType::PrgRom: disassemblyInfo = _prgCache[info.Address].get(); break;
		case SnesMemoryType::WorkRam: disassemblyInfo = _wramCache[info.Address].get(); break;
		case SnesMemoryType::SaveRam: disassemblyInfo = _sramCache[info.Address].get(); break;
	}

	if(disassemblyInfo) {
		return *disassemblyInfo;
	} else {
		return DisassemblyInfo();
	}
}

uint32_t Disassembler::GetLineCount()
{
	auto lock = _disassemblyLock.AcquireSafe();
	return (uint32_t)_disassembly.size();
}

uint32_t Disassembler::GetLineIndex(uint32_t cpuAddress)
{
	auto lock = _disassemblyLock.AcquireSafe();
	for(int i = 0; i < _disassembly.size(); i++) {
		if(_disassembly[i].CpuAddress == cpuAddress) {
			return i;
		}
	}
	return 0;
}

bool Disassembler::GetLineData(uint32_t lineIndex, CodeLineData &data)
{
	auto lock =_disassemblyLock.AcquireSafe();
	if(lineIndex < _disassembly.size()) {
		DisassemblyResult result = _disassembly[lineIndex];
		data.Address = result.CpuAddress;
		data.AbsoluteAddress = result.Address.Address;
		data.Flags = result.Flags;

		switch(result.Address.Type) {
			case SnesMemoryType::PrgRom: data.Flags |= (uint8_t)LineFlags::PrgRom; break;
			case SnesMemoryType::WorkRam: data.Flags |= (uint8_t)LineFlags::WorkRam; break;
			case SnesMemoryType::SaveRam: data.Flags |= (uint8_t)LineFlags::SaveRam; break;
		}

		bool isBlockStartEnd = (data.Flags & (LineFlags::BlockStart | LineFlags::BlockEnd)) != 0;

		if(!isBlockStartEnd && result.Address.Address >= 0) {
			shared_ptr<DisassemblyInfo> disInfo;
			uint8_t *source;
			uint32_t sourceLength;
			vector<shared_ptr<DisassemblyInfo>> *cache;
			GetSource(result.Address, &source, sourceLength, &cache);
			disInfo = (*cache)[result.Address.Address];

			if(!disInfo) {
				disInfo.reset(new DisassemblyInfo(source + result.Address.Address, 0));
			} else {
				data.Flags |= (uint8_t)LineFlags::VerifiedCode;
			}

			string text;
			disInfo->GetDisassembly(text, result.CpuAddress);
			memcpy(data.Text, text.c_str(), std::min<int>((int)text.size(), 1000));

			data.OpSize = disInfo->GetOperandSize() + 1;
			data.EffectiveAddress = disInfo->GetEffectiveAddress();
			data.Value = _memoryManager->Peek(result.CpuAddress);

			disInfo->GetByteCode(data.ByteCode);
			data.Comment[0] = 0;
		} else {
			if(data.Flags & LineFlags::SubStart) {
				string label = "sub start";
				memcpy(data.Text, label.c_str(), label.size() + 1);
			} else if(data.Flags & LineFlags::BlockStart) {
				string label = (data.Flags & LineFlags::VerifiedCode) ? "verified code" : "unidentified";
				memcpy(data.Text, label.c_str(), label.size() + 1);
			}
		}
		return true;
	}
	return false;
}

int32_t Disassembler::SearchCode(const char *searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards)
{
	auto lock = _disassemblyLock.AcquireSafe();
	int step = searchBackwards ? -1 : 1;
	CodeLineData lineData = {};
	for(int i = startPosition; i != endPosition; i += step) {
		GetLineData(i, lineData);
		string line = lineData.Text;
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);

		if(line.find(searchString) != string::npos) {
			return i;
		}

		//Continue search from start/end of document
		if(!searchBackwards && i == _disassembly.size() - 1) {
			i = 0;
		} else if(searchBackwards && i == 0) {
			i = (int32_t)(_disassembly.size() - 1);
		}
	}

	return -1;
}
