using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class Cx4DisassemblyManager : CpuDisassemblyManager
	{
		public override CpuType CpuType { get { return CpuType.Cx4; } }
		public override SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.Cx4Memory; } }
		public override int AddressSize { get { return 6; } }
		public override int ByteCodeSize { get { return 4; } }
		public override bool AllowSourceView { get { return false; } }

		public override void RefreshCode(ISymbolProvider symbolProvider, SourceFileInfo file)
		{
			this._provider = new CodeDataProvider(CpuType.Cx4);
		}

		protected override int GetFullAddress(int address, int length)
		{
			return address;
		}
	}
}
