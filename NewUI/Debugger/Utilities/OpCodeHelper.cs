using Avalonia.Controls;
using Mesen.Debugger.Controls;
using Mesen.Interop;
using Mesen.Localization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities;

public static class OpCodeHelper
{
	private static Dictionary<CpuType, CpuDocumentationData> _data = new();

	static OpCodeHelper()
	{
		InitNesDocumentation();
		InitSnesDocumentation();
		InitPceDocumentation();
	}

	public static DynamicTooltip? GetTooltip(CodeSegmentInfo seg)
	{
		if(!_data.TryGetValue(seg.Data.CpuType, out CpuDocumentationData? doc)) {
			return null;
		}

		string opname = (doc.OpComparer != null ? doc.OpComparer(seg.Text) : seg.Text).ToLower();
		if(!doc.OpDesc.TryGetValue(opname, out OpCodeDesc? desc)) {
			return null;
		}

		byte opcode = seg.Data.ByteCode[0];

		StackPanel panel = new StackPanel() { MaxWidth = 250 };
		panel.Children.Add(new TextBlock() { Text = desc.Name, FontSize = 14, Margin = new(2), TextWrapping = Avalonia.Media.TextWrapping.Wrap, FontWeight = Avalonia.Media.FontWeight.Bold });
		panel.Children.Add(new TextBlock() { Text = desc.Description, Margin = new(2), TextWrapping = Avalonia.Media.TextWrapping.Wrap });

		TooltipEntries items = new();
		items.AddCustomEntry("OP", panel);
		items.AddEntry("Byte Code", seg.Data.ByteCodeStr);
		items.AddEntry("Mode", doc.OpMode[opcode]);
		if(doc.OpCycleCount != null) {
			items.AddEntry("Cycle Count", doc.OpCycleCount[opcode]);
		}
		if(desc.Flags != null) {
			items.AddEntry("Affected Flags", desc.Flags.ToString());
		}

		return new DynamicTooltip() { Items = items };
	}

	private static Dictionary<string, OpCodeDesc> Get6502Documentation()
	{
		return new() {
			{ "adc", new OpCodeDesc("ADC - Add with Carry", "Add the value at the specified memory address to the accumulator + the carry bit. On overflow, the carry bit is set.", CpuFlag.Carry | CpuFlag.Zero | CpuFlag.Overflow | CpuFlag.Negative) },
			{ "and", new OpCodeDesc("AND - Bitwise AND", "Perform an AND operation between the accumulator and the value at the specified memory address.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "asl", new OpCodeDesc("ASL - Arithmetic Shift Left", "Shifts all the bits of the accumulator (or the byte at the specified memory address) by 1 bit to the left. Bit 0 will be set to 0 and the carry flag (C) will take the value of bit 7 (before the shift).", CpuFlag.Carry | CpuFlag.Zero | CpuFlag.Negative) },

			{ "bcc", new OpCodeDesc("BCC - Branch if Carry Clear", "If the carry flag (C) is clear, jump to location specified.") },
			{ "bcs", new OpCodeDesc("BCS - Branch if Carry Set", "If the carry flag (C) is set, jump to the location specified.") },
			{ "beq", new OpCodeDesc("BEQ - Branch if Equal", "If the zero flag (Z) is set, jump to the location specified.") },
			{ "bit", new OpCodeDesc("BIT - Bit Test", "Bits 6 and 7 of the byte at the specified memory address are copied to the negative (N) and overflow (V) flags. If the accumulator's value ANDed with that byte is 0, the zero flag (Z) is set (otherwise it is cleared).", CpuFlag.Zero | CpuFlag.Overflow | CpuFlag.Negative) },
			{ "bmi", new OpCodeDesc("BMI - Branch if Minus", "If the negative flag (N) is set, jump to the location specified.") },
			{ "bne", new OpCodeDesc("BNE - Branch if Not Equal", "If the zero flag (Z) is clear, jump to the location specified.") },
			{ "bpl", new OpCodeDesc("BPL - Branch if Positive", "If the negative flag (N) is clear, jump to the location specified.") },
			{ "brk", new OpCodeDesc("BRK - Break", "The BRK instruction causes the CPU to jump to its IRQ vector, as if an interrupt had occurred. The PC and status flags are pushed on the stack.") },
			{ "bvc", new OpCodeDesc("BVC - Branch if Overflow Clear", "If the overflow flag (V) is clear, jump to the location specified.") },
			{ "bvs", new OpCodeDesc("BVS - Branch if Overflow Set", "If the overflow flag (V) is set then, jump to the location specified.") },

			{ "clc", new OpCodeDesc("CLC - Clear carry flag (C)", "Clears the carry flag (C).", CpuFlag.Carry) },
			{ "cld", new OpCodeDesc("CLD - Clear Decimal Mode", "Clears the decimal mode flag (D).", CpuFlag.Decimal) },
			{ "cli", new OpCodeDesc("CLI - Clear Interrupt Disable", "Clears the interrupt disable flag (I).", CpuFlag.Interrupt) },
			{ "clv", new OpCodeDesc("CLV - Clear overflow flag (V)", "Clears the overflow flag (V).", CpuFlag.Overflow) },
			{ "cmp", new OpCodeDesc("CMP - Compare", "Compares the accumulator with the byte at the specified memory address..", CpuFlag.Zero | CpuFlag.Carry | CpuFlag.Negative) },
			{ "cpx", new OpCodeDesc("CPX - Compare X Register", "Compares the X register with the byte at the specified memory address.", CpuFlag.Zero | CpuFlag.Carry | CpuFlag.Negative) },
			{ "cpy", new OpCodeDesc("CPY - Compare Y Register", "Compares the Y register with the byte at the specified memory address.", CpuFlag.Zero | CpuFlag.Carry | CpuFlag.Negative) },

			{ "dec", new OpCodeDesc("DEC - Decrement Memory", "Subtracts one from the byte at the specified memory address.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "dex", new OpCodeDesc("DEX - Decrement X Register", "Subtracts one from the X register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "dey", new OpCodeDesc("DEY - Decrement Y Register", "Subtracts one from the Y register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "eor", new OpCodeDesc("EOR - Exclusive OR", "Performs an exclusive OR operation between the accumulator and the byte at the specified memory address.", CpuFlag.Zero | CpuFlag.Negative) },

			{ "inc", new OpCodeDesc("INC - Increment Memory", "Adds one to the the byte at the specified memory address.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "inx", new OpCodeDesc("INX - Increment X Register", "Adds one to the X register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "iny", new OpCodeDesc("INY - Increment Y Register", "Adds one to the Y register.", CpuFlag.Zero | CpuFlag.Negative) },

			{ "jmp", new OpCodeDesc("JMP - Jump", "Jumps to the specified location (alters the program counter)") },
			{ "jsr", new OpCodeDesc("JSR - Jump to Subroutine", "Pushes the address (minus one) of the next instruction to the stack and then jumps to the target address.") },

			{ "lda", new OpCodeDesc("LDA - Load Accumulator", "Loads a byte from the specified memory address into the accumulator.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "ldx", new OpCodeDesc("LDX - Load X Register", "Loads a byte from the specified memory address into the X register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "ldy", new OpCodeDesc("LDY - Load Y Register", "Loads a byte from the specified memory address into the Y register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "lsr", new OpCodeDesc("LSR - Logical Shift Right", "Shifts all the bits of the accumulator (or the byte at the specified memory address) by 1 bit to the right. Bit 7 will be set to 0 and the carry flag (C) will take the value of bit 0 (before the shift).", CpuFlag.Carry | CpuFlag.Zero | CpuFlag.Negative) },

			{ "nop", new OpCodeDesc("NOP - No Operation", "Performs no operation other than delaying execution of the next instruction by 2 cycles.") },

			{ "ora", new OpCodeDesc("ORA - Inclusive OR", "Performs an inclusive OR operation between the accumulator and the byte at the specified memory address.", CpuFlag.Zero | CpuFlag.Negative) },

			{ "pha", new OpCodeDesc("PHA - Push Accumulator", "Pushes the value of the accumulator to the stack.") },
			{ "php", new OpCodeDesc("PHP - Push Processor Status", "Pushes the value of the status flags to the stack.") },
			{ "pla", new OpCodeDesc("PLA - Pull Accumulator", "Pulls a byte from the stack and stores it into the accumulator.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "plp", new OpCodeDesc("PLP - Pull Processor Status", "Pulls a byte from the stack and stores it into the processor flags.  The flags will be modified based on the value pulled.", CpuFlag.Carry | CpuFlag.Decimal | CpuFlag.Interrupt | CpuFlag.Negative | CpuFlag.Overflow | CpuFlag.Zero) },

			{ "rol", new OpCodeDesc("ROL - Rotate Left", "Shifts all bits 1 position to the left. The right-most bit takes the current value of the carry flag (C). The left-most bit is stored into the carry flag (C).", CpuFlag.Carry | CpuFlag.Zero | CpuFlag.Negative) },
			{ "ror", new OpCodeDesc("ROR - Rotate Right", "Shifts all bits 1 position to the right. The left-most bit takes the current value of the carry flag (C). The right-most bit is stored into the carry flag (C).", CpuFlag.Carry | CpuFlag.Zero | CpuFlag.Negative) },
			{ "rti", new OpCodeDesc("RTI - Return from Interrupt", "The RTI instruction is used at the end of the interrupt handler to return execution to its original location.  It pulls the status flags and program counter from the stack.") },
			{ "rts", new OpCodeDesc("RTS - Return from Subroutine", "The RTS instruction is used at the end of a subroutine to return execution to the calling function. It pulls the status flags and program counter (minus 1) from the stack.") },

			{ "sbc", new OpCodeDesc("SBC - Subtract with Carry", "Substracts the byte at the specified memory address from the value of the accumulator (affected by the carry flag (C)).", CpuFlag.Carry | CpuFlag.Zero | CpuFlag.Overflow | CpuFlag.Negative) },
			{ "sec", new OpCodeDesc("SEC - Set carry flag (C)", "Sets the carry flag (C).", CpuFlag.Carry) },
			{ "sed", new OpCodeDesc("SED - Set Decimal Flag", "Sets the decimal mode flag (D).", CpuFlag.Decimal) },
			{ "sei", new OpCodeDesc("SEI - Set Interrupt Disable", "Sets the interrupt disable flag (I).", CpuFlag.Interrupt) },
			{ "sta", new OpCodeDesc("STA - Store Accumulator", "Stores the value of the accumulator into memory.") },
			{ "stx", new OpCodeDesc("STX - Store X Register", "Stores the value of the X register into memory.") },
			{ "sty", new OpCodeDesc("STY - Store Y Register", "Stores the value of the Y register into memory.") },
			{ "tax", new OpCodeDesc("TAX - Transfer A to X", "Copies the accumulator into the X register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "tay", new OpCodeDesc("TAY - Transfer A to Y", "Copies the accumulator into the Y register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "tsx", new OpCodeDesc("TSX - Transfer SP to X", "Copies the stack pointer into the X register.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "txa", new OpCodeDesc("TXA - Transfer X to A", "Copies the X register into the accumulator.", CpuFlag.Zero | CpuFlag.Negative) },
			{ "txs", new OpCodeDesc("TXS - Transfer X to SP", "Copies the X register into the stack pointer.") },
			{ "tya", new OpCodeDesc("TYA - Transfer Y to A", "Copies the Y register into the accumulator.", CpuFlag.Zero | CpuFlag.Negative) },
		};
	}

	private static void InitNesDocumentation()
	{
		Dictionary<string, OpCodeDesc> desc = Get6502Documentation();

		Dictionary<int, string> mode = new();
		Dictionary<int, string> cycleCount = new();
		for(int i = 0; i < 256; i++) {
			mode[i] = ResourceHelper.GetEnumText(_nesAddressingModes[i]);

			string cycles = _nesOpCycles[i].ToString();
			if(_nesOpCycles[i] != _nesOpCyclesCrossed[i]) {
				cycles += "-" + _nesOpCyclesCrossed[i];
			}
			cycleCount[i] = cycles;
		}

		_data[CpuType.Nes] = new CpuDocumentationData(desc, mode, cycleCount);
	}

	private static void InitSnesDocumentation()
	{
		Dictionary<string, OpCodeDesc> desc = Get6502Documentation();

		desc["cop"] = new OpCodeDesc("COP - Coprocessor", "Jump to the address in the COP vector.");
		desc["jml"] = new OpCodeDesc("JML - Jump Long", "Jumps to the specified 24-bit address");
		desc["jsl"] = new OpCodeDesc("JSL - Jump Subroutine Long", "Jumps to the specified subroutine (24-bit address)");
		
		desc["mvn"] = new OpCodeDesc("MVN - Block Move Negative", "");
		desc["mvp"] = new OpCodeDesc("MVP - Block Move Positive", "");
		
		desc["pea"] = new OpCodeDesc("PEA - Push Absolute Address", "Pushes the specified 16-bit immediate value to the stack.");
		desc["pei"] = new OpCodeDesc("PEI - Push Indirect Address", "Pushes the 16-bit value contained at the specified address to the stack.");
		desc["per"] = new OpCodeDesc("PER - Push PC Relative Address", "Pushes the relative address based on the current PC plus the specified relative value to the stack.");

		desc["rep"] = new OpCodeDesc("REP - Reset Status Bits", "Clears the specified status bits.");
		desc["sep"] = new OpCodeDesc("SEP - Set Status Bits", "Sets the specified status bits");
		
		desc["rtl"] = new OpCodeDesc("RTL - Return from Subroutine Long", "Returns from a subroutine (used with JSL)");
		desc["stp"] = new OpCodeDesc("STP - Stop", "Stops the CPU until reset.");
		desc["wai"] = new OpCodeDesc("WAI - Wait for Interrupt", "Stops the CPU until an IRQ occurs.");
		desc["wdm"] = new OpCodeDesc("WDM - Reserved (NOP)", "");
		
		desc["tcd"] = new OpCodeDesc("TCD - Transfer C Accumulator to Direct Register", "", CpuFlag.Zero | CpuFlag.Negative);
		desc["tcs"] = new OpCodeDesc("TCS - Transfer C Accumulator to Stack Register", "");
		desc["tdc"] = new OpCodeDesc("TDC - Transfer Direct register to C accumulator", "", CpuFlag.Zero | CpuFlag.Negative);
		
		desc["tsc"] = new OpCodeDesc("TSC - Transfer Stack register to C accumulator", "", CpuFlag.Zero | CpuFlag.Negative);
		desc["tsx"] = new OpCodeDesc("TSX - Transfer Stack register to X", "", CpuFlag.Zero | CpuFlag.Negative);
		desc["txy"] = new OpCodeDesc("TXY - Transfer X to Y", "", CpuFlag.Zero | CpuFlag.Negative);
		desc["tyx"] = new OpCodeDesc("TYX - Transfer Y to X", "", CpuFlag.Zero | CpuFlag.Negative);
		
		desc["xba"] = new OpCodeDesc("XBA - Exchange B and A accumulators", "", CpuFlag.Zero | CpuFlag.Negative);
		desc["xce"] = new OpCodeDesc("XCE - Exchange Carry and Emulation flags", "", CpuFlag.Carry | CpuFlag.Emulation);

		desc["tsb"] = new OpCodeDesc("TSB - Test and Set Bit", "The memory value is ORed with the content of A and the flags are updated based on the result.", CpuFlag.Zero | CpuFlag.Negative | CpuFlag.Overflow);
		desc["trb"] = new OpCodeDesc("TRB - Test and Reset Bit", "The memory value is ANDed with the complement of A (~A) and the flags are updated based on the result.", CpuFlag.Zero | CpuFlag.Negative | CpuFlag.Overflow);

		desc["bra"] = new OpCodeDesc("BRA - Branch Always", "Branch to the specified address.");
		desc["brl"] = new OpCodeDesc("BRL - Branch Always Long", "Branch to the specified address.");
		desc["stz"] = new OpCodeDesc("STZ - Store Zero", "Stores the value 0 into memory.");

		desc["phx"] = new OpCodeDesc("PHX - Push X register", "Pushes the value of the X register to the stack.");
		desc["plx"] = new OpCodeDesc("PLX - Pull X register", "Pulls a byte from the stack and stores it into the X register.", CpuFlag.Zero | CpuFlag.Negative);
		desc["phy"] = new OpCodeDesc("PHY - Push Y register", "Pushes the value of the Y register to the stack.");
		desc["ply"] = new OpCodeDesc("PLY - Pull Y register", "Pulls a byte from the stack and stores it into the Y register.", CpuFlag.Zero | CpuFlag.Negative);

		desc["phb"] = new OpCodeDesc("PHB - Push Data Bank Register", "Pushes the value of the Data Bank (B) register to the stack.");
		desc["phd"] = new OpCodeDesc("PHD - Push Direct Register", "Pushes the value of the Direct (D) register to the stack.");
		desc["phk"] = new OpCodeDesc("PHK - Push Program Bank Register", "Pushes the value of the Program Bank (K) register to the stack.");

		desc["plb"] = new OpCodeDesc("PLB - Pull Data Bank Register", "Pulls from the stack and stores the value into the Data Bank (B) register.", CpuFlag.Zero | CpuFlag.Negative);
		desc["pld"] = new OpCodeDesc("PLD - Pull Direct Register", "Pulls from the stack and stores the value into the Direct (D) register.", CpuFlag.Zero | CpuFlag.Negative);

		Dictionary<int, string> mode = new();
		for(int i = 0; i < 256; i++) {
			mode[i] = ResourceHelper.GetEnumText(_snesAddressingModes[i]);
		}
		_data[CpuType.Snes] = new CpuDocumentationData(desc, mode);
	}

	private static void InitPceDocumentation()
	{
		Dictionary<string, OpCodeDesc> desc = Get6502Documentation();

		desc["sxy"] = new OpCodeDesc("SXY - Swap X and Y", "The contents of the X and Y registers are swapped.");
		desc["sax"] = new OpCodeDesc("SAX - Swap A and X", "The contents of the A and X registers are swapped.");
		desc["say"] = new OpCodeDesc("SAY - Swap A and Y", "The contents of the X and Y registers are swapped.");

		desc["cla"] = new OpCodeDesc("CLA - Clear A", "Clears the accumulator.");
		desc["clx"] = new OpCodeDesc("CLX - Clear X", "Clears the X register.");
		desc["cly"] = new OpCodeDesc("CLY - Clear Y", "Clears the Y register.");

		desc["st0"] = new OpCodeDesc("ST0 - Store VDC 0", "Writes the immediate value to the VDC's address register (AR)");
		desc["st1"] = new OpCodeDesc("ST1 - Store VDC 1", "Writes the immediate value to the VDC's selected register (LSB)");
		desc["st2"] = new OpCodeDesc("ST2 - Store VDC 2", "Writes the immediate value to the VDC's selected register (MSB)");
		
		desc["tma"] = new OpCodeDesc("TMA - Transfer MPR to A", "Transfers the value of the selected MPR register (immediate value) to A.");
		desc["tam"] = new OpCodeDesc("TMA - Transfer A to MPR", "The accumulator's content is copied to the selected MPR registers (based on set bits in immediate value)");

		desc["tst"] = new OpCodeDesc("TST - Test memory", "The content of the memory address is ANDed with the immediate value. Flags are updated based on result.", CpuFlag.Overflow | CpuFlag.Negative | CpuFlag.Zero);

		desc["tii"] = new OpCodeDesc("TII - Transfer Block Data", "Transfers block data based on the source, destination and length operands. Source and destination address are incremented after each byte.");
		desc["tdd"] = new OpCodeDesc("TDD - Transfer Block Data", "Transfers block data based on the source, destination and length operands. Source and destination address are decremented after each byte.");
		desc["tin"] = new OpCodeDesc("TIN - Transfer Block Data", "Transfers block data based on the source, destination and length operands. Source is incremented after each byte. Destination is fixed.");
		desc["tia"] = new OpCodeDesc("TIA - Transfer Block Data", "Transfers block data based on the source, destination and length operands. Source is incremented after each byte. Destination is incremented and then decremented (alternates between 2 addresses.)");
		desc["tai"] = new OpCodeDesc("TAI - Transfer Block Data", "Transfers block data based on the source, destination and length operands. Source is incremented and then decremented (alternates between 2 addresses). Destination is incremented after each byte. ");
		
		desc["tsb"] = new OpCodeDesc("TSB - Test and Set Bit", "The memory value is ORed with the content of A and the flags are updated based on the result.", CpuFlag.Zero | CpuFlag.Negative | CpuFlag.Overflow);
		desc["trb"] = new OpCodeDesc("TRB - Test and Reset Bit", "The memory value is ANDed with the complement of A (~A) and the flags are updated based on the result.", CpuFlag.Zero | CpuFlag.Negative | CpuFlag.Overflow);
		
		desc["bsr"] = new OpCodeDesc("BSR - Branch Subroutine", "Branch to the specified address. PC is pushed onto the stack.");
		desc["bra"] = new OpCodeDesc("BRA - Branch Always", "Branch to the specified address.");
		
		desc["csl"] = new OpCodeDesc("CSL - Clock Speed Low", "Sets the CPU's clock speed to ~1.79MHz.");
		desc["csh"] = new OpCodeDesc("CSH - Clock Speed High", "Sets the CPU's clock speed to ~7.16MHz.");
		desc["set"] = new OpCodeDesc("SET - Set T", "Sets the memory flag (T).", CpuFlag.Memory);

		desc["rmb"] = new OpCodeDesc("RMBi - Reset Memory Bit", "Clears the specified bit at the zero page address.");
		desc["smb"] = new OpCodeDesc("SMBi - Set Memory Bit", "Sets the specified bit at the zero page address.");
		desc["bbr"] = new OpCodeDesc("BBRi - Branch on Bit Reset", "Branch if the specified bit at the zero page address is clear.");
		desc["bbs"] = new OpCodeDesc("BBSi - Branch on Bit Set", "Branch if the specified bit at the zero page address is set.");
		
		desc["stz"] = new OpCodeDesc("STZ - Store Zero", "Stores the value 0 into memory.");

		desc["phx"] = new OpCodeDesc("PHX - Push X register", "Pushes the value of the X register to the stack.");
		desc["plx"] = new OpCodeDesc("PLX - Pull X register", "Pulls a byte from the stack and stores it into the X register.", CpuFlag.Zero | CpuFlag.Negative);
		desc["phy"] = new OpCodeDesc("PHY - Push Y register", "Pushes the value of the Y register to the stack.");
		desc["ply"] = new OpCodeDesc("PLY - Pull Y register", "Pulls a byte from the stack and stores it into the Y register.", CpuFlag.Zero | CpuFlag.Negative);

		Dictionary<int, string> mode = new();
		for(int i = 0; i < 256; i++) {
			mode[i] = ResourceHelper.GetEnumText(_pceAddressingModes[i]);
		}
		_data[CpuType.Pce] = new CpuDocumentationData(desc, mode, opComparer: x => x.Substring(0, 3));
	}

	private enum AddrMode
	{
		//6502
		None, Acc, Imp, Imm, Rel,
		Zero, Abs, ZeroX, ZeroY,
		Ind, IndX, IndY, AbsX, AbsY,

		//65816
		AbsLng, AbsIdxXInd, AbsIdxX,
		AbsIdxY, AbsLngIdxX, AbsIndLng,
		AbsInd,
		BlkMov,
		Dir, DirIdxX, DirInd,
		DirIndLng, DirIndIdxY, DirIdxIndX,
		DirIdxY, DirIndLngIdxY,
		Imm8, Imm16, ImmX, ImmM,
		RelLng,
		Stk, StkRel, StkRelIndIdxY,

		//PCE
		AbsXInd,
		Block,
		ImZero, ImZeroX, ImAbs, ImAbsX,
		ZInd, ZeroRel,
	}

	private static AddrMode[] _nesAddressingModes = new AddrMode[256] {
		AddrMode.Imp,  AddrMode.IndX,    AddrMode.None, AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Acc,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX,
		AddrMode.Abs,  AddrMode.IndX,    AddrMode.None, AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Acc,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX,
		AddrMode.Imp,  AddrMode.IndX,    AddrMode.None, AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Acc,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX,
		AddrMode.Imp,  AddrMode.IndX,    AddrMode.None, AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Acc,  AddrMode.Imm,  AddrMode.Ind,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX,
		AddrMode.Imm,  AddrMode.IndX,    AddrMode.Imm,  AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Imp,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroY,   AddrMode.ZeroY,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsY, AddrMode.AbsY,
		AddrMode.Imm,  AddrMode.IndX,    AddrMode.Imm,  AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Imp,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroY,   AddrMode.ZeroY,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsY, AddrMode.AbsY,
		AddrMode.Imm,  AddrMode.IndX,    AddrMode.Imm,  AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Imp,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX,
		AddrMode.Imm,  AddrMode.IndX,    AddrMode.Imm,  AddrMode.IndX,   AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Zero,    AddrMode.Imp,  AddrMode.Imm,  AddrMode.Imp,  AddrMode.Imm,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,  AddrMode.Abs,
		AddrMode.Rel,  AddrMode.IndY,    AddrMode.None, AddrMode.IndY,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.ZeroX,   AddrMode.Imp,  AddrMode.AbsY, AddrMode.Imp,  AddrMode.AbsY, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX, AddrMode.AbsX,
	};

	private static AddrMode[] _snesAddressingModes = new AddrMode[256] {
		AddrMode.Imm8,  AddrMode.DirIdxIndX, AddrMode.Imm8,     AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Stk, AddrMode.ImmM,    AddrMode.Acc, AddrMode.Stk, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // 0
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.Dir,     AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Acc, AddrMode.Imp, AddrMode.Abs,        AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX, // 1
		AddrMode.Abs,   AddrMode.DirIdxIndX, AddrMode.AbsLng,   AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Stk, AddrMode.ImmM,    AddrMode.Acc, AddrMode.Stk, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // 2
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Acc, AddrMode.Imp, AddrMode.AbsIdxX,    AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX, // 3
		AddrMode.Stk,   AddrMode.DirIdxIndX, AddrMode.Imm8,     AddrMode.StkRel,        AddrMode.BlkMov,  AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Stk, AddrMode.ImmM,    AddrMode.Acc, AddrMode.Stk, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // 4
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.BlkMov,  AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Stk, AddrMode.Imp, AddrMode.AbsLng,     AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX, // 5
		AddrMode.Stk,   AddrMode.DirIdxIndX, AddrMode.RelLng,   AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Stk, AddrMode.ImmM,    AddrMode.Acc, AddrMode.Stk, AddrMode.AbsInd,     AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // 6
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Stk, AddrMode.Imp, AddrMode.AbsIdxXInd, AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX, // 7
		AddrMode.Rel,   AddrMode.DirIdxIndX, AddrMode.RelLng,   AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Imp, AddrMode.ImmM,    AddrMode.Imp, AddrMode.Stk, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // 8
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIdxY, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Imp, AddrMode.Imp, AddrMode.Abs,        AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX, // 9
		AddrMode.ImmX,  AddrMode.DirIdxIndX, AddrMode.ImmX,     AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Imp, AddrMode.ImmM,    AddrMode.Imp, AddrMode.Stk, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // A
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIdxY, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Imp, AddrMode.Imp, AddrMode.AbsIdxX,    AddrMode.AbsIdxX, AddrMode.AbsIdxY, AddrMode.AbsLngIdxX, // B
		AddrMode.ImmX,  AddrMode.DirIdxIndX, AddrMode.Imm8,     AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Imp, AddrMode.ImmM,    AddrMode.Imp, AddrMode.Imp, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // C
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.Dir,     AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Stk, AddrMode.Imp, AddrMode.AbsIndLng,  AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX, // D
		AddrMode.ImmX,  AddrMode.DirIdxIndX, AddrMode.Imm8,     AddrMode.StkRel,        AddrMode.Dir,     AddrMode.Dir,     AddrMode.Dir,     AddrMode.DirIndLng,     AddrMode.Imp, AddrMode.ImmM,    AddrMode.Imp, AddrMode.Imp, AddrMode.Abs,        AddrMode.Abs,     AddrMode.Abs,     AddrMode.AbsLng,     // E
		AddrMode.Rel,   AddrMode.DirIndIdxY, AddrMode.DirInd,   AddrMode.StkRelIndIdxY, AddrMode.Imm16,   AddrMode.DirIdxX, AddrMode.DirIdxX, AddrMode.DirIndLngIdxY, AddrMode.Imp, AddrMode.AbsIdxY, AddrMode.Stk, AddrMode.Imp, AddrMode.AbsIdxXInd, AddrMode.AbsIdxX, AddrMode.AbsIdxX, AddrMode.AbsLngIdxX  // F
	};

	private static AddrMode[] _pceAddressingModes = new AddrMode[256] {
		AddrMode.Imm,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.Imm,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Acc,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//0
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.Imm,		AddrMode.Zero,		AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//1
		AddrMode.Abs,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.Imm,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Acc,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//2
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.Imp,		AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.AbsX,		AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//3
		AddrMode.Imp,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.Imm,		AddrMode.Rel,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Acc,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//4
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.Imm,		AddrMode.Imp,		AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Imp,		AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//5
		AddrMode.Imp,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.Imp,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Acc,	AddrMode.Imp,	AddrMode.Ind,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//6
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.Block,	AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.AbsXInd,	AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//7
		AddrMode.Rel,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.ImZero,	AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//8
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.ImAbs,	AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.ZeroY,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//9
		AddrMode.Imm,	AddrMode.IndX,		AddrMode.Imm,	AddrMode.ImZeroX,	AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//A
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.ImAbsX,	AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.ZeroY,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.AbsX,		AddrMode.AbsX,	AddrMode.AbsY,	AddrMode.ZeroRel,	//B
		AddrMode.Imm,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.Block,	AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//C
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.Block,	AddrMode.Imp,		AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Imp,		AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//D
		AddrMode.Imm,	AddrMode.IndX,		AddrMode.Imp,	AddrMode.Block,	AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Zero,		AddrMode.Imp,	AddrMode.Imm,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Abs,		AddrMode.Abs,	AddrMode.Abs,	AddrMode.ZeroRel,	//E
		AddrMode.Rel,	AddrMode.IndY,		AddrMode.ZInd,	AddrMode.Block,	AddrMode.Imp,		AddrMode.ZeroX,	AddrMode.ZeroX,	AddrMode.Zero,		AddrMode.Imp,	AddrMode.AbsY,	AddrMode.Imp,	AddrMode.Imp,	AddrMode.Imp,		AddrMode.AbsX,	AddrMode.AbsX,	AddrMode.ZeroRel,	//F
	};

	private static readonly int[] _nesOpCycles = new int[256] {
		7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
		2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
		2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
		2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
		2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
		2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
		2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		2, 6, 3, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
	};

	private static readonly int[] _nesOpCyclesCrossed = new int[256] {
		7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
		3, 6, 2, 8, 4, 4, 6, 6, 2, 5, 2, 7, 5, 5, 7, 7,
		6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
		3, 6, 2, 8, 4, 4, 6, 6, 2, 5, 2, 7, 5, 5, 7, 7,
		6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
		3, 6, 2, 8, 4, 4, 6, 6, 2, 5, 2, 7, 5, 5, 7, 7,
		6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
		3, 6, 2, 8, 4, 4, 6, 6, 2, 5, 2, 7, 5, 5, 7, 7,
		2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
		3, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
		2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
		3, 6, 2, 5, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
		2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		3, 6, 2, 8, 4, 4, 6, 6, 2, 5, 2, 7, 5, 5, 7, 7,
		2, 6, 3, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		3, 6, 2, 8, 4, 4, 6, 6, 2, 5, 2, 7, 5, 5, 7, 7,
	};

	[Flags]
	private enum CpuFlag { None = 0, Carry = 1, Decimal = 2, Interrupt = 4, Negative = 8, Overflow = 16, Zero = 32, Memory = 64, Index = 128, Emulation = 256 }

	private class OpCodeDesc
	{
		internal string Name { get; set; }
		internal string Description { get; set; }
		internal Enum? Flags { get; set; }

		internal OpCodeDesc(string name, string desc, Enum? flags = null)
		{
			Name = name;
			Description = desc;
			Flags = flags;
		}
	}

	private class CpuDocumentationData
	{
		public Dictionary<string, OpCodeDesc> OpDesc { get; }
		public Dictionary<int, string> OpMode { get; }
		public Dictionary<int, string>? OpCycleCount { get; }
		public Func<string, string>? OpComparer { get; }

		public CpuDocumentationData(Dictionary<string, OpCodeDesc> opDesc, Dictionary<int, string> opMode, Dictionary<int, string>? opCycleCount = null, Func<string, string>? opComparer = null)
		{
			OpDesc = opDesc;
			OpMode = opMode;
			OpCycleCount = opCycleCount;
			OpComparer = opComparer;
		}
	}
}