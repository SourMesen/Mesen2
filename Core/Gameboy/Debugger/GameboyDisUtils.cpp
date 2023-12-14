#include "pch.h"
#include "Gameboy/Debugger/GameboyDisUtils.h"
#include "Gameboy/GbTypes.h"
#include "Gameboy/Debugger/DummyGbCpu.h"
#include "Gameboy/Gameboy.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"

constexpr const char* _opTemplate[256] = {
	"NOP",			"LD BC, e",		"LD (BC), A",	"INC BC",	"INC B",			"DEC B",			"LD B, d",		"RLCA",		"LD (a), SP",	"ADD HL, BC",	"LD A, (BC)",	"DEC BC",	"INC C",		"DEC C",		"LD C, d",		"RRCA",
	"STOP d",		"LD DE, e",		"LD (DE), A",	"INC DE",	"INC D",			"DEC D",			"LD D, d",		"RLA",		"JR r",			"ADD HL, DE",	"LD A, (DE)",	"DEC DE",	"INC E",		"DEC E",		"LD E, d",		"RRA",
	"JR NZ, r",		"LD HL, e",		"LD (HL+), A",	"INC HL",	"INC H",			"DEC H",			"LD H, d",		"DAA",		"JR Z, r",		"ADD HL, HL",	"LD A, (HL+)",	"DEC HL",	"INC L",		"DEC L",		"LD L, d",		"CPL",
	"JR NC, r",		"LD SP, e",		"LD (HL-), A",	"INC SP",	"INC (HL)",		"DEC (HL)",		"LD (HL), d",	"SCF",		"JR C, r",		"ADD HL, SP",	"LD A, (HL-)",	"DEC SP",	"INC A",		"DEC A",		"LD A, d",		"CCF",
	"LD B, B",		"LD B, C",		"LD B, D",		"LD B, E",	"LD B, H",		"LD B, L",		"LD B, (HL)",	"LD B, A",	"LD C, B",		"LD C, C",		"LD C, D",		"LD C, E",	"LD C, H",	"LD C, L",	"LD C, (HL)",	"LD C, A",
	"LD D, B",		"LD D, C",		"LD D, D",		"LD D, E",	"LD D, H",		"LD D, L",		"LD D, (HL)",	"LD D, A",	"LD E, B",		"LD E, C",		"LD E, D",		"LD E, E",	"LD E, H",	"LD E, L",	"LD E, (HL)",	"LD E, A",
	"LD H, B",		"LD H, C",		"LD H, D",		"LD H, E",	"LD H, H",		"LD H, L",		"LD H, (HL)",	"LD H, A",	"LD L, B",		"LD L, C",		"LD L, D",		"LD L, E",	"LD L, H",	"LD L, L",	"LD L, (HL)",	"LD L, A",
	"LD (HL), B",	"LD (HL), C",	"LD (HL), D",	"LD (HL), E","LD (HL), H",	"LD (HL), L",	"HALT",			"LD (HL), A","LD A, B",		"LD A, C",		"LD A, D",		"LD A, E",	"LD A, H",	"LD A, L",	"LD A, (HL)",	"LD A, A",
	"ADD A, B",		"ADD A, C",		"ADD A, D",		"ADD A, E",	"ADD A, H",		"ADD A, L",		"ADD A, (HL)",	"ADD A, A",	"ADC A, B",		"ADC A, C",		"ADC A, D",		"ADC A, E",	"ADC A, H",	"ADC A, L",	"ADC A, (HL)",	"ADC A, A",
	"SUB B",			"SUB C",			"SUB D",			"SUB E",		"SUB H",			"SUB L",			"SUB (HL)",		"SUB A",		"SBC A, B",		"SBC A, C",		"SBC A, D",		"SBC A, E",	"SBC A, H",	"SBC A, L",	"SBC A, (HL)",	"SBC A, A",
	"AND B",			"AND C",			"AND D",			"AND E",		"AND H",			"AND L",			"AND (HL)",		"AND A",		"XOR B",			"XOR C",			"XOR D",			"XOR E",		"XOR H",		"XOR L",		"XOR (HL)",		"XOR A",
	"OR B",			"OR C",			"OR D",			"OR E",		"OR H",			"OR L",			"OR (HL)",		"OR A",		"CP B",			"CP C",			"CP D",			"CP E",		"CP H",		"CP L",		"CP (HL)",		"CP A",
	"RET NZ",		"POP BC",		"JP NZ, a",		"JP a",		"CALL NZ, a",	"PUSH BC",		"ADD A, d",		"RST 00H",	"RET Z",			"RET",			"JP Z, a",		"PREFIX",	"CALL Z, a","CALL a",	"ADC A, d",		"RST 08H",
	"RET NC",		"POP DE",		"JP NC, a",		"ILL_D3",	"CALL NC, a",	"PUSH DE",		"SUB d",			"RST 10H",	"RET C",			"RETI",			"JP C, a",		"ILL_DB",	"CALL C, a","ILL_DD",	"SBC A, d",		"RST 18H",
	"LDH (c), A",	"POP HL",		"LD ($FF00+C), A","ILL_E3","ILL_E4",		"PUSH HL",		"AND d",			"RST 20H",	"ADD SP, d",	"JP HL",			"LD (a), A",	"ILL_EB",	"ILL_EC",	"ILL_ED",	"XOR d",			"RST 28H",
	"LDH A, (c)",	"POP AF",		"LD A, ($FF00+C)","DI",		"ILL_F4",		"PUSH AF",		"OR d",			"RST 30H",	"LD HL, SP+d",	"LD SP, HL",	"LD A, (a)",	"EI",			"ILL_FC",	"ILL_FD",	"CP d",			"RST 38H"
};

constexpr const char* _cbTemplate[256] = {
	"RLC B",		"RLC C",		"RLC D",		"RLC E",		"RLC H",		"RLC L",		"RLC (HL)",		"RLC A",		"RRC B",		"RRC C",		"RRC D",		"RRC E",		"RRC H",		"RRC L",		"RRC (HL)",		"RRC A",
	"RL B",		"RL C",		"RL D",		"RL E",		"RL H",		"RL L",		"RL (HL)",		"RL A",		"RR B",		"RR C",		"RR D",		"RR E",		"RR H",		"RR L",		"RR (HL)",		"RR A",
	"SLA B",		"SLA C",		"SLA D",		"SLA E",		"SLA H",		"SLA L",		"SLA (HL)",		"SLA A",		"SRA B",		"SRA C",		"SRA D",		"SRA E",		"SRA H",		"SRA L",		"SRA (HL)",		"SRA A",
	"SWAP B",	"SWAP C",	"SWAP D",	"SWAP E",	"SWAP H",	"SWAP L",	"SWAP (HL)",	"SWAP A",	"SRL B",		"SRL C",		"SRL D",		"SRL E",		"SRL H",		"SRL L",		"SRL (HL)",		"SRL A",
	"BIT 0, B",	"BIT 0, C",	"BIT 0, D",	"BIT 0, E",	"BIT 0, H",	"BIT 0, L",	"BIT 0, (HL)",	"BIT 0, A",	"BIT 1, B",	"BIT 1, C",	"BIT 1, D",	"BIT 1, E",	"BIT 1, H",	"BIT 1, L",	"BIT 1, (HL)",	"BIT 1, A",
	"BIT 2, B",	"BIT 2, C",	"BIT 2, D",	"BIT 2, E",	"BIT 2, H",	"BIT 2, L",	"BIT 2, (HL)",	"BIT 2, A",	"BIT 3, B",	"BIT 3, C",	"BIT 3, D",	"BIT 3, E",	"BIT 3, H",	"BIT 3, L",	"BIT 3, (HL)",	"BIT 3, A",
	"BIT 4, B",	"BIT 4, C",	"BIT 4, D",	"BIT 4, E",	"BIT 4, H",	"BIT 4, L",	"BIT 4, (HL)",	"BIT 4, A",	"BIT 5, B",	"BIT 5, C",	"BIT 5, D",	"BIT 5, E",	"BIT 5, H",	"BIT 5, L",	"BIT 5, (HL)",	"BIT 5, A",
	"BIT 6, B",	"BIT 6, C",	"BIT 6, D",	"BIT 6, E",	"BIT 6, H",	"BIT 6, L",	"BIT 6, (HL)",	"BIT 6, A",	"BIT 7, B",	"BIT 7, C",	"BIT 7, D",	"BIT 7, E",	"BIT 7, H",	"BIT 7, L",	"BIT 7, (HL)",	"BIT 7, A",
	"RES 0, B",	"RES 0, C",	"RES 0, D",	"RES 0, E",	"RES 0, H",	"RES 0, L",	"RES 0, (HL)",	"RES 0, A",	"RES 1, B",	"RES 1, C",	"RES 1, D",	"RES 1, E",	"RES 1, H",	"RES 1, L",	"RES 1, (HL)",	"RES 1, A",
	"RES 2, B",	"RES 2, C",	"RES 2, D",	"RES 2, E",	"RES 2, H",	"RES 2, L",	"RES 2, (HL)",	"RES 2, A",	"RES 3, B",	"RES 3, C",	"RES 3, D",	"RES 3, E",	"RES 3, H",	"RES 3, L",	"RES 3, (HL)",	"RES 3, A",
	"RES 4, B",	"RES 4, C",	"RES 4, D",	"RES 4, E",	"RES 4, H",	"RES 4, L",	"RES 4, (HL)",	"RES 4, A",	"RES 5, B",	"RES 5, C",	"RES 5, D",	"RES 5, E",	"RES 5, H",	"RES 5, L",	"RES 5, (HL)",	"RES 5, A",
	"RES 6, B",	"RES 6, C",	"RES 6, D",	"RES 6, E",	"RES 6, H",	"RES 6, L",	"RES 6, (HL)",	"RES 6, A",	"RES 7, B",	"RES 7, C",	"RES 7, D",	"RES 7, E",	"RES 7, H",	"RES 7, L",	"RES 7, (HL)",	"RES 7, A",
	"SET 0, B",	"SET 0, C",	"SET 0, D",	"SET 0, E",	"SET 0, H",	"SET 0, L",	"SET 0, (HL)",	"SET 0, A",	"SET 1, B",	"SET 1, C",	"SET 1, D",	"SET 1, E",	"SET 1, H",	"SET 1, L",	"SET 1, (HL)",	"SET 1, A",
	"SET 2, B",	"SET 2, C",	"SET 2, D",	"SET 2, E",	"SET 2, H",	"SET 2, L",	"SET 2, (HL)",	"SET 2, A",	"SET 3, B",	"SET 3, C",	"SET 3, D",	"SET 3, E",	"SET 3, H",	"SET 3, L",	"SET 3, (HL)",	"SET 3, A",
	"SET 4, B",	"SET 4, C",	"SET 4, D",	"SET 4, E",	"SET 4, H",	"SET 4, L",	"SET 4, (HL)",	"SET 4, A",	"SET 5, B",	"SET 5, C",	"SET 5, D",	"SET 5, E",	"SET 5, H",	"SET 5, L",	"SET 5, (HL)",	"SET 5, A",
	"SET 6, B",	"SET 6, C",	"SET 6, D",	"SET 6, E",	"SET 6, H",	"SET 6, L",	"SET 6, (HL)",	"SET 6, A",	"SET 7, B",	"SET 7, C",	"SET 7, D",	"SET 7, E",	"SET 7, H",	"SET 7, L",	"SET 7, (HL)",	"SET 7, A",
};

constexpr const uint8_t _opSize[256] = {
	1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
	1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1,
	2,1,1,1,1,1,2,1,2,1,3,1,1,1,2,1,
	2,1,1,1,1,1,2,1,2,1,3,1,1,1,2,1,
};

enum class AddrType : uint8_t
{
	None,
	BC,
	DE,
	HL,
	C,
	Suff,
	Stk
};

static constexpr const AddrType _gbEffAddrType[256] = {
	AddrType::None,AddrType::None,AddrType::BC,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::BC,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::None,AddrType::DE,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::DE,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::HL,	AddrType::HL,	AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::HL,	AddrType::HL,	AddrType::HL,	AddrType::HL,	AddrType::HL,	AddrType::HL,	AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::HL,	AddrType::None,
	AddrType::None,AddrType::Stk,	AddrType::None,AddrType::None,AddrType::None,AddrType::Stk,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::Suff,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::Stk,	AddrType::None,AddrType::None,AddrType::None,AddrType::Stk,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::Stk,	AddrType::C,	AddrType::None,AddrType::None,AddrType::Stk,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,
	AddrType::None,AddrType::Stk,	AddrType::C,	AddrType::None,AddrType::None,AddrType::Stk,	AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None,AddrType::None
};

void GameboyDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	AddressInfo addrInfo { 0, MemoryType::GameboyMemory };
	auto getOperand = [&str, &addrInfo, labelManager](uint16_t addr) {
		addrInfo.Address = addr;
		string label = labelManager ? labelManager->GetLabel(addrInfo) :"";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(addr));
		} else {
			str.Write(label, true);
		}
	};

	uint8_t* byteCode = info.GetByteCode();
	const char* op = byteCode[0] == 0xCB ? _cbTemplate[byteCode[1]] : _opTemplate[byteCode[0]];
	if(byteCode[0] == 0xCB) {
		byteCode++;
	}

	int i = 0;
	while(op[i]) {
		switch(op[i]) {
			//Relative jumps
			case 'r': getOperand((uint16_t)(memoryAddr + (int8_t)byteCode[1] + GetOpSize(byteCode[0]))); break;

			//Jump addresses, memory addresses
			case 'a': getOperand((uint16_t)(byteCode[1] | (byteCode[2] << 8))); break;
			case 'c': getOperand((uint16_t)(0xFF00 | byteCode[1])); break;

			//Immediate values
			case 'd': str.WriteAll("$", HexUtilities::ToHex(byteCode[1])); break;
			case 'e': str.WriteAll("$", HexUtilities::ToHex((uint16_t)(byteCode[1] | (byteCode[2] << 8)))); break;

			default: str.Write(op[i]); break;
		}
		i++;
	}

	out += str.ToString();
}

EffectiveAddressInfo GameboyDisUtils::GetEffectiveAddress(DisassemblyInfo& info, Gameboy* console, GbCpuState& state)
{
	bool isJump = GameboyDisUtils::IsUnconditionalJump(info.GetOpCode()) || GameboyDisUtils::IsConditionalJump(info.GetOpCode());
	if(isJump) {
		//For jumps, show no address/value
		return { };
	}

	switch(_gbEffAddrType[info.GetOpCode()]) {
		default: break;

		case AddrType::BC: return { (state.B << 8) | state.C, 1, true };
		case AddrType::DE: return { (state.D << 8) | state.E, 1, true };
		case AddrType::HL: return { (state.H << 8) | state.L, 1, true };
		case AddrType::C: return { 0xFF00 + state.C, 1, true };
		case AddrType::Suff:
			if((info.GetByteCode()[1] & 0x07) == 0x06) {
				return { (state.H << 8) | state.L, 1, true };
			}
			break;

		case AddrType::Stk: return {};
	}

	DummyGbCpu dummyCpu;
	dummyCpu.Init(console->GetEmulator(), console, console->GetMemoryManager());
	dummyCpu.SetDummyState(state);
	dummyCpu.Exec();

	uint32_t count = dummyCpu.GetOperationCount();
	for(int i = count - 1; i > 0; i--) {
		MemoryOperationInfo opInfo = dummyCpu.GetOperationInfo(i);
		if(opInfo.Type != MemoryOperationType::ExecOperand) {
			return { (int32_t)opInfo.Address, 1, false };
		}
	}
	
	return {};
}

uint8_t GameboyDisUtils::GetOpSize(uint8_t opCode)
{
	return _opSize[opCode];
}

bool GameboyDisUtils::IsJumpToSub(uint8_t opCode)
{
	switch(opCode) {
		case 0xC4: //CALL NZ,a16
		case 0xC7: //RST 00H
		case 0xCD: //CALL a16
		case 0xCC: //CALL Z,a16
		case 0xCF: //RST 08H
		case 0xD4: //CALL NC,a16
		case 0xD7: //RST 10H
		case 0xDC: //CALL C,a16
		case 0xDF: //RST 18H
		case 0xE7: //RST 20H
		case 0xEF: //RST 28H
		case 0xF7: //RST 30H
		case 0xFF: //RST 38H
			return true;

		default:
			return false;
	}
}

bool GameboyDisUtils::IsReturnInstruction(uint8_t opCode)
{
	switch(opCode) {
		case 0xC0: //RET NZ
		case 0xC8: //RET Z
		case 0xD0: //RET NC
		case 0xD8: //RET C
		case 0xC9: //RET
		case 0xD9: //RETI
			return true;

		default:
			return false;
	}
}

bool GameboyDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x18: //JR r8
		case 0xC3: //JP a16
		case 0xC7: //RST 00H
		case 0xC9: //RET
		case 0xCD: //CALL a16
		case 0xCF: //RST 08H
		case 0xD7: //RST 10H
		case 0xD9: //RETI
		case 0xDF: //RST 18H
		case 0xE7: //RST 20H
		case 0xE9: //JP (HL)
		case 0xEF: //RST 28H
		case 0xF7: //RST 30H
		case 0xFF: //RST 38H
			return true;

		default:
			return false;
	}
}

bool GameboyDisUtils::IsConditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x20: //JR NZ,r8
		case 0x28: //JR Z,r8
		case 0x30: //JR NC,r8
		case 0x38: //JR C,r8
		case 0xC0: //RET NZ
		case 0xC2: //JP NZ,a16
		case 0xC4: //CALL NZ,a16
		case 0xC8: //RET Z
		case 0xCA: //JP Z,a16
		case 0xCC: //CALL Z,a16
		case 0xD0: //RET NC
		case 0xD2: //JP NC,a16
		case 0xD4: //CALL NC,a16
		case 0xD8: //RET C
		case 0xDA: //JP C,a16
		case 0xDC: //CALL C,a16
			return true;

		default:
			return false;
	}
}


CdlFlags::CdlFlags GameboyDisUtils::GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc)
{
	switch(opCode) {
		case 0xC4: //CALL NZ,a16
		case 0xC7: //RST 00H
		case 0xCD: //CALL a16
		case 0xCC: //CALL Z,a16
		case 0xCF: //RST 08H
		case 0xD4: //CALL NC,a16
		case 0xD7: //RST 10H
		case 0xDC: //CALL C,a16
		case 0xDF: //RST 18H
		case 0xE7: //RST 20H
		case 0xEF: //RST 28H
		case 0xF7: //RST 30H
		case 0xFF: //RST 38H
			return (pc != prevPc + GameboyDisUtils::GetOpSize(opCode)) ? CdlFlags::SubEntryPoint : CdlFlags::None;

		case 0x18: //JR r8
		case 0xC3: //JP a16
		case 0xE9: //JP (HL)
		case 0x20: //JR NZ,r8
		case 0x28: //JR Z,r8
		case 0x30: //JR NC,r8
		case 0x38: //JR C,r8
		case 0xC2: //JP NZ,a16
		case 0xCA: //JP Z,a16
		case 0xD2: //JP NC,a16
		case 0xDA: //JP C,a16
			return (pc != prevPc + GameboyDisUtils::GetOpSize(opCode)) ? CdlFlags::JumpTarget : CdlFlags::None;

		default:
			return CdlFlags::None;
	}
}

string GameboyDisUtils::GetOpTemplate(uint8_t op, bool prefixed)
{
	return prefixed ? _cbTemplate[op] : _opTemplate[op];
}
