#include "stdafx.h"
#include "LabelManager.h"
#include "Debugger.h"
#include "DebugUtilities.h"

LabelManager::LabelManager(Debugger *debugger)
{
	_debugger = debugger;
}

void LabelManager::ClearLabels()
{
	_codeComments.clear();
	_codeLabels.clear();
	_codeLabelReverseLookup.clear();
}

void LabelManager::SetLabel(uint32_t address, SnesMemoryType memType, string label, string comment)
{
	uint64_t key = GetLabelKey(address, memType);

	auto existingLabel = _codeLabels.find(key);
	if(existingLabel != _codeLabels.end()) {
		_codeLabelReverseLookup.erase(existingLabel->second);
	}

	_codeLabels.erase(key);
	if(!label.empty()) {
		if(label.size() > 400) {
			//Restrict labels to 400 bytes
			label = label.substr(0, 400);
		}
		_codeLabels.emplace(key, label);
		_codeLabelReverseLookup.emplace(label, key);
	}

	_codeComments.erase(key);
	if(!comment.empty()) {
		_codeComments.emplace(key, comment);
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
				return result->second;
			}
		}
	}

	return "";
}

string LabelManager::GetComment(AddressInfo absAddress)
{
	uint64_t key = GetLabelKey(absAddress.Address, absAddress.Type);

	if(key >= 0) {
		auto result = _codeComments.find(key);
		if(result != _codeComments.end()) {
			return result->second;
		}
	}

	return "";
}

void LabelManager::GetLabelAndComment(AddressInfo address, string &label, string &comment)
{
	if(address.Type <= DebugUtilities::GetLastCpuMemoryType()) {
		address = _debugger->GetAbsoluteAddress(address);
	}

	if(address.Address >= 0) {
		int64_t key = GetLabelKey(address.Address, address.Type);

		if(key >= 0) {
			auto result = _codeLabels.find(key);
			if(result != _codeLabels.end()) {
				label = result->second;
			} else {
				label.clear();
			}

			auto commentResult = _codeComments.find(key);
			if(commentResult != _codeComments.end()) {
				comment = commentResult->second;
			} else {
				comment.clear();
			}
		}
	} else {
		label.clear();
		comment.clear();
	}
}

bool LabelManager::ContainsLabel(string &label)
{
	return _codeLabelReverseLookup.find(label) != _codeLabelReverseLookup.end();
}

int32_t LabelManager::GetLabelRelativeAddress(string &label)
{
	auto result = _codeLabelReverseLookup.find(label);
	if(result != _codeLabelReverseLookup.end()) {
		uint64_t key = result->second;
		SnesMemoryType type = GetKeyMemoryType(key);
		AddressInfo addr { (int32_t)(key & 0xFFFFFFFF), type };
		return _debugger->GetRelativeAddress(addr).Address;
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
			return
				_codeLabels.find(key) != _codeLabels.end() ||
				_codeComments.find(key) != _codeComments.end();
		}
	}
	return false;
}
