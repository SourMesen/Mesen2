using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class SpcDisassemblyManager : IDisassemblyManager
	{
		private CodeDataProvider _provider;

		public ICodeDataProvider Provider { get { return this._provider; } }
		public int AddressSize { get { return 4; } }
		public int ByteCodeSize { get { return 3; } }

		public void RefreshCode()
		{
			this._provider = new CodeDataProvider(CpuType.Spc);
		}

		public void ToggleBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				BreakpointManager.ToggleBreakpoint(new AddressInfo() {
					Address = address,
					Type = SnesMemoryType.SpcMemory
				});
			}
		}

		public void EnableDisableBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				BreakpointManager.EnableDisableBreakpoint(new AddressInfo() {
					Address = address,
					Type = SnesMemoryType.SpcMemory
				});
			}
		}
	}
}
