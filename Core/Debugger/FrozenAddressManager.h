#pragma once
#include "pch.h"

class FrozenAddressManager
{
protected:
	unordered_set<uint32_t> _frozenAddresses;

public:
	void UpdateFrozenAddresses(uint32_t start, uint32_t end, bool freeze)
	{
		if(freeze) {
			for(uint32_t i = start; i <= end; i++) {
				_frozenAddresses.emplace(i);
			}
		} else {
			for(uint32_t i = start; i <= end; i++) {
				_frozenAddresses.erase(i);
			}
		}
	}

	bool IsFrozenAddress(uint32_t addr)
	{
		return _frozenAddresses.size() > 0 && _frozenAddresses.find(addr) != _frozenAddresses.end();
	}

	void GetFrozenState(uint32_t start, uint32_t end, bool* outState)
	{
		for(uint32_t i = start; i <= end; i++) {
			outState[i - start] = _frozenAddresses.find(i) != _frozenAddresses.end();
		}
	}
};