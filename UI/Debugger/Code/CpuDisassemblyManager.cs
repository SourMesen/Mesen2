using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class CpuDisassemblyManager : IDisassemblyManager
	{
		private CpuCodeDataProvider _provider;

		public ICodeDataProvider Provider { get { return this._provider; } }

		public void RefreshCode()
		{
			this._provider = new CpuCodeDataProvider();
		}

		public void ToggleBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				BreakpointManager.ToggleBreakpoint(new AddressInfo() {
					Address = address,
					Type = SnesMemoryType.CpuMemory
				});
			}
		}
	}
}
