using Mesen.Debugger.Integration;
using Mesen.Interop;

namespace Mesen.Debugger.Disassembly
{
	public class GsuDisassemblyManager : CpuDisassemblyManager
	{
		public override CpuType CpuType { get { return CpuType.Gsu; } }
		public override MemoryType RelativeMemoryType { get { return MemoryType.GsuMemory; } }
		public override int AddressSize { get { return 6; } }
		public override int ByteCodeSize { get { return 3; } }
		public override bool AllowSourceView { get { return false; } }

		public override void RefreshCode(ISymbolProvider symbolProvider, SourceFileInfo? file)
		{
			this._provider = new CodeDataProvider(CpuType.Gsu);
		}

		protected override int GetFullAddress(int address, int length)
		{
			return address;
		}
	}
}
