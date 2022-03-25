#include "stdafx.h"
#include <algorithm>
#include "Debugger/Disassembler.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/DebugUtilities.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SpcTypes.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"
#include "Gameboy/GbTypes.h"
#include "NES/NesTypes.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"

Disassembler::Disassembler(IConsole* console, Debugger* debugger)
{
	_debugger = debugger;
	_labelManager = debugger->GetLabelManager();
	_console = console;
	_settings = debugger->GetEmulator()->GetSettings();
	_memoryDumper = _debugger->GetMemoryDumper();

	for(int i = (int)MemoryType::SnesPrgRom; i < (int)MemoryType::Register; i++) {
		InitSource((MemoryType)i);
	}
}

void Disassembler::InitSource(MemoryType type)
{
	uint32_t size = _memoryDumper->GetMemorySize(type);
	_sources[(int)type] = { vector<DisassemblyInfo>(size), size };
}

DisassemblerSource& Disassembler::GetSource(MemoryType type)
{
	return _sources[(int)type];
}

uint32_t Disassembler::BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type)
{
	DisassemblerSource& src = GetSource(addrInfo.Type);

	int returnSize = 0;
	int32_t address = addrInfo.Address;
	do {
		DisassemblyInfo &disInfo = src.Cache[address];
		if(!disInfo.IsInitialized() || !disInfo.IsValid(cpuFlags)) {
			disInfo.Initialize(address, cpuFlags, type, addrInfo.Type, _memoryDumper);
			for(int i = 1; i < disInfo.GetOpSize() && address + i < src.Cache.size() ; i++) {
				//Clear any instructions that start in the middle of this one
				//(can happen when resizing an instruction after X/M updates)
				src.Cache[address + i] = DisassemblyInfo();
			}
			returnSize += disInfo.GetOpSize();
		} else {
			returnSize += disInfo.GetOpSize();
			break;
		}

		if(!disInfo.CanDisassembleNextOp()) {
			//Can't assume what follows is code, stop disassembling
			break;
		}

		disInfo.UpdateCpuFlags(cpuFlags);
		address += disInfo.GetOpSize();
	} while(address >= 0 && address < (int32_t)src.Cache.size());

	return returnSize;
}

void Disassembler::ResetPrgCache()
{
	InitSource(MemoryType::SnesPrgRom);
	InitSource(MemoryType::GbPrgRom);
	InitSource(MemoryType::NesPrgRom);
}

void Disassembler::InvalidateCache(AddressInfo addrInfo, CpuType type)
{
	if(addrInfo.Address >= 0) {
		DisassemblerSource& src = GetSource(addrInfo.Type);
		for(int i = 0; i < 4; i++) {
			if(addrInfo.Address >= i) {
				if(src.Cache[addrInfo.Address - i].IsInitialized()) {
					src.Cache[addrInfo.Address - i].Reset();
				}
			}
		}
	}
}

vector<DisassemblyResult> Disassembler::Disassemble(CpuType cpuType, uint16_t bank)
{
	constexpr int bytesPerRow = 8;

	vector<DisassemblyResult> results;
	results.reserve(20000);

	bool disUnident = _settings->CheckDebuggerFlag(DebuggerFlags::DisassembleUnidentifiedData);
	bool disData = _settings->CheckDebuggerFlag(DebuggerFlags::DisassembleVerifiedData);
	bool showUnident = _settings->CheckDebuggerFlag(DebuggerFlags::ShowUnidentifiedData);
	bool showData = _settings->CheckDebuggerFlag(DebuggerFlags::ShowVerifiedData);

	bool inUnknownBlock = false;
	bool inVerifiedBlock = false;
	bool inUnmappedBlock = false;
	LabelInfo labelInfo;
	AddressInfo prevAddrInfo = {};
	int byteCounter = 0;
	
	CodeDataLogger* cdl = _debugger->GetCodeDataLogger(cpuType);
	MemoryType cdlMemType = cdl ? cdl->GetPrgMemoryType() : MemoryType::Register;

	AddressInfo relAddress = {};
	relAddress.Type = DebugUtilities::GetCpuMemoryType(cpuType);

	if(bank > GetMaxBank(cpuType)) {
		return results;
	}

	int32_t bankStart = bank << 16;
	int32_t bankEnd = (bank + 1) << 16;
	bankEnd = std::min<int32_t>(bankEnd, (int32_t)_memoryDumper->GetMemorySize(relAddress.Type));

	AddressInfo addrInfo = {};

	auto pushEndBlock = [&relAddress, &prevAddrInfo, &results, &inVerifiedBlock, &inUnknownBlock, &byteCounter, showData, showUnident, bytesPerRow]() {
		if(inUnknownBlock || inVerifiedBlock) {
			int flags = LineFlags::BlockEnd;
			if(inVerifiedBlock) {
				flags |= LineFlags::VerifiedData;
			}
			if((inVerifiedBlock && showData) || (inUnknownBlock && showUnident)) {
				flags |= LineFlags::ShowAsData;
			}

			results[results.size() - 1].SetByteCount(bytesPerRow - byteCounter + 1);
			results.push_back(DisassemblyResult(prevAddrInfo, relAddress.Address - 1, flags));
			inUnknownBlock = false;
			inVerifiedBlock = false;
			byteCounter = 0;
		}
	};

	for(int32_t i = bankStart; i < bankEnd; i++) {
		relAddress.Address = i;
		addrInfo = _console->GetAbsoluteAddress(relAddress);

		if(addrInfo.Address < 0 || addrInfo.Type == MemoryType::Register) {
			pushEndBlock();
			inUnmappedBlock = true;
			continue;
		}

		if(inUnmappedBlock) {
			int32_t prevAddress = results.size() > 0 ? results[results.size() - 1].CpuAddress + 1 : bankStart;
			results.push_back(DisassemblyResult(prevAddress, LineFlags::BlockStart | LineFlags::UnmappedMemory));
			results.push_back(DisassemblyResult(prevAddress, LineFlags::UnmappedMemory | LineFlags::Empty));
			results.push_back(DisassemblyResult(i - 1, LineFlags::BlockEnd | LineFlags::UnmappedMemory));
			inUnmappedBlock = false;
		}

		DisassemblerSource& src = GetSource(addrInfo.Type);
		DisassemblyInfo disassemblyInfo = src.Cache[addrInfo.Address];
		
		uint8_t opSize = 0;

		bool isCode = addrInfo.Type == cdlMemType && cdl ? cdl->IsCode(addrInfo.Address) : false;
		bool isData = addrInfo.Type == cdlMemType && cdl ? cdl->IsData(addrInfo.Address) : false;

		if(disassemblyInfo.IsInitialized()) {
			opSize = disassemblyInfo.GetOpSize();
		} else if((isData && disData) || (!isData && !isCode && disUnident)) {
			disassemblyInfo.Initialize(i, 0, cpuType, relAddress.Type, _memoryDumper);
			opSize = disassemblyInfo.GetOpSize();
		}

		if(opSize > 0) {
			pushEndBlock();

			if(addrInfo.Type == cdlMemType && cdl && cdl->IsSubEntryPoint(addrInfo.Address)) {
				results.push_back(DisassemblyResult(addrInfo, i, LineFlags::SubStart | LineFlags::BlockStart | LineFlags::VerifiedCode | LineFlags::Empty));
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

			if(disassemblyInfo.IsReturnInstruction()) {
				//End of function
				results.push_back(DisassemblyResult(i, LineFlags::VerifiedCode | LineFlags::BlockEnd | LineFlags::Empty));
			} 

			//Move to the end of the instruction (but realign disassembly if another valid instruction is found)
			//This can sometimes happen if the 2nd byte of BRK/COP is reused as the first byte of the next instruction
			//Also required when disassembling unvalidated data as code (to realign once we find verified code)
			for(int j = 1, max = (int)src.Cache.size(); j < opSize && addrInfo.Address + j < max; j++) {
				if(src.Cache[addrInfo.Address + j].IsInitialized()) {
					break;
				}
				i++;
			}
		} else {
			if(showData || showUnident) {
				if((isData && inUnknownBlock) || (!isData && inVerifiedBlock)) {
					pushEndBlock();
				}
			}

			if(byteCounter > 0) {
				//If showing as hex data, add a new data line every 8 bytes
				byteCounter--;
				if(byteCounter == 0) {
					results[results.size() - 1].SetByteCount(bytesPerRow);
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::ShowAsData | (isData ? LineFlags::VerifiedData : 0), bytesPerRow));
					byteCounter = bytesPerRow;
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
					byteCounter = bytesPerRow;
				} else {
					//If not showing the data at all, display 1 empty line
					results.push_back(DisassemblyResult(addrInfo, i, LineFlags::Empty | (isData ? LineFlags::VerifiedData : 0)));
				}
			}
			prevAddrInfo = addrInfo;
		}
	}

	if(inUnknownBlock || inVerifiedBlock) {
		int flags = LineFlags::BlockEnd | (inVerifiedBlock ? LineFlags::VerifiedData : 0) | (((inVerifiedBlock && showData) || (inUnknownBlock && showUnident)) ? LineFlags::ShowAsData : 0);
		results.push_back(DisassemblyResult(addrInfo, bankEnd - 1, flags));
	}

	return results;
}

CodeLineData Disassembler::GetLineData(DisassemblyResult& row, CpuType type, MemoryType memType)
{
	CodeLineData data = {};
	data.Address = row.CpuAddress;
	data.AbsoluteAddress = row.Address.Address;
	data.EffectiveAddress = -1;
	data.Flags = row.Flags;

	switch(row.Address.Type) {
		default: break;
		case MemoryType::GbPrgRom:
		case MemoryType::SnesPrgRom:
		case MemoryType::NesPrgRom:
			data.Flags |= (uint8_t)LineFlags::PrgRom;
			break;

		case MemoryType::GbWorkRam:
		case MemoryType::SnesWorkRam:
		case MemoryType::NesWorkRam:
			data.Flags |= (uint8_t)LineFlags::WorkRam;
			break;

		case MemoryType::GbCartRam:
		case MemoryType::SnesSaveRam:
		case MemoryType::NesSaveRam:
			data.Flags |= (uint8_t)LineFlags::SaveRam;
			break;
	}

	bool isBlockStartEnd = (data.Flags & (LineFlags::BlockStart | LineFlags::BlockEnd | LineFlags::Empty)) != 0;
	if(!isBlockStartEnd && row.Address.Address >= 0) {
		if((data.Flags & LineFlags::ShowAsData)) {
			FastString str(".db", 3);
			for(int i = 0; i < row.GetByteCount(); i++) {
				str.Write(" $", 2);
				str.Write(HexUtilities::ToHexChar(_memoryDumper->GetMemoryValue(memType, row.CpuAddress + i)), 2);
			}
			memcpy(data.Text, str.ToString(), str.GetSize());
		} else if((data.Flags & LineFlags::Comment) && row.CommentLine >= 0) {
			string comment = ";" + StringUtilities::Split(_labelManager->GetComment(row.Address), '\n')[row.CommentLine];
			data.Flags |= LineFlags::VerifiedCode;
			memcpy(data.Comment, comment.c_str(), std::min<int>((int)comment.size(), 1000));
		} else if(data.Flags & LineFlags::Label) {
			string label = _labelManager->GetLabel(row.Address) + ":";
			data.Flags |= LineFlags::VerifiedCode;
			memcpy(data.Text, label.c_str(), std::min<int>((int)label.size(), 1000));
		} else {
			DisassemblerSource& src = GetSource(row.Address.Type);
			DisassemblyInfo disInfo = src.Cache[row.Address.Address];
			CpuType lineCpuType = disInfo.IsInitialized() ? disInfo.GetCpuType() : type;

			switch(lineCpuType) {
				case CpuType::Snes:
				case CpuType::Sa1:
				{
					SnesCpuState state = (SnesCpuState&)_debugger->GetCpuStateRef(lineCpuType);
					state.PC = (uint16_t)row.CpuAddress;
					state.K = (row.CpuAddress >> 16);

					CodeDataLogger* cdl = _debugger->GetCodeDataLogger(lineCpuType);
					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, state.PS, lineCpuType, row.Address.Type, _memoryDumper);
					} else {
						data.Flags |= (row.Address.Type != MemoryType::SnesPrgRom || cdl->IsCode(data.AbsoluteAddress)) ? LineFlags::VerifiedCode : LineFlags::UnexecutedCode;
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

				case CpuType::Spc:
				{
					SpcState state = (SpcState&)_debugger->GetCpuStateRef(lineCpuType);
					state.PC = (uint16_t)row.CpuAddress;

					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, 0, CpuType::Spc, row.Address.Type, _memoryDumper);
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

				case CpuType::Gsu:
				{
					GsuState state = (GsuState&)_debugger->GetCpuStateRef(lineCpuType);
					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, 0, CpuType::Gsu, row.Address.Type, _memoryDumper);
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
					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, 0, type, row.Address.Type, _memoryDumper);
					} else {
						data.Flags |= LineFlags::VerifiedCode;
					}

					data.OpSize = disInfo.GetOpSize();
					data.EffectiveAddress = -1;
					data.ValueSize = 0;
					break;

				case CpuType::Cx4:
				{
					Cx4State state = (Cx4State&)_debugger->GetCpuStateRef(lineCpuType);
					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, 0, type, row.Address.Type, _memoryDumper);
					} else {
						data.Flags |= LineFlags::VerifiedCode;
					}

					data.OpSize = disInfo.GetOpSize();
					data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);
					data.ValueSize = 0;
					break;
				}

				case CpuType::Gameboy:
				{
					GbCpuState state = (GbCpuState&)_debugger->GetCpuStateRef(lineCpuType);
					CodeDataLogger* cdl = _debugger->GetCodeDataLogger(lineCpuType);
					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, 0, CpuType::Gameboy, row.Address.Type, _memoryDumper);
					} else {
						data.Flags |= cdl->IsCode(data.AbsoluteAddress) ? LineFlags::VerifiedCode : LineFlags::UnexecutedCode;
					}

					data.OpSize = disInfo.GetOpSize();
					data.EffectiveAddress = disInfo.GetEffectiveAddress(_debugger, &state, lineCpuType);
					data.ValueSize = 0;
					break;
				}

				case CpuType::Nes:
				{
					NesCpuState state = (NesCpuState&)_debugger->GetCpuStateRef(lineCpuType);
					CodeDataLogger* cdl = _debugger->GetCodeDataLogger(lineCpuType);
					if(!disInfo.IsInitialized()) {
						disInfo = DisassemblyInfo(row.Address.Address, 0, CpuType::Nes, row.Address.Type, _memoryDumper);
					} else {
						data.Flags |= cdl->IsCode(data.AbsoluteAddress) ? LineFlags::VerifiedCode : LineFlags::UnexecutedCode;
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
			disInfo.GetDisassembly(text, row.CpuAddress, _labelManager, _settings);
			memcpy(data.Text, text.c_str(), std::min<int>((int)text.size(), 1000));

			disInfo.GetByteCode(data.ByteCode);

			if(data.Flags & LineFlags::Comment) {
				string comment = ";" + _labelManager->GetComment(row.Address);
				memcpy(data.Comment, comment.c_str(), std::min<int>((int)comment.size(), 1000));
			} else {
				data.Comment[0] = 0;
			}
		}
	} else {
		if(data.Flags & LineFlags::SubStart) {
			string label = _labelManager->GetLabel(row.Address);
			if(label.empty()) {
				label = "sub start";
			}
			memcpy(data.Text, label.c_str(), label.size() + 1);
		} else if(data.Flags & LineFlags::BlockStart) {
			string label = (data.Flags & LineFlags::VerifiedData) ? "data" : "unidentified";
			if(data.Flags & LineFlags::UnmappedMemory) {
				label = "unmapped";
			}
			memcpy(data.Text, label.c_str(), label.size() + 1);
		}
	}

	return data;
}

int32_t Disassembler::GetMatchingRow(vector<DisassemblyResult>& rows, uint32_t address, bool returnFirstRow)
{
	int32_t i;
	for(i = 0; i < (int32_t)rows.size(); i++) {
		if(rows[i].CpuAddress == (int32_t)address) {
			if(i + 1 >= rows.size() || rows[i + 1].CpuAddress != (int32_t)address || address == 0 || returnFirstRow) {
				//Keep going down until the last instance of the matching address is found
				//Except for address 0, to ensure scrolling to the very top is allowed
				break;
			}
		} else if(rows[i].CpuAddress > (int32_t)address) {
			while(i > 0 && (rows[i].CpuAddress > (int32_t)address || rows[i].CpuAddress < 0)) {
				i--;
			}
			break;
		}
	}
	return std::max(0, i);
}

uint32_t Disassembler::GetDisassemblyOutput(CpuType type, uint32_t address, CodeLineData output[], uint32_t rowCount)
{
	uint16_t bank = address >> 16;
	Timer timer;
	vector<DisassemblyResult> rows = Disassemble(type, bank);

	int32_t i = GetMatchingRow(rows, address, true);

	if(i >= (int32_t)rows.size()) {
		return 0;
	}

	MemoryType memType = DebugUtilities::GetCpuMemoryType(type);
	uint32_t maxBank = (_memoryDumper->GetMemorySize(memType) - 1) >> 16;

	int32_t row;
	for(row = 0; row < (int32_t)rowCount; row++){
		if(row + i >= rows.size()) {
			if(bank < maxBank) {
				bank++;
				rows = Disassemble(type, bank);
				if(rows.size() == 0) {
					break;
				}
				i = -row;
			} else {
				break;
			}
		}

		output[row] = GetLineData(rows[row + i], type, memType);
	}

	return row;
}

uint16_t Disassembler::GetMaxBank(CpuType cpuType)
{
	AddressInfo relAddress = {};
	relAddress.Type = DebugUtilities::GetCpuMemoryType(cpuType);
	return (_memoryDumper->GetMemorySize(relAddress.Type) - 1) >> 16;
}

int32_t Disassembler::GetDisassemblyRowAddress(CpuType cpuType, uint32_t address, int32_t rowOffset)
{
	uint16_t bank = address >> 16;
	vector<DisassemblyResult> rows = Disassemble(cpuType, bank);
	int32_t len = (int32_t)rows.size();
	if(len == 0) {
		return address;
	}

	uint16_t maxBank = GetMaxBank(cpuType);
	int32_t i = GetMatchingRow(rows, address, false);

	if(rowOffset > 0) {
		while(len > 0) {
			for(; i < len; i++) {
				if(rowOffset <= 0 && rows[i].CpuAddress >= 0 && rows[i].CpuAddress != (int32_t)address) {
					return rows[i].CpuAddress;
				}
				rowOffset--;
			}

			//End of bank, didn't find an appropriate row to jump to, try the next bank
			if(bank == maxBank) {
				//Reached bottom of last bank, return the bottom row
				return rows[len - 1].CpuAddress >= 0 ? rows[len - 1].CpuAddress : address;
			}

			bank++;
			rows = Disassemble(cpuType, bank);
			len = (int32_t)rows.size();
			i = 0;
		}
	} else if(rowOffset < 0) {
		while(len > 0) {
			for(; i >= 0; i--) {
				if(rowOffset >= 0 && rows[i].CpuAddress >= 0 && rows[i].CpuAddress != (int32_t)address) {
					return rows[i].CpuAddress;
				}
				rowOffset++;
			}

			//Start of bank, didn't find an appropriate row to jump to, try the previous bank
			if(bank == 0) {
				//Reached top of first bank, return the top row
				return rows[0].CpuAddress >= 0 ? rows[0].CpuAddress : address;
			}

			bank--;
			rows = Disassemble(cpuType, bank);
			len = (int32_t)rows.size();
			i = len - 1;
		}
	}

	return address;
}

int32_t Disassembler::SearchDisassembly(CpuType type, const char *searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards)
{
	//TODO
	/*DebugBreakHelper helper(_debugger);
	
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
	*/
	return -1;
}
