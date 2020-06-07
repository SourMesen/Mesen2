#pragma once
#include "stdafx.h"
#include "GbTypes.h"
#include "../Utilities/ISerializable.h"

class GbMemoryManager;
class GbPpu;

class GbDmaController final : public ISerializable
{
private:
	GbDmaControllerState _state;
	GbMemoryManager* _memoryManager;
	GbPpu* _ppu;
	
	void ProcessDmaBlock();

public:
	GbDmaController(GbMemoryManager* memoryManager, GbPpu* ppu);

	GbDmaControllerState GetState();

	void Exec();

	bool IsOamDmaRunning();

	uint8_t Read();
	void Write(uint8_t value);

	uint8_t ReadCgb(uint16_t addr);
	void WriteCgb(uint16_t addr, uint8_t value);

	void ProcessHdma();

	void Serialize(Serializer& s) override;
};