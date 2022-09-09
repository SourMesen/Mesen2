#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class IPceMapper : public ISerializable
{
protected:
	bool _mappedBanks[0x100] = {};

public:
	virtual ~IPceMapper() = default;

	virtual uint8_t Read(uint8_t bank, uint16_t addr, uint8_t value) { return value; }
	virtual void Write(uint8_t bank, uint16_t addr, uint8_t value) {}

	bool IsBankMapped(uint8_t bank) { return _mappedBanks[bank]; }

	virtual void Serialize(Serializer& s) override {}
};
