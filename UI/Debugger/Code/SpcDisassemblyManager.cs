using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class SpcDisassemblyManager : CpuDisassemblyManager
	{
		public override SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.SpcMemory; } }
		public override int AddressSize { get { return 4; } }
		public override int ByteCodeSize { get { return 3; } }

		public override void RefreshCode()
		{
			this._provider = new CodeDataProvider(CpuType.Spc);
		}
	}
}
