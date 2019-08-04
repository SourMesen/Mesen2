#pragma once
#include "stdafx.h"
#include <unordered_map>
#include <functional>
#include "DebugTypes.h"

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

	int64_t GetLabelKey(uint32_t absoluteAddr, SnesMemoryType memType);
	SnesMemoryType GetKeyMemoryType(uint64_t key);

public:
	LabelManager(Debugger *debugger);

	void SetLabel(uint32_t address, SnesMemoryType memType, string label, string comment);
	void ClearLabels();

	int32_t GetLabelRelativeAddress(string &label);

	string GetLabel(AddressInfo address);
	string GetComment(AddressInfo absAddress);
	bool GetLabelAndComment(AddressInfo address, LabelInfo &label);

	bool ContainsLabel(string &label);

	bool HasLabelOrComment(AddressInfo address);
};
