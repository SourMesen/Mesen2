#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"
#include "Gameboy/GbTypes.h"

class GbMemoryManager;
class GbApu;

class GbTimer : public ISerializable
{
private:
	GbMemoryManager* _memoryManager = nullptr;
	GbApu* _apu = nullptr;
	GbTimerState _state = {};
	
	void SetDivider(uint16_t value);
	void ReloadCounter();

public:
	virtual ~GbTimer();

	void Init(GbMemoryManager* memoryManager, GbApu* apu);

	GbTimerState GetState();

	void Exec();
	
	bool IsFrameSequencerBitSet();

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};