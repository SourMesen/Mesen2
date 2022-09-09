#pragma once
#include "pch.h"
#include <unordered_map>
#include <functional>
#include "Debugger/DebugTypes.h"

class Debugger;

class AddressHasher
{
public:
	size_t operator()(const uint64_t& addr) const
	{
		//Quick hash for addresses
		return addr;
	}
};

struct LabelInfo
{
	string Label;
	string Comment;
};

class LabelManager
{
private:
	unordered_map<uint64_t, LabelInfo, AddressHasher> _codeLabels;
	unordered_map<string, uint64_t> _codeLabelReverseLookup;

	Debugger *_debugger;

	int64_t GetLabelKey(uint32_t absoluteAddr, MemoryType memType);
	MemoryType GetKeyMemoryType(uint64_t key);
	bool InternalGetLabel(AddressInfo address, string& label);

public:
	LabelManager(Debugger *debugger);

	void SetLabel(uint32_t address, MemoryType memType, string label, string comment);
	void ClearLabels();

	AddressInfo GetLabelAbsoluteAddress(string& label);
	int32_t GetLabelRelativeAddress(string &label, CpuType cpuType);

	string GetLabel(AddressInfo address, bool checkRegisterLabels = true);
	string GetComment(AddressInfo absAddress);
	bool GetLabelAndComment(AddressInfo address, LabelInfo &label);

	bool ContainsLabel(string &label);

	bool HasLabelOrComment(AddressInfo address);
};
