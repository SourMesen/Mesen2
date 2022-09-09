#pragma once
#include "pch.h"
#include <regex>
#include "Debugger/Base6502Assembler.h"
#include "NES/NesTypes.h"

class LabelManager;

class NesAssembler final : public Base6502Assembler<NesAddrMode>
{
private:
	string GetOpName(uint8_t opcode) override;
	bool IsOfficialOp(uint8_t opcode) override;
	NesAddrMode GetOpMode(uint8_t opcode) override;
	AssemblerSpecialCodes ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass) override;

public:
	NesAssembler(LabelManager* labelManager);
	virtual ~NesAssembler() = default;
};