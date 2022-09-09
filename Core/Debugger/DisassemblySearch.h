#pragma once
#include "pch.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

class Disassembler;
class LabelManager;
enum class CpuType : uint8_t;

struct DisassemblySearchOptions
{
	bool MatchCase;
	bool MatchWholeWord;
	bool SearchBackwards;
	bool SkipFirstLine;
};

class DisassemblySearch
{
private:
	Disassembler* _disassembler;
	LabelManager* _labelManager;

	uint32_t SearchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options, CodeLineData searchResults[], uint32_t maxResultCount);

	template<bool matchCase> bool TextContains(string& needle, const char* hay, int size, DisassemblySearchOptions& options);
	bool TextContains(string& needle, const char* hay, int size, DisassemblySearchOptions& options);
	bool IsWordSeparator(char c);

public:
	DisassemblySearch(Disassembler* disassembler, LabelManager* labelManager);

	int32_t SearchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options);
	uint32_t FindOccurrences(CpuType cpuType, const char* searchString, DisassemblySearchOptions options, CodeLineData output[], uint32_t maxResultCount);
};