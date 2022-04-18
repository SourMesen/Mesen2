#pragma once
#include "stdafx.h"

class PceMemoryManager;

class PceTimer
{
private:
	uint8_t _reloadValue = 0;
	uint8_t _counter = 0;
	uint16_t _scaler = 1024 * 3;
	bool _enabled = false;
	PceMemoryManager* _memoryManager = nullptr;
	uint64_t _lastClock = 0;

public:
	PceTimer(PceMemoryManager* memoryManager);

	void Exec();

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);
};
