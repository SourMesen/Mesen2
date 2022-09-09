#pragma once
#include "pch.h"
#include <regex>
#include "Debugger/Base6502Assembler.h"

enum class SnesAddrMode : uint8_t;

class SnesAssembler : public Base6502Assembler<SnesAddrMode>
{
protected:
	string GetOpName(uint8_t opcode) override;
	SnesAddrMode GetOpMode(uint8_t opcode) override;
	bool IsOfficialOp(uint8_t opcode) override;
	AssemblerSpecialCodes ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass) override;

public:
	SnesAssembler(LabelManager* labelManager);
	virtual ~SnesAssembler() {}
};