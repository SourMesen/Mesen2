#pragma once
#include "pch.h"
#include "PCE/IPceMapper.h"
#include "PCE/PceTypes.h"
#include "Utilities/ISerializable.h"

class Emulator;
class PceConsole;

class PceArcadeCard final : public IPceMapper
{
private:
	static constexpr int ArcadeRamMemSize = 0x200000;

	PceArcadeCardState _state = {};
	Emulator* _emu = nullptr;
	uint8_t* _ram = nullptr;
	bool _isRamUsed = false;

	uint32_t GetAddress(PceArcadeCardPortConfig& port);
	void ProcessAutoInc(PceArcadeCardPortConfig& port);
	void AddOffsetToBase(PceArcadeCardPortConfig& port);

	uint8_t ReadPortValue(uint8_t portNumber);
	void WritePortValue(uint8_t portNumber, uint8_t value);

	uint8_t ReadPortRegister(uint8_t portNumber, uint8_t reg);
	void WritePortRegister(uint8_t portNumber, uint8_t reg, uint8_t value);

public:
	PceArcadeCard(PceConsole* console, Emulator* emu);
	virtual ~PceArcadeCard();

	PceArcadeCardState& GetState() { return _state; }

	uint8_t Read(uint8_t bank, uint16_t addr, uint8_t value) override;
	void Write(uint8_t bank, uint16_t addr, uint8_t value) override;
	
	void Serialize(Serializer& s) override;
};
