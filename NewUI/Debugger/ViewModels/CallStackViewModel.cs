using Dock.Model.ReactiveUI.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Mesen.Debugger.ViewModels
{
	public class CallStackViewModel : Tool
	{
		private CpuType _cpuType;

		[Reactive] public List<StackInfo> StackFrames { get; private set; } = new List<StackInfo>();

		//For designer
		public CallStackViewModel() : this(CpuType.Cpu) { }

		public CallStackViewModel(CpuType cpuType)
		{
			_cpuType = cpuType;
			Id = "CallStack";
			Title = "Call Stack";
			UpdateCallStack();
		}

		public void UpdateCallStack()
		{
			StackFrames = GetStackInfo();
		}

		private List<StackInfo> GetStackInfo()
		{
			StackFrameInfo[] stackFrames = DebugApi.GetCallstack(_cpuType);

			int relDestinationAddr = -1;

			List<StackInfo> stack = new List<StackInfo>();
			for(int i = 0, len = stackFrames.Length; i < len; i++) {
				int relSubEntryAddr = i == 0 ? -1 : (int)stackFrames[i - 1].Target;

				stack.Insert(0, new StackInfo() {
					SubName = this.GetFunctionName(relSubEntryAddr, i == 0 ? StackFrameFlags.None : stackFrames[i - 1].Flags),
					Address = stackFrames[i].Source,
				});

				relDestinationAddr = (int)stackFrames[i].Target;
			}

			//Add current location
			stack.Insert(0, new StackInfo() {
				SubName = this.GetFunctionName(relDestinationAddr, stackFrames.Length == 0 ? StackFrameFlags.None : stackFrames[^1].Flags),
				Address = DebugUtilities.GetProgramCounter(_cpuType),
			});

			return stack;
		}

		private string GetFunctionName(int relSubEntryAddr, StackFrameFlags flags)
		{
			if(relSubEntryAddr < 0) {
				return "[bottom of stack]";
			}

			string format = "X" + _cpuType.GetAddressSize();
			CodeLabel? label = relSubEntryAddr >= 0 ? LabelManager.GetLabel(new AddressInfo() { Address = relSubEntryAddr, Type = _cpuType.ToMemoryType() }) : null;
			if(label != null) {
				return label.Label + " ($" + relSubEntryAddr.ToString(format) + ")";
			} else if(flags == StackFrameFlags.Nmi) {
				return "[nmi] $" + relSubEntryAddr.ToString(format);
			} else if(flags == StackFrameFlags.Irq) {
				return "[irq] $" + relSubEntryAddr.ToString(format);
			}

			return "$" + relSubEntryAddr.ToString(format);
		}

		public class StackInfo
		{
			public string SubName { get; set; }
			public UInt32 Address { get; set; }
		}
	}
}
