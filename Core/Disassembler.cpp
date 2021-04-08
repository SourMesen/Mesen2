#include "stdafx.h"
#include <algorithm>
#include "Disassembler.h"
#include "DisassemblyInfo.h"
#include "Debugger.h"
#include "LabelManager.h"
#include "MemoryDumper.h"
#include "CodeDataLogger.h"
#include "DebugBreakHelper.h"
#include "EmuSettings.h"
#include "DebugUtilities.h"
#include "SNES/CpuTypes.h"
#include "SNES/SpcTypes.h"
#include "SNES/GsuTypes.h"
#include "Gameboy/GbTypes.h"
#include "NES/NesTypes.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"

Disassembler::Disassembler(IConsole* console, Debugger* debugger)
{
	_debugger = debugger;
	_labelManager = debugger->GetLabelManager();
	_console = console;
	_settings = debugger->GetEmulator()->GetSettings().get();
	_memoryDumper = _debugger->GetMemoryDumper();

	for(int i = 0; i < (int)DebugUtilities::GetLastCpuType(); i++) {
		_disassemblyResult[i] = vector<DisassemblyResult>();
		_needDisassemble[i] = true;
	}

	for(int i = (int)SnesMemoryType::PrgRom; i < (int)SnesMemoryType::Register; i++) {
		InitSource((SnesMemoryType)i);
	}

	//TODO
	/*if(_necDsp) {
		//Build cache for the entire DSP chip (since it only contains instructions)
		AddressInfo dspStart = { 0, SnesMemoryType::DspProgramRom };
		BuildCache(dspStart, 0, CpuType::NecDsp);
	}*/
}

void Disassembler::InitSource(SnesMemoryType type)
{
	uint8_t* src = _memoryDumper->GetMemoryBuffer(type);
	uint32_t size = _memoryDumper->GetMemorySize(type);
	_disassemblyCache[(int)type] = vector<DisassemblyInfo>(size);
	_sources[(int)type] = { src, &_disassemblyCache[(int)type], size };
}

DisassemblerSource& Disassembler::GetSource(SnesMemoryType type)
{
	if(_sources[(int)type].Data == nullptr) {
		throw std::runtime_error("Disassembler::GetSource() invalid memory type");
	}

	return _sources[(int)type];
}

uint32_t Disassembler::BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type)
{
	DisassemblerSource& src = GetSource(addrInfo.Type);

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
	InitSource(SnesMemoryType::PrgRom);
	InitSource(SnesMemoryType::GbPrgRom);
	_needDisassemble[(int)CpuType::Cpu] = true;
	_needDisassemble[(int)CpuType::Sa1] = true;
	_needDisassemble[(int)CpuType::Gsu] = true;
	_needDisassemble[(int)CpuType::Cx4] = true;
	_needDisassemble[(int)CpuType::Gameboy] = true;
	_needDisassemble[(int)CpuType::Nes] = true;
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
	
	vector<DisassemblyResult> &results = _disassemblyResult[(int)cpuType];
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
	
	AddressInfo absAddress = {};
	absAddress.Type = DebugUtilities::GetCpuMemoryType(cpuType);;
	int32_t maxAddr = _memoryDumper->GetMemorySize(absAddress.Type) - 1;
	CodeDataLogger* cdl = nullptr;
	if(cpuType == CpuType::Cpu || cpuType == CpuType::Gameboy) {
		cdl = _debugger->GetCodeDataLogger(cpuType).get();
	}

	for(int32_t i = 0; i <= maxAddr; i++) {
		prevAddrInfo = addrInfo;
		
		absAddress.Address = i;
		addrInfo = _debugger->GetAbsoluteAddress(absAddress);
		if(addrInfo.Address < 0 || addrInfo.Type == SnesMemoryType::Register) {
			continue;
		}

		DisassemblerSource src = GetSource(addrInfo.Type);

		DisassemblyInfo disassemblyInfo = (*src.Cache)[addrInfo.Address];
		
		uint8_t opSize = 0;
		uint8_t opCode = (src.Data + addrInfo.Address)[0];

		bool isCode = addrInfo.Type == SnesMemoryType::PrgRom ? cdl->IsCode(addrInfo.Address) : false;
		bool isData = addrInfo.Type == SnesMemoryType::PrgRom ? cdl->IsData(addrInfo.Address) : false;

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

			if(addrInfo.Type == SnesMemoryType::PrgRom && cdl->IsSubEntryPoint(addrInfo.Address)) {
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
	DisassemblyInfo disassemblyInfo = (*GetSource(info.Type).Cache)[info.Address];
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
	vector<DisassemblyResult>& source = _disassemblyResult[(int)type];
	return (uint32_t)source.size();
}

uint32_t Disassembler::GetLineIndex(CpuType type, uint32_t cpuAddress)
{
	auto lock = _disassemblyLock.AcquireSafe();
	vector<DisassemblyResult>& source = _disassemblyResult[(int)type];
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

	vector<DisassemblyResult>& source = _disassemblyResult[(int)type];
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
						CpuState state = (CpuState&)_debugger->GetStateRef(lineCpuType);
						state.PC = (uint16_t)result.CpuAddress;
						state.K = (result.CpuAddress >> 16);

						CodeDataLogger* cdl = _debugger->GetCodeDataLogger(lineCpuType).get();
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, state.PS, lineCpuType);
						} else {
							data.Flags |= (result.Address.Type != SnesMemoryType::PrgRom || cdl->IsCode(data.AbsoluteAddress)) ? LineFlags::VerifiedCode : LineFlags::UnexecutedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);

						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
						} else {
							data.ValueSize = 0;
						}
						break;
					}
					
					case CpuType::Spc: {
						SpcState state = (SpcState&)_debugger->GetStateRef(lineCpuType);
						state.PC = (uint16_t)result.CpuAddress;

						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Spc);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);
						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
							data.ValueSize = 1;
						} else {
							data.ValueSize = 0;
						}
						break;
					}

					case CpuType::Gsu: {
						GsuState state = (GsuState&)_debugger->GetStateRef(lineCpuType);
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Gsu);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);
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

					case CpuType::Gameboy: {
						GbCpuState state = (GbCpuState&)_debugger->GetStateRef(lineCpuType);
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Gameboy);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);
						data.ValueSize = 0;
						break;
					}

					case CpuType::Nes: {
						NesCpuState state = (NesCpuState&)_debugger->GetStateRef(lineCpuType);
						if(!disInfo.IsInitialized()) {
							disInfo = DisassemblyInfo(src.Data + result.Address.Address, 0, CpuType::Nes);
						} else {
							data.Flags |= LineFlags::VerifiedCode;
						}

						data.OpSize = disInfo.GetOpSize();
						data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);
						if(data.EffectiveAddress >= 0) {
							data.Value = disInfo.GetMemoryValue(data.EffectiveAddress, _memoryDumper, memType, data.ValueSize);
						} else {
							data.ValueSize = 0;
						}
						break;
					}
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
	vector<DisassemblyResult>& source = _disassemblyResult[(int)type];
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
