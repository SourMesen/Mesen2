#include "stdafx.h"
#include "LabelManager.h"
#include "Debugger.h"
#include "DebugUtilities.h"
#include "DebugBreakHelper.h"

LabelManager::LabelManager(Debugger *debugger)
{
	_debugger = debugger;
}

void LabelManager::ClearLabels()
{
	DebugBreakHelper helper(_debugger);
	_codeLabels.clear();
	_codeLabelReverseLookup.clear();
}

void LabelManager::SetLabel(uint32_t address, SnesMemoryType memType, string label, string comment)
{
	DebugBreakHelper helper(_debugger);
	uint64_t key = GetLabelKey(address, memType);

	auto existingLabel = _codeLabels.find(key);
	if(existingLabel != _codeLabels.end()) {
		_codeLabelReverseLookup.erase(existingLabel->second.Label);
	}

	_codeLabels.erase(key);
	if(!label.empty() || !comment.empty()) {
		if(label.size() > 400) {
			//Restrict labels to 400 bytes
			label = label.substr(0, 400);
		}

		LabelInfo labelInfo;
		labelInfo.Label = label;
		labelInfo.Comment = comment;

		_codeLabels.emplace(key, labelInfo);
		_codeLabelReverseLookup.emplace(label, key);
	}
}

int64_t LabelManager::GetLabelKey(uint32_t absoluteAddr, SnesMemoryType memType)
{
	switch(memType) {
		case SnesMemoryType::PrgRom: return absoluteAddr | ((uint64_t)1 << 32);
		case SnesMemoryType::WorkRam: return absoluteAddr | ((uint64_t)2 << 32);
		case SnesMemoryType::SaveRam: return absoluteAddr | ((uint64_t)3 << 32);
		case SnesMemoryType::Register: return absoluteAddr | ((uint64_t)4 << 32);
		case SnesMemoryType::SpcRam: return absoluteAddr | ((uint64_t)5 << 32);
		case SnesMemoryType::SpcRom: return absoluteAddr | ((uint64_t)6 << 32);
		case SnesMemoryType::Sa1InternalRam: return absoluteAddr | ((uint64_t)7 << 32);
		case SnesMemoryType::GsuWorkRam: return absoluteAddr | ((uint64_t)8 << 32);
		case SnesMemoryType::BsxPsRam: return absoluteAddr | ((uint64_t)9 << 32);
		case SnesMemoryType::BsxMemoryPack: return absoluteAddr | ((uint64_t)10 << 32);
		case SnesMemoryType::DspProgramRom: return absoluteAddr | ((uint64_t)11 << 32);
		case SnesMemoryType::GbPrgRom: return absoluteAddr | ((uint64_t)12 << 32);
		case SnesMemoryType::GbWorkRam: return absoluteAddr | ((uint64_t)13 << 32);
		case SnesMemoryType::GbCartRam: return absoluteAddr | ((uint64_t)14 << 32);
		case SnesMemoryType::GbHighRam: return absoluteAddr | ((uint64_t)15 << 32);
		case SnesMemoryType::GbBootRom: return absoluteAddr | ((uint64_t)16 << 32);
		default: return -1;
	}
}

SnesMemoryType LabelManager::GetKeyMemoryType(uint64_t key)
{
	switch(key & ~(uint64_t)0xFFFFFFFF) {
		case ((uint64_t)1 << 32): return SnesMemoryType::PrgRom; break;
		case ((uint64_t)2 << 32): return SnesMemoryType::WorkRam; break;
		case ((uint64_t)3 << 32): return SnesMemoryType::SaveRam; break;
		case ((uint64_t)4 << 32): return SnesMemoryType::Register; break;
		case ((uint64_t)5 << 32): return SnesMemoryType::SpcRam; break;
		case ((uint64_t)6 << 32): return SnesMemoryType::SpcRom; break;
		case ((uint64_t)7 << 32): return SnesMemoryType::Sa1InternalRam; break;
		case ((uint64_t)8 << 32): return SnesMemoryType::GsuWorkRam; break;
		case ((uint64_t)9 << 32): return SnesMemoryType::BsxPsRam; break;
		case ((uint64_t)10 << 32): return SnesMemoryType::BsxMemoryPack; break;
		case ((uint64_t)11 << 32): return SnesMemoryType::DspProgramRom; break;
		case ((uint64_t)12 << 32): return SnesMemoryType::GbPrgRom; break;
		case ((uint64_t)13 << 32): return SnesMemoryType::GbWorkRam; break;
		case ((uint64_t)14 << 32): return SnesMemoryType::GbCartRam; break;
		case ((uint64_t)15 << 32): return SnesMemoryType::GbHighRam; break;
		case ((uint64_t)16 << 32): return SnesMemoryType::GbBootRom; break;
	}

	throw std::runtime_error("Invalid label key");
}

string LabelManager::GetLabel(AddressInfo address)
{
	if(address.Type <= DebugUtilities::GetLastCpuMemoryType()) {
		address = _debugger->GetAbsoluteAddress(address);
	}

	if(address.Address >= 0) {
		int64_t key = GetLabelKey(address.Address, address.Type);
		if(key >= 0) {
			auto result = _codeLabels.find(key);
			if(result != _codeLabels.end()) {
				return result->second.Label;
			}
		}
	}

	return "";
}

string LabelManager::GetComment(AddressInfo absAddress)
{
	uint64_t key = GetLabelKey(absAddress.Address, absAddress.Type);

	if(key >= 0) {
		auto result = _codeLabels.find(key);
		if(result != _codeLabels.end()) {
			return result->second.Comment;
		}
	}

	return "";
}

bool LabelManager::GetLabelAndComment(AddressInfo address, LabelInfo &labelInfo)
{
	if(address.Type <= DebugUtilities::GetLastCpuMemoryType()) {
		address = _debugger->GetAbsoluteAddress(address);
	}

	if(address.Address >= 0) {
		int64_t key = GetLabelKey(address.Address, address.Type);

		if(key >= 0) {
			auto result = _codeLabels.find(key);
			if(result != _codeLabels.end()) {
				labelInfo = result->second;
				return true;
			}
		}
	}
	return false;
}

bool LabelManager::ContainsLabel(string &label)
{
	return _codeLabelReverseLookup.find(label) != _codeLabelReverseLookup.end();
}

int32_t LabelManager::GetLabelRelativeAddress(string &label, CpuType cpuType)
{
	auto result = _codeLabelReverseLookup.find(label);
	if(result != _codeLabelReverseLookup.end()) {
		uint64_t key = result->second;
		SnesMemoryType type = GetKeyMemoryType(key);
		AddressInfo addr { (int32_t)(key & 0xFFFFFFFF), type };
		return _debugger->GetRelativeAddress(addr, cpuType).Address;
	}
	//Label doesn't exist
	return -2;
}

bool LabelManager::HasLabelOrComment(AddressInfo address)
{
	if(address.Type <= DebugUtilities::GetLastCpuMemoryType()) {
		address = _debugger->GetAbsoluteAddress(address);
	}

	if(address.Address >= 0) {
		uint64_t key = GetLabelKey(address.Address, address.Type);
		if(key >= 0) {
			return _codeLabels.find(key) != _codeLabels.end();
		}
	}
	return false;
}
