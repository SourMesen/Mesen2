using Mesen.Interop;

namespace Mesen.Debugger.Integration
{
	public class NesDbgImporter : DbgImporter
	{
		public NesDbgImporter(RomFormat romFormat) : base(CpuType.Nes, romFormat, new() { MemoryType.NesPrgRom, MemoryType.NesWorkRam, MemoryType.NesSaveRam, MemoryType.NesInternalRam })
		{
		}

		protected override CdlFlags GetOpFlags(byte opCode, int opSize)
		{
			return CdlFlags.None;
		}

		protected override bool IsBranchInstruction(byte opCode)
		{
			return opCode == 0x20 || opCode == 0x10 || opCode == 0x30 || opCode == 0x50 || opCode == 0x70 || opCode == 0x90 || opCode == 0xB0 || opCode == 0xD0 || opCode == 0xF0 || opCode == 0x4C || opCode == 0x20 || opCode == 0x4C || opCode == 0x6C;
		}

		protected override bool IsJumpToSubroutine(byte opCode)
		{
			return opCode == 0x20; //JSR/JSL
		}
	}
}
