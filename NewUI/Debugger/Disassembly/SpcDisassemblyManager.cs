using Mesen.Debugger.Integration;
using Mesen.GUI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Disassembly
{
	public class SpcDisassemblyManager : CpuDisassemblyManager
	{
		public override CpuType CpuType { get { return CpuType.Spc; } }
		public override SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.SpcMemory; } }
		public override int AddressSize { get { return 4; } }
		public override int ByteCodeSize { get { return 3; } }
		public override bool AllowSourceView { get { return false; } }

		public override void RefreshCode(ISymbolProvider symbolProvider, SourceFileInfo file)
		{
			this._provider = new CodeDataProvider(CpuType.Spc);
		}

		protected override int GetFullAddress(int address, int length)
		{
			SpcState state = DebugApi.GetState().Spc;
			if(length == 2) {
				//Determine address based on direct page flag
				return (state.PS.HasFlag(SpcFlags.DirectPage) ? 0x100 : 0) | address;
			} 
			return address;
		}
	}
}
