#include "pch.h"
#include "Debugger/Disassembler.h"
#include "Debugger/DisassemblySearch.h"
#include "Debugger/LabelManager.h"

DisassemblySearch::DisassemblySearch(Disassembler* disassembler, LabelManager* labelManager)
{
	_disassembler = disassembler;
	_labelManager = labelManager;
}

int32_t DisassemblySearch::SearchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options)
{
	CodeLineData results[1] = {};
	uint32_t resultCount = SearchDisassembly(cpuType, searchString, startAddress, options, results, 1);
	return resultCount > 0 ? results[0].Address : -1;
}

uint32_t DisassemblySearch::FindOccurrences(CpuType cpuType, const char* searchString, DisassemblySearchOptions options, CodeLineData output[], uint32_t maxResultCount)
{
	return SearchDisassembly(cpuType, searchString, 0, options, output, maxResultCount);
}

uint32_t DisassemblySearch::SearchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options, CodeLineData searchResults[], uint32_t maxResultCount)
{
	MemoryType memType = DebugUtilities::GetCpuMemoryType(cpuType);
	uint16_t bank = startAddress >> 16;
	uint16_t maxBank = _disassembler->GetMaxBank(cpuType);

	vector<DisassemblyResult> rows = _disassembler->Disassemble(cpuType, bank);
	if(rows.empty()) {
		return -1;
	}
	int step = options.SearchBackwards ? -1 : 1;

	string searchStr = searchString;

	int32_t startRow = _disassembler->GetMatchingRow(rows, startAddress, options.SearchBackwards);
	if(options.SearchBackwards) {
		startRow--;
	} else if(options.SkipFirstLine) {
		startRow++;
	}

	if(startRow >= 0 && startRow < rows.size()) {
		startAddress = rows[startRow].CpuAddress;
	}

	uint32_t resultCount = 0;

	int32_t prevAddress = -1;

	CodeLineData lineData = {};
	int rowCounter = 0;

	string txt;

	do {
		for(int i = startRow; i >= 0 && i < rows.size(); i += step) {
			if(rows[i].CpuAddress < 0) {
				continue;
			}

			if(
				(!options.SearchBackwards && prevAddress < startAddress && rows[i].CpuAddress >= startAddress) ||
				(options.SearchBackwards && prevAddress > startAddress && rows[i].CpuAddress <= startAddress) ||
				rowCounter > 500000
			) {
				if(rowCounter > 0) {
					//Checked entire memory space without finding a match (or checked over 500k rows), give up
					return resultCount;
				}
			}

			rowCounter++;

			prevAddress = rows[i].CpuAddress;

			_disassembler->GetLineData(rows[i], cpuType, memType, lineData);

			if(TextContains(searchStr, lineData.Text, 1000, options)) {
				searchResults[resultCount] = lineData;
				if(maxResultCount == ++resultCount) {
					return resultCount;
				}
				continue;
			}

			if(TextContains(searchStr, lineData.Comment, 1000, options)) {
				searchResults[resultCount] = lineData;
				if(maxResultCount == ++resultCount) {
					return resultCount;
				}
				continue;
			}

			if(lineData.EffectiveAddress.ShowAddress && lineData.EffectiveAddress.Address >= 0) {
				txt = _labelManager->GetLabel({ (int32_t)lineData.EffectiveAddress.Address, lineData.EffectiveAddress.Type });
				if(txt.empty()) {
					txt = "[$" + DebugUtilities::AddressToHex(lineData.LineCpuType, lineData.EffectiveAddress.Address) + "]";
				} else {
					txt = "[" + txt + "]";
				}

				if(TextContains(searchStr, txt.c_str(), (int)txt.size(), options)) {
					searchResults[resultCount] = lineData;
					if(maxResultCount == ++resultCount) {
						return resultCount;
					}
					continue;
				}
			}

			if(maxResultCount == 1 && lineData.EffectiveAddress.ValueSize > 0) {
				txt = "$" + (lineData.EffectiveAddress.ValueSize == 2 ? HexUtilities::ToHex((uint16_t)lineData.Value) : HexUtilities::ToHex((uint8_t)lineData.Value));
				if(TextContains(searchStr, txt.c_str(), (int)txt.size(), options)) {
					searchResults[resultCount] = lineData;
					if(maxResultCount == ++resultCount) {
						return resultCount;
					}
					continue;
				}
			}
		}

		//No match found, go to next/previous bank
		int nextBank = (int)bank + step;
		if(nextBank < 0) {
			nextBank = maxBank;
		} else if(nextBank > maxBank) {
			if(startAddress == 0) {
				return resultCount;
			}
			nextBank = 0;
		}
		bank = (uint16_t)nextBank;
		rows = _disassembler->Disassemble(cpuType, bank);
		if(rows.empty()) {
			return resultCount;
		}
		startRow = options.SearchBackwards ? (int32_t)rows.size() - 1 : 0;
	} while(true);

	return resultCount;
}

bool DisassemblySearch::TextContains(string& needle, const char* hay, int size, DisassemblySearchOptions& options)
{
	if(options.MatchCase) {
		return TextContains<true>(needle, hay, size, options);
	} else {
		return TextContains<false>(needle, hay, size, options);
	}
}

template<bool matchCase>
bool DisassemblySearch::TextContains(string& needle, const char* hay, int size, DisassemblySearchOptions& options)
{
	int pos = 0;
	for(int j = 0; j < size; j++) {
		char c = hay[j];
		if(c <= 0) {
			break;
		}

		if(needle[pos] == (matchCase ? c : tolower(c))) {
			if(options.MatchWholeWord && pos == 0 && j > 0 && !IsWordSeparator(hay[j - 1])) {
				continue;
			}

			pos++;
			if(pos == (int)needle.size()) {
				if(options.MatchWholeWord && j < size - 1 && !IsWordSeparator(hay[j + 1])) {
					j -= pos - 1;
					pos = 0;
					continue;
				}
				return true;
			}
		} else {
			j -= pos;
			pos = 0;
		}
	}
	return false;
}

bool DisassemblySearch::IsWordSeparator(char c)
{
	return !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '@' || c == '$' || c == '#');
}