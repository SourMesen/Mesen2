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
		protected CodeDataProvider _provider;
		public ICodeDataProvider Provider { get { return this._provider; } }

		public virtual SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.CpuMemory; } }
		public virtual int AddressSize { get { return 6; } }
		public virtual int ByteCodeSize { get { return 4; } }

		public virtual void RefreshCode()
		{
			this._provider = new CodeDataProvider(CpuType.Cpu);
		}

		public void ToggleBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				AddressInfo relAddress = new AddressInfo() {
					Address = address,
					Type = RelativeMemoryType
				};

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
				if(absAddress.Address < 0) {
					BreakpointManager.ToggleBreakpoint(relAddress);
				} else {
					BreakpointManager.ToggleBreakpoint(absAddress);
				}
			}
		}

		public void EnableDisableBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				BreakpointManager.EnableDisableBreakpoint(new AddressInfo() {
					Address = address,
					Type = RelativeMemoryType
				});
			}
		}
	}
}
