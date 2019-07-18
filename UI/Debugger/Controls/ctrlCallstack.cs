using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Debugger.Labels;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlCallstack : BaseControl
	{
		public delegate void NavigateToAddressHandler(UInt32 address);
		public event NavigateToAddressHandler FunctionSelected;

		private StackFrameInfo[] _stackFrames;
		private UInt32 _programCounter;
		private string _format = "X6";
		private CpuType _cpuType;

		public ctrlCallstack()
		{
			InitializeComponent();
		}

		public void UpdateCallstack(CpuType cpuType)
		{
			_cpuType = cpuType;
			_format = cpuType == CpuType.Cpu ? "X6" : "X4";
			List<StackInfo> stack = GetStackInfo();
			this.UpdateList(stack);
		}

		private List<StackInfo> GetStackInfo()
		{
			_stackFrames = DebugApi.GetCallstack(_cpuType);
			DebugState state = DebugApi.GetState();

			if(_cpuType == CpuType.Cpu) {
				_programCounter = (uint)(state.Cpu.K << 16) | state.Cpu.PC;
			} else {
				_programCounter = (uint)state.Spc.PC;
			}

			int relDestinationAddr = -1;

			List<StackInfo> stack = new List<StackInfo>();
			for(int i = 0, len = _stackFrames.Length; i < len; i++) {
				int relSubEntryAddr = i == 0 ? -1 : (int)_stackFrames[i-1].Target;

				stack.Add(new StackInfo() {
					SubName = this.GetFunctionName(relSubEntryAddr, i == 0 ? StackFrameFlags.None : _stackFrames[i - 1].Flags),
					Address = _stackFrames[i].Source,
				});

				relDestinationAddr = (int)_stackFrames[i].Target;
			}

			//Add current location
			stack.Add(new StackInfo() {
				SubName = this.GetFunctionName(relDestinationAddr, _stackFrames.Length == 0 ? StackFrameFlags.None : _stackFrames[_stackFrames.Length - 1].Flags),
				Address = _programCounter,
			});

			return stack;
		}

		private void UpdateList(List<StackInfo> stack)
		{
			this.lstCallstack.BeginUpdate();
			while(this.lstCallstack.Items.Count > stack.Count) {
				this.lstCallstack.Items.RemoveAt(this.lstCallstack.Items.Count - 1);
			}
			while(this.lstCallstack.Items.Count < stack.Count) {
				this.lstCallstack.Items.Add("").SubItems.AddRange(new string[] { "", "" });
			}

			for(int i = 0, len = stack.Count; i < len; i++) {
				StackInfo stackInfo = stack[i];
				ListViewItem item = this.lstCallstack.Items[len - i - 1];

				item.Text = stackInfo.SubName;
				item.SubItems[1].Text = "@ $" + stackInfo.Address.ToString(_format);
				item.Font = new Font(item.Font, FontStyle.Regular);
			}
			this.lstCallstack.EndUpdate();
		}
		
		private string GetFunctionName(int relSubEntryAddr, StackFrameFlags flags)
		{
			if(relSubEntryAddr < 0) {
				return "[bottom of stack]";
			}

			CodeLabel label = relSubEntryAddr >= 0 ? LabelManager.GetLabel(new AddressInfo() { Address = relSubEntryAddr, Type = _cpuType.ToMemoryType() }) : null;
			if(label != null) {
				return label.Label + " ($" + relSubEntryAddr.ToString(_format) + ")";
			} else if(flags == StackFrameFlags.Nmi) {
				return "[nmi] $" + relSubEntryAddr.ToString(_format);
			} else if(flags == StackFrameFlags.Irq) {
				return "[irq] $" + relSubEntryAddr.ToString(_format);
			}

			return "$" + relSubEntryAddr.ToString(_format);
		}

		private void lstCallstack_DoubleClick(object sender, EventArgs e)
		{
			if(this.lstCallstack.SelectedIndices.Count > 0) {
				if(this.lstCallstack.SelectedIndices[0] == 0) {
					this.FunctionSelected(_programCounter);
				} else {
					StackFrameInfo stackFrameInfo = _stackFrames[(this.lstCallstack.Items.Count - 1 - this.lstCallstack.SelectedIndices[0])];
					this.FunctionSelected?.Invoke((UInt32)stackFrameInfo.Source);
				}
			}
		}

		private class StackInfo
		{
			public string SubName;
			public UInt32 Address;
		}
	}
}
