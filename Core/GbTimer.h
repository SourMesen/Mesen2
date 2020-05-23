#pragma once
#include "stdafx.h"
#include "../Utilities/ISerializable.h"
#include "../Utilities/Serializer.h"

class GbMemoryManager;
class GbApu;

class GbTimer : public ISerializable
{
private:
	GbMemoryManager* _memoryManager;
	GbApu* _apu;

	uint16_t _divider = 0;

	bool _needReload = false; //Set after TIMA (_counter) overflowed, next cycle will reload TMA into TIMA
	bool _reloaded = false; //Set during the cycle on which TIMA is reloaded (affects TMA/TIMA writes)
	uint8_t _counter = 0;
	uint8_t _modulo = 0;

	uint8_t _control = 0;
	bool _timerEnabled = false;
	uint16_t _timerDivider = 1024;
	
	void SetDivider(uint16_t value);
	void ReloadCounter();

public:
	GbTimer(GbMemoryManager* memoryManager, GbApu* apu);
	virtual ~GbTimer();

	void Exec();

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};