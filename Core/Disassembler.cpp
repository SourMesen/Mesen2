#include "stdafx.h"
#include <algorithm>
#include "Disassembler.h"
#include "DisassemblyInfo.h"
#include "Cpu.h"
#include "Spc.h"
#include "NecDsp.h"
#include "Sa1.h"
#include "Gsu.h"
#include "Debugger.h"
#include "MemoryManager.h"
#include "LabelManager.h"
#include "CpuTypes.h"
#include "MemoryDumper.h"
#include "Console.h"
#include "CodeDataLogger.h"
#include "DebugBreakHelper.h"
#include "BaseCartridge.h"
#include "EmuSettings.h"
#include "DebugUtilities.h"
#include "../Utilities/FastString.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/StringUtilities.h"

Disassembler::Disassembler(shared_ptr<Console> console, shared_ptr<CodeDataLogger> cdl, Debugger* debugger)
{
	_cdl = cdl;
	_debugger = debugger;
	_labelManager = debugger->GetLabelManager();
	_console = console.get();
	_spc = console->GetSpc().get();
	_memoryDumper = _debugger->GetMemoryDumper().get();
	_memoryManager = console->GetMemoryManager().get();

	_prgRom = console->GetCartridge()->DebugGetPrgRom();
	_prgRomSize = console->GetCartridge()->DebugGetPrgRomSize();
	_sram = console->GetCartridge()->DebugGetSaveRam();
	_sramSize = console->GetCartridge()->DebugGetSaveRamSize();
	_wram = _memoryManager->DebugGetWorkRam();
	_wramSize = MemoryManager::WorkRamSize;

	_spcRam = console->GetSpc()->GetSpcRam();
	_spcRamSize = Spc::SpcRamSize;
	_spcRom = console->GetSpc()->GetSpcRom();
	_spcRomSize = Spc::SpcRomSize;

	_necDspProgramRom = console->GetCartridge()->GetDsp() ? console->GetCartridge()->GetDsp()->DebugGetProgramRom() : nullptr;
	_necDspProgramRomSize = console->GetCartridge()->GetDsp() ? console->GetCartridge()->GetDsp()->DebugGetProgramRomSize() : 0;

	_sa1InternalRam = console->GetCartridge()->GetSa1() ? console->GetCartridge()->GetSa1()->DebugGetInternalRam() : nullptr;
	_sa1InternalRamSize = console->GetCartridge()->GetSa1() ? console->GetCartridge()->GetSa1()->DebugGetInternalRamSize() : 0;

	_gsuWorkRam = console->GetCartridge()->GetGsu() ? console->GetCartridge()->GetGsu()->DebugGetWorkRam() : nullptr;
	_gsuWorkRamSize = console->GetCartridge()->GetGsu() ? console->GetCartridge()->GetGsu()->DebugGetWorkRamSize() : 0;

	_prgCache = vector<DisassemblyInfo>(_prgRomSize);
	_sramCache = vector<DisassemblyInfo>(_sramSize);
	_wramCache = vector<DisassemblyInfo>(_wramSize);
	_spcRamCache = vector<DisassemblyInfo>(_spcRamSize);
	_spcRomCache = vector<DisassemblyInfo>(_spcRomSize);
	_necDspRomCache = vector<DisassemblyInfo>(_necDspProgramRomSize);
	_sa1InternalRamCache = vector<DisassemblyInfo>(_sa1InternalRamSize);
	_gsuWorkRamCache = vector<DisassemblyInfo>(_gsuWorkRamSize);

	for(int i = 0; i < (int)DebugUtilities::GetLastCpuType(); i++) {
		_needDisassemble[i] = true;
	}
}

void Disassembler::GetSource(AddressInfo &info, uint8_t **source, uint32_t &size, vector<DisassemblyInfo> **cache)
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
		
		case SnesMemoryType::SpcRam:
			*source = _spcRam;
			*cache = &_spcRamCache;
			size = _spcRamSize;
			break;

		case SnesMemoryType::SpcRom:
			*source = _spcRom;
			*cache = &_spcRomCache;
			size = _spcRomSize;
			break;

		case SnesMemoryType::DspProgramRom:
			*source = _necDspProgramRom;
			*cache = &_necDspRomCache;
			size = _necDspProgramRomSize;
			break;

		case SnesMemoryType::Sa1InternalRam:
			*source = _sa1InternalRam;
			*cache = &_sa1InternalRamCache;
			size = _sa1InternalRamSize;
			break;

		case SnesMemoryType::GsuWorkRam:
			*source = _gsuWorkRam;
			*cache = &_gsuWorkRamCache;
			size = _gsuWorkRamSize;
			break;

		default:
			throw std::runtime_error("Disassembler::GetSource() invalid memory type");
	}
}

vector<DisassemblyResult>& Disassembler::GetDisassemblyList(CpuType type)
{
	switch(type) {
		case CpuType::Cpu: return _disassembly;
		case CpuType::Spc: return _spcDisassembly;
		case CpuType::Sa1: return _sa1Disassembly;
		case CpuType::Gsu: return _gsuDisassembly;
		case CpuType::NecDsp: break;
		case CpuType::Cx4: break;
	}
	throw std::runtime_error("Disassembly::GetDisassemblyList(): Invalid cpu type");
}

uint32_t Disassembler::BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type)
{
	uint8_t *source;
	uint32_t sourceLength;
	vector<DisassemblyInfo> *cache;
	GetSource(addrInfo, &source, sourceLength, &cache);

	bool needDisassemble = false;
	int returnSize = 0;
	int32_t address = addrInfo.Address;
	while(address >= 0 && address < (int32_t)cache->size()) {
		DisassemblyInfo &disInfo = (*cache)[address];
		if(!disInfo.IsInitialized() || !disInfo.IsValid(cpuFlags)) {
			disInfo.Initialize(source+address, cpuFlags, type);
			needDisassemble = true;
			returnSize += disInfo.GetOpSize();
		} else {
			returnSize += disInfo.GetOpSize();
			break;
		}

		if(disInfo.UpdateCpuFlags(cpuFlags)) {
			address += disInfo.GetOpSize();
		} else {
			break;
		}
	}

	if(needDisassemble) {
		SetDisassembleFlag(type);
	}

	return returnSize;
}

void Disassembler::SetDisassembleFlag(CpuType type)
{
	if(type == CpuType::Cpu || type == CpuType::Sa1 || type == CpuType::Gsu) {
		_needDisassemble[(int)CpuType::Cpu] = true;
		_needDisassemble[(int)CpuType::Sa1] = true;
		_needDisassemble[(int)CpuType::Gsu] = true;
	} else if(type == CpuType::Spc) {
		_needDisassemble[(int)CpuType::Spc] = true;
	}
}

void Disassembler::ResetPrgCache()
{
	_prgCache = vector<DisassemblyInfo>(_prgRomSize);
	_needDisassemble[(int)CpuType::Cpu] = true;
	_needDisassemble[(int)CpuType::Sa1] = true;
	_needDisassemble[(int)CpuType::Gsu] = true;
}

void Disassembler::InvalidateCache(AddressInfo addrInfo, CpuType type)
{
	uint8_t *source;
	uint32_t sourceLength;
	vector<DisassemblyInfo> *cache;
	GetSource(addrInfo, &source, sourceLength, &cache);
	bool needDisassemble = false;

	if(addrInfo.Address >= 0) {
		for(int i = 0; i < 4; i++) {
			if(addrInfo.Address >= i) {
				if((*cache)[addrInfo.Address - i].IsInitialized()) {
					(*cache)[addrInfo.Address - i].Reset();
					needDisassemble = true;
				}
			}
		}
	}
	
	if(needDisassemble) {
		SetDisassembleFlag(type);
	}
}

void Disassembler::Disassemble(CpuType cpuType)
{
	if(!_needDisassemble[(int)cpuType]) {
		return;
	}

	_needDisassemble[(int)cpuType] = false;

	auto lock = _disassemblyLock.AcquireSafe(); 
	
	bool isSpc = cpuType == CpuType::Spc;
	MemoryMappings *mappings = nullptr;
	switch(cpuType) {
		case CpuType::Cpu: mappings = _memoryManager->GetMemoryMappings(); break;

		case CpuType::Sa1:
			if(!_console->GetCartridge()->GetSa1()) {
				return;
			}
			mappings = _console->GetCartridge()->GetSa1()->GetMemoryMappings(); 
			break;

		case CpuType::Gsu:
			if(!_console->GetCartridge()->GetGsu()) {
				return;
			}
			mappings = _console->GetCartridge()->GetGsu()->GetMemoryMappings();
			break;

		case CpuType::Spc: mappings = nullptr; break;
		default: throw std::runtime_error("Disassemble(): Invalid cpu type");
	}

	vector<DisassemblyResult> &results = GetDisassemblyList(cpuType);
	int32_t maxAddr = isSpc ? 0xFFFF : 0xFFFFFF;
	results.clear();

	uint8_t *source;
	uint32_t sourceLength;
	vector<DisassemblyInfo> *cache;

	bool disUnident = _console->GetSettings()->CheckDebuggerFlag(DebuggerFlags::DisassembleUnidentifiedData);
	bool disData = _console->GetSettings()->CheckDebuggerFlag(DebuggerFlags::DisassembleVerifiedData);
	bool showUnident = _console->GetSettings()->CheckDebuggerFlag(DebuggerFlags::ShowUnidentifiedData);
	bool showData = _console->GetSettings()->CheckDebuggerFlag(DebuggerFlags::ShowVerifiedData);

	bool inUnknownBlock = false;
	bool inVerifiedBlock = false;

	AddressInfo addrInfo = {};
	AddressInfo prevAddrInfo = {};
	string label;
	string comment;
	int byteCounter = 0;
	for(int32_t i = 0; i <= maxAddr; i++) {
		prevAddrInfo = addrInfo;
		addrInfo = isSpc ? _spc->GetAbsoluteAddress(i) : mappings->GetAbsoluteAddress(i);

		if(addrInfo.Address < 0) {
			continue;
		}

		GetSource(addrInfo, &source, sourceLength, &cache);

		DisassemblyInfo disassemblyInfo = (*cache)[addrInfo.Address];
		
		uint8_t opSize = 0;
		uint8_t opCode = (source + addrInfo.Address)[0];
		bool needRealign = true;

		bool isCode = addrInfo.Type == SnesMemoryType::PrgRom ? _cdl->IsCode(addrInfo.Address) : false;
		bool isData = addrInfo.Type == SnesMemoryType::PrgRom ? _cdl->IsData(addrInfo.Address) : false;

		if(disassemblyInfo.IsInitialized()) {
			opSize = disassemblyInfo.GetOpSize();
			needRealign = false;
		} else if((isData && disData) || (!isData && !isCode && disUnident)) {
			opSize = DisassemblyInfo::GetOpSize(opCode, 0, cpuType);
		}

		if(opSize > 0) {
			if(inUnknownBlock || inVerifiedBlock) {
				int flags = LineFlags::BlockEnd | (inVerifiedBlock ? LineFlags::VerifiedData : 0) | (((inVerifiedBlock && showData) || (inUnknownBlock && showUnident)) ? LineFlags::ShowAsData : 0);
				results.push_back(DisassemblyResult(prevAddrInfo, i - 1, flags));
				inUnknownBlock = false;
				inVerifiedBlock = false;
			}
			byteCounter = 0;

			if(addrInfo.Type == SnesMemoryType::PrgRom && _cdl->IsSubEntryPoint(addrInfo.Address)) {
				results.push_back(DisassemblyResult(addrInfo, i, LineFlags::SubStart | LineFlags::BlockStart | LineFlags::VerifiedCode));
			}

			_labelManager->GetLabelAndComment(addrInfo, label, comment);

			bool hasMultipleComment = comment.find_first_of('\n') != string::npos;
			if(hasMultipleComment) {
				int16_t lineCount = 0;
				for(char c : comment) {
					if(c == '\n') {
						results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Comment, lineCount));
						lineCount++;
					}
				}
				results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Comment, lineCount));
			} 
			
			if(label.size()) {
				results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Label));
			}

			if(!hasMultipleComment && comment.size()) {
				results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Comment));
			} else {
				results.push_back(DisassemblyResult(addrInfo, i));
			}

			if(needRealign) {
				for(int j = 1, max = (int)(*cache).size(); j < opSize && addrInfo.Address + j < max; j++) {
					if((*cache)[addrInfo.Address + j].IsInitialized()) {
						break;
					}
					i++;
				}
			} else {
				i += opSize - 1;
			}

			if(DisassemblyInfo::IsReturnInstruction(opCode, cpuType)) {
				//End of function
				results.push_back(DisassemblyResult(-1, LineFlags::VerifiedCode | LineFlags::BlockEnd));
			} 
		} else {
			if(showData || showUnident) {
				if((isData && inUnknownBlock) || (!isData && inVerifiedBlock)) {
					if(isData && inUnknownBlock) {
						//In an unknown block and the next byte is data, end the block
						results.push_back(DisassemblyResult(addrInfo, i, LineFlags::BlockEnd | (showUnident ? LineFlags::ShowAsData : 0)));
					} else if(!isData && inVerifiedBlock) {
						//In a verified data block and the next byte is unknown, end the block
						results.push_back(DisassemblyResult(addrInfo, i, LineFlags::BlockEnd | LineFlags::VerifiedData | (showData ? LineFlags::ShowAsData : 0)));
					}
					inUnknownBlock = false;
					inVerifiedBlock = false;
					byteCounter = 0;
				}
			}

			if(byteCounter > 0) {
				//If showing as hex data, add a new data line every 8 bytes
				byteCounter--;
				if(byteCounter == 0) {
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::ShowAsData | (isData ? LineFlags::VerifiedData : 0)));
					byteCounter = 8;
				}
			} else if(!inUnknownBlock && !inVerifiedBlock) {
				//If not in a block, start a new block based on the current byte's type (data vs unidentified)
				bool showAsData = (isData && showData) || ((!isData && !isCode) && showUnident);
				if(isData) {
					inVerifiedBlock = true;
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::BlockStart | LineFlags::VerifiedData | (showAsData ? LineFlags::ShowAsData : 0)));
				} else {
					inUnknownBlock = true;
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::BlockStart | (showAsData ? LineFlags::ShowAsData : 0)));
				}

				if(showAsData) {
					//If showing data as hex, add the first row of the block
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::ShowAsData | (isData ? LineFlags::VerifiedData : 0)));
					byteCounter = 8;
				} else {
					//If not showing the data at all, display 1 empty line
					results.push_back(DisassemblyResult(-1, LineFlags::None | (isData ? LineFlags::VerifiedData : 0)));
				}
			}
		}
	}

	if(inUnknownBlock || inVerifiedBlock) {
		int flags = LineFlags::BlockEnd | (inVerifiedBlock ? LineFlags::VerifiedData : 0) | (((inVerifiedBlock && showData) || (inUnknownBlock && showUnident)) ? LineFlags::ShowAsData : 0);
		results.push_back(DisassemblyResult(addrInfo, maxAddr, flags));
	}
}

DisassemblyInfo Disassembler::GetDisassemblyInfo(AddressInfo &info)
{
	DisassemblyInfo disassemblyInfo;
	switch(info.Type) {
		default: break;
		case SnesMemoryType::PrgRom: disassemblyInfo = _prgCache[info.Address]; break;
		case SnesMemoryType::WorkRam: disassemblyInfo = _wramCache[info.Address]; break;
		case SnesMemoryType::SaveRam: disassemblyInfo = _sramCache[info.Address]; break;
		case SnesMemoryType::SpcRam: disassemblyInfo = _spcRamCache[info.Address]; break;
		case SnesMemoryType::SpcRom: disassemblyInfo = _spcRomCache[info.Address]; break;
		case SnesMemoryType::DspProgramRom: disassemblyInfo = _necDspRomCache[info.Address]; break;
		case SnesMemoryType::Sa1InternalRam: disassemblyInfo = _sa1InternalRamCache[info.Address]; break;
	}

	if(disassemblyInfo.IsInitialized()) {
		return disassemblyInfo;
	} else {
		return DisassemblyInfo();
	}
}

void Disassembler::RefreshDisassembly(CpuType type)
{
	DebugBreakHelper helper(_debugger);
	_needDisassemble[(int)type] = true;
	Disassemble(type);
}

uint32_t Disassembler::GetLineCount(CpuType type)
{
	auto lock = _disassemblyLock.AcquireSafe();
	vector<DisassemblyResult> &source = GetDisassemblyList(type);
	return (uint32_t)source.size();
}

uint32_t Disassembler::GetLineIndex(CpuType type, uint32_t cpuAddress)
{
	auto lock = _disassemblyLock.AcquireSafe();
	vector<DisassemblyResult> &source = GetDisassemblyList(type);
	uint32_t lastAddress = 0;
	for(size_t i = 1; i < source.size(); i++) {
		if(source[i].CpuAddress < 0 || (source[i].Flags & LineFlags::SubStart) | (source[i].Flags & LineFlags::Label) || ((source[i].Flags & LineFlags::Comment) && source[i].CommentLine >= 0)) {
			continue;
		}

		if(cpuAddress == (uint32_t)source[i].CpuAddress) {
			return (uint32_t)i;
		} else if(cpuAddress >= lastAddress && cpuAddress < (uint32_t)source[i].CpuAddress) {
			return (uint32_t)i - 1;
		}

		lastAddress = source[i].CpuAddress;
	}
	return 0;
}

bool Disassembler::GetLineData(CpuType type, uint32_t lineIndex, CodeLineData &data)
{
	auto lock =_disassemblyLock.AcquireSafe();

	vector<DisassemblyResult> &source = GetDisassemblyList(type);
	SnesMemoryType memType = DebugUtilities::GetCpuMemoryType(type);
	int32_t maxAddr = type == CpuType::Spc ? 0xFFFF : 0xFFFFFF;
	if(lineIndex < source.size()) {
		DisassemblyResult result = source[lineIndex];
		data.Address = -1;
		data.AbsoluteAddress = -1;
		data.Flags = result.Flags;

		switch(result.Address.Type) {
			default: break;
			case SnesMemoryType::PrgRom: data.Flags |= (uint8_t)LineFlags::PrgRom; break;
			case SnesMemoryType::WorkRam: data.Flags |= (uint8_t)LineFlags::WorkRam; break;
			case SnesMemoryType::SaveRam: data.Flags |= (uint8_t)LineFlags::SaveRam; break;
		}

		bool isBlockStartEnd = (data.Flags & (LineFlags::BlockStart | LineFlags::BlockEnd)) != 0;
		if(!isBlockStartEnd && result.Address.Address >= 0) {
			if((data.Flags & LineFlags::ShowAsData)) {
				FastString str(".db", 3);
				int nextAddr = lineIndex < source.size() - 2 ? source[lineIndex+1].CpuAddress : (maxAddr + 1);
				for(int i = 0; i < 8 && result.CpuAddress+i < nextAddr; i++) {
					str.Write(" $", 2);
					str.Write(HexUtilities::ToHexChar(_memoryDumper->GetMemoryValue(memType, result.CpuAddress + i)), 2);
				}
				data.Address = result.CpuAddress;
				data.AbsoluteAddress = result.Address.Address;
				memcpy(data.Text, str.ToString(), str.GetSize());
			} else if((data.Flags & LineFlags::Comment) && result.CommentLine >= 0) {
				string comment = ";" + StringUtilities::Split(_labelManager->GetComment(result.Address), '\n')[result.CommentLine];
				data.Flags |= LineFlags::VerifiedCode;
				memcpy(data.Comment, comment.c_str(), std::min<int>((int)comment.size(), 1000));
			} else if(data.Flags & LineFlags::Label) {
				string label = _labelManager->GetLabel(result.Address) + ":";
				data.Flags |= LineFlags::VerifiedCode;
				memcpy(data.Text, label.c_str(), std::min<int>((int)label.size(), 1000));
			} else {
				uint8_t *source;
				uint32_t sourceLength;
				vector<DisassemblyInfo> *cache;
				GetSource(result.Address, &source, sourceLength, &cache);
				DisassemblyInfo disInfo = (*cache)[result.Address.Address];
				CpuType lineCpuType = disInfo.IsInitialized() ? disInfo.GetCpuType() : type;

				data.Address = result.CpuAddress;
				data.AbsoluteAddress = result.Address.Address;

				switch(lineCpuType) {
					case CpuType::Cpu:
					case CpuType::Sa1: {
						CpuState state = _console->GetCpu()->GetState();
						state.PC = (uint16_t)result.CpuAddress;
						state.K = (result.CpuAddress >> 16);

						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(source + result.Address.Address, state.PS, CpuType::Cpu);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_console, &state);

						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
						} else {
							data.ValueSize = 0;
						}
						break;
					}
					
					case CpuType::Spc: {
						SpcState state = _console->GetSpc()->GetState();
						state.PC = (uint16_t)result.CpuAddress;

						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(source + result.Address.Address, 0, CpuType::Spc);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_console, &state);
						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
							data.ValueSize = 1;
						} else {
							data.ValueSize = 0;
						}
						break;
					}

					case CpuType::Gsu: {
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(source + result.Address.Address, 0, CpuType::Gsu);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = -1;
						data.ValueSize = 0;
						break;
					}

					case CpuType::Cx4:
					case CpuType::NecDsp:
						throw std::runtime_error("GetLineData - CPU type not supported");
				}

				string text;
				disInfo.GetDisassembly(text, result.CpuAddress, _labelManager.get());
				memcpy(data.Text, text.c_str(), std::min<int>((int)text.size(), 1000));

				disInfo.GetByteCode(data.ByteCode);

				if(data.Flags & LineFlags::Comment) {
					string comment = ";" + _labelManager->GetComment(result.Address);
					memcpy(data.Comment, comment.c_str(), std::min<int>((int)comment.size(), 1000));
				} else {
					data.Comment[0] = 0;
				}
			}
		} else {
			if(data.Flags & LineFlags::SubStart) {
				string label = _labelManager->GetLabel(result.Address);
				if(label.empty()) {
					label = "sub start";
				}
				memcpy(data.Text, label.c_str(), label.size() + 1);
			} else if(data.Flags & LineFlags::BlockStart) {
				string label = (data.Flags & LineFlags::VerifiedData) ? "data" : "unidentified";
				memcpy(data.Text, label.c_str(), label.size() + 1);
			}

			if(data.Flags & (LineFlags::BlockStart | LineFlags::BlockEnd)) {
				if(!(data.Flags & LineFlags::ShowAsData)) {
					//For hidden blocks, give the start/end lines an address
					data.Address = result.CpuAddress;
					data.AbsoluteAddress = result.Address.Address;
				}
			}
		}
		return true;
	}
	return false;
}

int32_t Disassembler::SearchDisassembly(CpuType type, const char *searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards)
{
	auto lock = _disassemblyLock.AcquireSafe();
	vector<DisassemblyResult> &source = GetDisassemblyList(type);
	int step = searchBackwards ? -1 : 1;
	CodeLineData lineData = {};
	for(int i = startPosition; i != endPosition; i += step) {
		GetLineData(type, i, lineData);
		string line = lineData.Text;
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);

		if(line.find(searchString) != string::npos) {
			return i;
		}

		//Continue search from start/end of document
		if(!searchBackwards && i == (int)(source.size() - 1)) {
			i = 0;
		} else if(searchBackwards && i == 0) {
			i = (int32_t)(source.size() - 1);
		}
	}

	return -1;
}
