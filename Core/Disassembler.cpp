#include "stdafx.h"
#include <algorithm>
#include "Disassembler.h"
#include "DisassemblyInfo.h"
#include "Cpu.h"
#include "Spc.h"
#include "NecDsp.h"
#include "Sa1.h"
#include "Gsu.h"
#include "Cx4.h"
#include "BsxCart.h"
#include "BsxMemoryPack.h"
#include "Gameboy.h"
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
	shared_ptr<BaseCartridge> cart = console->GetCartridge();

	_cdl = cdl;
	_debugger = debugger;
	_labelManager = debugger->GetLabelManager();
	_console = console.get();
	_cpu = console->GetCpu().get();
	_spc = console->GetSpc().get();
	_gsu = cart->GetGsu();
	_sa1 = cart->GetSa1();
	_gameboy = cart->GetGameboy();
	_settings = console->GetSettings().get();
	_memoryDumper = _debugger->GetMemoryDumper().get();
	_memoryManager = console->GetMemoryManager().get();

	_prgRom = console->GetCartridge()->DebugGetPrgRom();
	_prgRomSize = console->GetCartridge()->DebugGetPrgRomSize();
	_sram = console->GetCartridge()->DebugGetSaveRam();
	_sramSize = console->GetCartridge()->DebugGetSaveRamSize();
	_wram = _memoryManager->DebugGetWorkRam();
	_wramSize = MemoryManager::WorkRamSize;

	_spcRam = _spc->GetSpcRam();
	_spcRamSize = Spc::SpcRamSize;
	_spcRom = _spc->GetSpcRom();
	_spcRomSize = Spc::SpcRomSize;

	_necDspProgramRom = cart->GetDsp() ? cart->GetDsp()->DebugGetProgramRom() : nullptr;
	_necDspProgramRomSize = cart->GetDsp() ? cart->GetDsp()->DebugGetProgramRomSize() : 0;

	_sa1InternalRam = _sa1 ? _sa1->DebugGetInternalRam() : nullptr;
	_sa1InternalRamSize = _sa1 ? _sa1->DebugGetInternalRamSize() : 0;

	_gsuWorkRam = _gsu ? _gsu->DebugGetWorkRam() : nullptr;
	_gsuWorkRamSize = _gsu ? _gsu->DebugGetWorkRamSize() : 0;

	_bsxPsRam = cart->GetBsx() ? cart->GetBsx()->DebugGetPsRam() : nullptr;
	_bsxPsRamSize = cart->GetBsx() ? cart->GetBsx()->DebugGetPsRamSize() : 0;
	_bsxMemPack = cart->GetBsx() ? cart->GetBsxMemoryPack()->DebugGetMemoryPack() : nullptr;
	_bsxMemPackSize = cart->GetBsx() ? cart->GetBsxMemoryPack()->DebugGetMemoryPackSize() : 0;

	_gbPrgRom = _gameboy ? _gameboy->DebugGetMemory(SnesMemoryType::GbPrgRom) : nullptr;
	_gbPrgRomSize = _gameboy ? _gameboy->DebugGetMemorySize(SnesMemoryType::GbPrgRom) : 0;
	_gbWorkRam = _gameboy ? _gameboy->DebugGetMemory(SnesMemoryType::GbWorkRam) : nullptr;
	_gbWorkRamSize = _gameboy ? _gameboy->DebugGetMemorySize(SnesMemoryType::GbWorkRam) : 0;
	_gbCartRam = _gameboy ? _gameboy->DebugGetMemory(SnesMemoryType::GbCartRam) : nullptr;
	_gbCartRamSize = _gameboy ? _gameboy->DebugGetMemorySize(SnesMemoryType::GbCartRam) : 0;
	_gbHighRam = _gameboy ? _gameboy->DebugGetMemory(SnesMemoryType::GbHighRam) : nullptr;
	_gbHighRamSize = _gameboy ? _gameboy->DebugGetMemorySize(SnesMemoryType::GbHighRam) : 0;
	_gbBootRom = _gameboy ? _gameboy->DebugGetMemory(SnesMemoryType::GbBootRom) : nullptr;
	_gbBootRomSize = _gameboy ? _gameboy->DebugGetMemorySize(SnesMemoryType::GbBootRom) : 0;

	_prgCache = vector<DisassemblyInfo>(_prgRomSize);
	_sramCache = vector<DisassemblyInfo>(_sramSize);
	_wramCache = vector<DisassemblyInfo>(_wramSize);
	_spcRamCache = vector<DisassemblyInfo>(_spcRamSize);
	_spcRomCache = vector<DisassemblyInfo>(_spcRomSize);
	_necDspRomCache = vector<DisassemblyInfo>(_necDspProgramRomSize);
	_sa1InternalRamCache = vector<DisassemblyInfo>(_sa1InternalRamSize);
	_gsuWorkRamCache = vector<DisassemblyInfo>(_gsuWorkRamSize);
	_bsxPsRamCache = vector<DisassemblyInfo>(_bsxPsRamSize);
	_bsxMemPackCache = vector<DisassemblyInfo>(_bsxMemPackSize);
	
	_gbPrgCache = vector<DisassemblyInfo>(_gbPrgRomSize);
	_gbWorkRamCache = vector<DisassemblyInfo>(_gbWorkRamSize);
	_gbCartRamCache = vector<DisassemblyInfo>(_gbCartRamSize);
	_gbHighRamCache = vector<DisassemblyInfo>(_gbHighRamSize);
	_gbBootRomCache = vector<DisassemblyInfo>(_gbBootRomSize);

	for(int i = 0; i < (int)DebugUtilities::GetLastCpuType(); i++) {
		_needDisassemble[i] = true;
	}

	_sources[(int)SnesMemoryType::PrgRom] = { _prgRom, &_prgCache, _prgRomSize };
	_sources[(int)SnesMemoryType::WorkRam] = { _wram, &_wramCache, _wramSize };
	_sources[(int)SnesMemoryType::SaveRam] = { _sram, &_sramCache, _sramSize };
	_sources[(int)SnesMemoryType::SpcRam] = { _spcRam, &_spcRamCache, _spcRamSize };
	_sources[(int)SnesMemoryType::SpcRom] = { _spcRom, &_spcRomCache, _spcRomSize };
	_sources[(int)SnesMemoryType::DspProgramRom] = { _necDspProgramRom, &_necDspRomCache, _necDspProgramRomSize };
	_sources[(int)SnesMemoryType::Sa1InternalRam] = { _sa1InternalRam, &_sa1InternalRamCache, _sa1InternalRamSize };
	_sources[(int)SnesMemoryType::GsuWorkRam] = { _gsuWorkRam, &_gsuWorkRamCache, _gsuWorkRamSize };
	_sources[(int)SnesMemoryType::BsxPsRam] = { _bsxPsRam, &_bsxPsRamCache, _bsxPsRamSize };
	_sources[(int)SnesMemoryType::BsxMemoryPack] = { _bsxMemPack, &_bsxMemPackCache, _bsxMemPackSize };
	
	_sources[(int)SnesMemoryType::GbPrgRom] = { _gbPrgRom, &_gbPrgCache, _gbPrgRomSize };
	_sources[(int)SnesMemoryType::GbWorkRam] = { _gbWorkRam, &_gbWorkRamCache, _gbWorkRamSize };
	_sources[(int)SnesMemoryType::GbCartRam] = { _gbCartRam, &_gbCartRamCache, _gbCartRamSize };
	_sources[(int)SnesMemoryType::GbHighRam] = { _gbHighRam, &_gbHighRamCache, _gbHighRamSize };
	_sources[(int)SnesMemoryType::GbBootRom] = { _gbBootRom, &_gbBootRomCache, _gbBootRomSize };

	if(_necDspProgramRomSize > 0) {
		//Build cache for the entire DSP chip (since it only contains instructions)
		AddressInfo dspStart = { 0, SnesMemoryType::DspProgramRom };
		BuildCache(dspStart, 0, CpuType::NecDsp);
	}
}

DisassemblerSource Disassembler::GetSource(SnesMemoryType type)
{
	if(_sources[(int)type].Data == nullptr) {
		throw std::runtime_error("Disassembler::GetSource() invalid memory type");
	}

	return _sources[(int)type];
}

vector<DisassemblyResult>& Disassembler::GetDisassemblyList(CpuType type)
{
	switch(type) {
		case CpuType::Cpu: return _disassembly;
		case CpuType::Spc: return _spcDisassembly;
		case CpuType::NecDsp: return _necDspDisassembly;
		case CpuType::Sa1: return _sa1Disassembly;
		case CpuType::Gsu: return _gsuDisassembly;
		case CpuType::Cx4: return _cx4Disassembly;
		case CpuType::Gameboy: return _gbDisassembly;
	}
	throw std::runtime_error("Disassembly::GetDisassemblyList(): Invalid cpu type");
}

uint32_t Disassembler::BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type)
{
	DisassemblerSource src = GetSource(addrInfo.Type);

	bool needDisassemble = false;
	int returnSize = 0;
	int32_t address = addrInfo.Address;
	while(address >= 0 && address < (int32_t)src.Cache->size()) {
		DisassemblyInfo &disInfo = (*src.Cache)[address];
		if(!disInfo.IsInitialized() || !disInfo.IsValid(cpuFlags)) {
			disInfo.Initialize(src.Data+address, cpuFlags, type);
			for(int i = 1; i < disInfo.GetOpSize(); i++) {
				//Clear any instructions that start in the middle of this one
				//(can happen when resizing an instruction after X/M updates)
				(*src.Cache)[address + i] = DisassemblyInfo();
			}
			needDisassemble = true;
			returnSize += disInfo.GetOpSize();
		} else {
			returnSize += disInfo.GetOpSize();
			break;
		}

		if(disInfo.IsUnconditionalJump()) {
			//Can't assume what follows is code, stop disassembling
			break;
		}

		disInfo.UpdateCpuFlags(cpuFlags);
		address += disInfo.GetOpSize();
	}

	if(needDisassemble) {
		SetDisassembleFlag(type);
	}

	return returnSize;
}

void Disassembler::SetDisassembleFlag(CpuType type)
{
	if(type == CpuType::Cpu || type == CpuType::Sa1 || type == CpuType::Gsu || type == CpuType::Cx4) {
		_needDisassemble[(int)CpuType::Cpu] = true;
		_needDisassemble[(int)CpuType::Sa1] = true;
		_needDisassemble[(int)CpuType::Gsu] = true;
		_needDisassemble[(int)CpuType::Cx4] = true;
	} else {
		_needDisassemble[(int)type] = true;
	}
}

void Disassembler::ResetPrgCache()
{
	_prgCache = vector<DisassemblyInfo>(_prgRomSize);
	_gbPrgCache = vector<DisassemblyInfo>(_gbPrgRomSize);
	_needDisassemble[(int)CpuType::Cpu] = true;
	_needDisassemble[(int)CpuType::Sa1] = true;
	_needDisassemble[(int)CpuType::Gsu] = true;
	_needDisassemble[(int)CpuType::Cx4] = true;
	_needDisassemble[(int)CpuType::Gameboy] = true;
}

void Disassembler::InvalidateCache(AddressInfo addrInfo, CpuType type)
{
	DisassemblerSource src = GetSource(addrInfo.Type);
	bool needDisassemble = false;

	if(addrInfo.Address >= 0) {
		for(int i = 0; i < 4; i++) {
			if(addrInfo.Address >= i) {
				if((*src.Cache)[addrInfo.Address - i].IsInitialized()) {
					(*src.Cache)[addrInfo.Address - i].Reset();
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
	bool isGb = cpuType == CpuType::Gameboy;
	bool isDsp = cpuType == CpuType::NecDsp;
	MemoryMappings *mappings = nullptr;
	int32_t maxAddr = 0xFFFFFF;
	switch(cpuType) {
		case CpuType::Cpu: 
			mappings = _memoryManager->GetMemoryMappings();
			break;

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

		case CpuType::NecDsp:
			if(!_console->GetCartridge()->GetDsp()) {
				return;
			}
			mappings = nullptr;
			maxAddr = _necDspProgramRomSize - 1;
			break;

		case CpuType::Gameboy:
		case CpuType::Spc:
			if(!_gameboy) {
				return;
			}
			mappings = nullptr; 
			maxAddr = 0xFFFF;
			break;

		case CpuType::Cx4:
			if(!_console->GetCartridge()->GetCx4()) {
				return;
			}
			mappings = _console->GetCartridge()->GetCx4()->GetMemoryMappings();
			break;

		default: throw std::runtime_error("Disassemble(): Invalid cpu type");
	}

	vector<DisassemblyResult> &results = GetDisassemblyList(cpuType);
	results.clear();

	bool disUnident = _settings->CheckDebuggerFlag(DebuggerFlags::DisassembleUnidentifiedData);
	bool disData = _settings->CheckDebuggerFlag(DebuggerFlags::DisassembleVerifiedData);
	bool showUnident = _settings->CheckDebuggerFlag(DebuggerFlags::ShowUnidentifiedData);
	bool showData = _settings->CheckDebuggerFlag(DebuggerFlags::ShowVerifiedData);

	bool inUnknownBlock = false;
	bool inVerifiedBlock = false;
	LabelInfo labelInfo;
	AddressInfo addrInfo = {};
	AddressInfo prevAddrInfo = {};
	int byteCounter = 0;
	for(int32_t i = 0; i <= maxAddr; i++) {
		prevAddrInfo = addrInfo;
		if(isDsp) {
			addrInfo = { i, SnesMemoryType::DspProgramRom };
		} else {
			if(isGb) {
				addrInfo = _gameboy->GetAbsoluteAddress(i);
			} else if(isSpc) {
				addrInfo = _spc->GetAbsoluteAddress(i);
			} else {
				addrInfo = mappings->GetAbsoluteAddress(i);
			}
		}

		if(addrInfo.Address < 0) {
			continue;
		}

		DisassemblerSource src = GetSource(addrInfo.Type);

		DisassemblyInfo disassemblyInfo = (*src.Cache)[addrInfo.Address];
		
		uint8_t opSize = 0;
		uint8_t opCode = (src.Data + addrInfo.Address)[0];

		bool isCode = addrInfo.Type == SnesMemoryType::PrgRom ? _cdl->IsCode(addrInfo.Address) : false;
		bool isData = addrInfo.Type == SnesMemoryType::PrgRom ? _cdl->IsData(addrInfo.Address) : false;

		if(disassemblyInfo.IsInitialized()) {
			opSize = disassemblyInfo.GetOpSize();
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

			if(_labelManager->GetLabelAndComment(addrInfo, labelInfo)) {
				bool hasMultipleComment = labelInfo.Comment.find_first_of('\n') != string::npos;
				if(hasMultipleComment) {
					int16_t lineCount = 0;
					for(char c : labelInfo.Comment) {
						if(c == '\n') {
							results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Comment, lineCount));
							lineCount++;
						}
					}
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Comment, lineCount));
				}

				if(labelInfo.Label.size()) {
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Label));
				}

				if(!hasMultipleComment && labelInfo.Comment.size()) {
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Comment));
				} else {
					results.push_back(DisassemblyResult(addrInfo, i));
				}
			} else {
				results.push_back(DisassemblyResult(addrInfo, i));
			}

			//Move to the end of the instruction (but realign disassembly if another valid instruction is found)
			//This can sometimes happen if the 2nd byte of BRK/COP is reused as the first byte of the next instruction
			//Also required when disassembling unvalidated data as code (to realign once we find verified code)
			for(int j = 1, max = (int)(*src.Cache).size(); j < opSize && addrInfo.Address + j < max; j++) {
				if((*src.Cache)[addrInfo.Address + j].IsInitialized()) {
					break;
				}
				i++;
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
						results.push_back(DisassemblyResult(prevAddrInfo, i - 1, LineFlags::BlockEnd | (showUnident ? LineFlags::ShowAsData : 0)));
					} else if(!isData && inVerifiedBlock) {
						//In a verified data block and the next byte is unknown, end the block
						results.push_back(DisassemblyResult(prevAddrInfo, i - 1, LineFlags::BlockEnd | LineFlags::VerifiedData | (showData ? LineFlags::ShowAsData : 0)));
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

DisassemblyInfo Disassembler::GetDisassemblyInfo(AddressInfo &info, uint32_t cpuAddress, uint8_t cpuFlags, CpuType type)
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
		case SnesMemoryType::GbPrgRom: disassemblyInfo = _gbPrgCache[info.Address]; break;
		case SnesMemoryType::GbWorkRam: disassemblyInfo = _gbWorkRamCache[info.Address]; break;
		case SnesMemoryType::GbCartRam: disassemblyInfo = _gbCartRamCache[info.Address]; break;
	}

	if(!disassemblyInfo.IsInitialized()) {
		disassemblyInfo.Initialize(cpuAddress, cpuFlags, type, _memoryDumper);
	}
	return disassemblyInfo;
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
		data.EffectiveAddress = -1;
		data.Flags = result.Flags;

		switch(result.Address.Type) {
			default: break;
			case SnesMemoryType::GbPrgRom:
			case SnesMemoryType::PrgRom: data.Flags |= (uint8_t)LineFlags::PrgRom; break;

			case SnesMemoryType::GbWorkRam:
			case SnesMemoryType::WorkRam: data.Flags |= (uint8_t)LineFlags::WorkRam; break;
			
			case SnesMemoryType::GbCartRam:
			case SnesMemoryType::SaveRam: data.Flags |= (uint8_t)LineFlags::SaveRam; break;
		}

		bool isBlockStartEnd = (data.Flags & (LineFlags::BlockStart | LineFlags::BlockEnd)) != 0;
		if(!isBlockStartEnd && result.Address.Address >= 0) {
			if((data.Flags & LineFlags::ShowAsData)) {
				FastString str(".db", 3);
				int nextAddr = lineIndex < source.size() - 2 ? (source[lineIndex+1].CpuAddress + 1) : (maxAddr + 1);
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
				DisassemblerSource src = GetSource(result.Address.Type);
				DisassemblyInfo disInfo = (*src.Cache)[result.Address.Address];
				CpuType lineCpuType = disInfo.IsInitialized() ? disInfo.GetCpuType() : type;

				data.Address = result.CpuAddress;
				data.AbsoluteAddress = result.Address.Address;

				switch(lineCpuType) {
					case CpuType::Cpu:
					case CpuType::Sa1: {
						CpuState state = type == CpuType::Sa1 ? _sa1->GetCpuState() : _cpu->GetState();
						state.PC = (uint16_t)result.CpuAddress;
						state.K = (result.CpuAddress >> 16);

						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, state.PS, lineCpuType);
						} else {
							data.Flags |= (result.Address.Type != SnesMemoryType::PrgRom || _cdl->IsCode(data.AbsoluteAddress)) ? LineFlags::VerifiedCode : LineFlags::UnexecutedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_console, &state, lineCpuType);

						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
						} else {
							data.ValueSize = 0;
						}
						break;
					}
					
					case CpuType::Spc: {
						SpcState state = _spc->GetState();
						state.PC = (uint16_t)result.CpuAddress;

						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Spc);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_console, &state, lineCpuType);
						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
							data.ValueSize = 1;
						} else {
							data.ValueSize = 0;
						}
						break;
					}

					case CpuType::Gsu: {
						GsuState state = _gsu->GetState();
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Gsu);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_console, &state, lineCpuType);
						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
							data.ValueSize = 2;
						} else {
							data.ValueSize = 0;
						}
						break;
					}
					
					case CpuType::NecDsp:
					case CpuType::Cx4:
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, type);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = -1;
						data.ValueSize = 0;
						break;

					case CpuType::Gameboy:
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Gameboy);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = -1;
						data.ValueSize = 0;
						break;
				}

				string text;
				disInfo.GetDisassembly(text, result.CpuAddress, _labelManager.get(), _settings);
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
				if(!(data.Flags & (LineFlags::ShowAsData | LineFlags::SubStart))) {
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
