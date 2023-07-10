#pragma once
#include "pch.h"
#include <regex>
#include "Debugger/Base6502Assembler.h"
#include "PCE/PceTypes.h"

class LabelManager;

class PceAssembler final : public Base6502Assembler<PceAddrMode>
{
private:
	string GetOpName(uint8_t opcode) override;
	PceAddrMode GetOpMode(uint8_t opcode) override;
	bool IsOfficialOp(uint8_t opcode) override;
	void AdjustLabelOperand(AssemblerOperand& operand);
	void AdjustLabelOperands(AssemblerLineData& op);
	AssemblerSpecialCodes ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass) override;

public:
	PceAssembler(LabelManager* labelManager);
	virtual ~PceAssembler() = default;
};