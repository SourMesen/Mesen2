#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class IPceMapper : public ISerializable
{
public:
	virtual ~IPceMapper() = default;

	virtual uint8_t Read(uint8_t bank, uint16_t addr, uint8_t value) { return value; }
	virtual void Write(uint8_t bank, uint16_t addr, uint8_t value) {}

	virtual void Serialize(Serializer& s) override {}
};
