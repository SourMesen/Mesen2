#pragma once
#include "stdafx.h"

class IPceMapper
{
public:
	virtual uint8_t Read(uint8_t bank, uint16_t addr, uint8_t value) { return value; }
	virtual void Write(uint8_t bank, uint16_t addr, uint8_t value) {}
};
