using Mesen.Debugger.Integration;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Disassembly
{
	public class GbDisassemblyManager : CpuDisassemblyManager
	{
		public override CpuType CpuType { get { return CpuType.Gameboy; } }
		public override SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.GameboyMemory; } }
		public override SnesMemoryType PrgMemoryType { get { return SnesMemoryType.GbPrgRom; } }
		public override int AddressSize { get { return 4; } }
		public override int ByteCodeSize { get { return 3; } }
		public override bool AllowSourceView { get { return false; } }

		public override void RefreshCode(ISymbolProvider symbolProvider, SourceFileInfo? file)
		{
			this._provider = new CodeDataProvider(CpuType.Gameboy);
		}

		protected override int GetFullAddress(int address, int length)
		{
			return address;
		}
	}
}
