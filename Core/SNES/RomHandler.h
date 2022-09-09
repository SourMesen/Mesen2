#pragma once
#include "pch.h"
#include "SNES/RamHandler.h"

class RomHandler : public RamHandler
{
public:
	using RamHandler::RamHandler;

	void Write(uint32_t addr, uint8_t value) override
	{
	}
};