using Mesen.Interop;

namespace Mesen.Debugger.Integration
{
	public class PceDbgImporter : DbgImporter
	{
		public PceDbgImporter(RomFormat romFormat) : base(CpuType.Pce, romFormat, new() { MemoryType.PcePrgRom, MemoryType.PceWorkRam, MemoryType.PceSaveRam, MemoryType.PceCdromRam, MemoryType.PceCardRam })
		{
		}

		protected override CdlFlags GetOpFlags(byte opCode, int opSize)
		{
			return CdlFlags.None;
		}

		protected override bool IsBranchInstruction(byte opCode)
		{
			switch(opCode) {
				case 0x20: //JSR
				case 0x40: //RTI
				case 0x44: //BSR
				case 0x4C: //JMP (Absolute)
				case 0x60: //RTS
				case 0x6C: //JMP (Indirect)
				case 0x7C: //JMP (Absolute,X)
				case 0x80: //BRA

				case 0x10: //BPL
				case 0x30: //BMI
				case 0x50: //BVC
				case 0x70: //BVS
				case 0x90: //BCC
				case 0xB0: //BCS
				case 0xD0: //BNE
				case 0xF0: //BEQ

				//BBR
				case 0x0F:
				case 0x1F:
				case 0x2F:
				case 0x3F:
				case 0x4F:
				case 0x5F:
				case 0x6F:
				case 0x7F:

				//BBS
				case 0x8F:
				case 0x9F:
				case 0xAF:
				case 0xBF:
				case 0xCF:
				case 0xDF:
				case 0xEF:
				case 0xFF:
					return true;
			}

			return false;
		}

		protected override bool IsJumpToSubroutine(byte opCode)
		{
			return opCode == 0x20 || opCode == 0x44 || opCode == 0x00;
		}
	}
}
