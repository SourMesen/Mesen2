using Mesen.Interop;

namespace Mesen.Debugger.Integration
{
	public class SnesDbgImporter : DbgImporter
	{
		public SnesDbgImporter(RomFormat romFormat) : base(CpuType.Snes, romFormat, new() { MemoryType.SnesPrgRom, MemoryType.SnesWorkRam, MemoryType.SnesSaveRam, MemoryType.SpcRam })
		{
		}

		protected override CdlFlags GetOpFlags(byte opCode, int opSize)
		{
			if(opSize == 2) {
				if(IsVarWidthMemoryInstruction(opCode)) {
					//8-bit immediate memory operation, set M flag
					return CdlFlags.MemoryMode8;
				} else if(IsVarWidthIndexInstruction(opCode)) {
					//8-bit immediate index operation, set X flag
					return CdlFlags.IndexMode8;
				}
			}

			return CdlFlags.None;
		}

		protected override bool IsBranchInstruction(byte opCode)
		{
			return opCode == 0x20 || opCode == 0x10 || opCode == 0x30 || opCode == 0x50 || opCode == 0x70 || opCode == 0x80 || opCode == 0x90 || opCode == 0xB0 || opCode == 0xD0 || opCode == 0xF0 || opCode == 0x4C || opCode == 0x20 || opCode == 0x4C || opCode == 0x5C || opCode == 0x6C;
		}

		protected override bool IsJumpToSubroutine(byte opCode)
		{
			return opCode == 0x20 || opCode == 0x22; //JSR/JSL
		}

		private bool IsVarWidthIndexInstruction(byte opCode)
		{
			return opCode == 0xA0 || opCode == 0xA2 || opCode == 0xC0 || opCode == 0xE0;
		}

		private bool IsVarWidthMemoryInstruction(byte opCode)
		{
			return opCode == 0x09 || opCode == 0x29 || opCode == 0x49 || opCode == 0x69 || opCode == 0x89 || opCode == 0xA9 || opCode == 0xC9 || opCode == 0xE9;
		}
	}
}
