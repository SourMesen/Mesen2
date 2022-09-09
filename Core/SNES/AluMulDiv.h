#pragma once
#include "pch.h"
#include "SNES/InternalRegisterTypes.h"
#include "Utilities/ISerializable.h"

class SnesCpu;

class AluMulDiv final : public ISerializable
{
private:
	SnesCpu *_cpu = nullptr;

	uint64_t _prevCpuCycle = 0;

	AluState _state;

	uint32_t _shift = 0;
	uint8_t _multCounter = 0;
	uint8_t _divCounter = 0;
	
public:
	void Initialize(SnesCpu* cpu);
	
	void Run(bool isRead);

	uint8_t Read(uint16_t addr);
	uint8_t Peek(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	AluState GetState();

	void Serialize(Serializer &s) override;
};