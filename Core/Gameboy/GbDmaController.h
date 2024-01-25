#pragma once
#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Utilities/ISerializable.h"

class GbMemoryManager;
class GbPpu;
class GbCpu;
class Gameboy;

class GbDmaController final : public ISerializable
{
private:
	GbDmaControllerState _state = {};
	GbMemoryManager* _memoryManager = nullptr;
	GbPpu* _ppu = nullptr;
	GbCpu* _cpu = nullptr;
	Gameboy* _gameboy = nullptr;
	
	void ProcessDmaBlock();
	uint16_t GetOamReadAddress();

public:
	void Init(Gameboy* gameboy, GbMemoryManager* memoryManager, GbPpu* ppu, GbCpu* cpu);

	GbDmaControllerState GetState();

	void Exec();

	uint8_t GetLastWriteAddress();

	bool IsOamDmaConflict(uint16_t addr);
	uint16_t ProcessOamDmaReadConflict(uint16_t addr);
	bool IsOamDmaRunning();

	uint8_t Read();
	void Write(uint8_t value);

	uint8_t ReadCgb(uint16_t addr);
	void WriteCgb(uint16_t addr, uint8_t value);

	void ProcessHdma();

	void Serialize(Serializer& s) override;
};