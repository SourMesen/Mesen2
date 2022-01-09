using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
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
		public CpuType CpuType { get; }
		public DisassemblyViewModel Disassembly { get; }

		[Reactive] public List<StackInfo> CallStackContent { get; private set; } = new List<StackInfo>();

		private StackFrameInfo[] _stackFrames = Array.Empty<StackFrameInfo>();

		[Obsolete("For designer only")]
		public CallStackViewModel() : this(CpuType.Cpu, new()) { }

		public CallStackViewModel(CpuType cpuType, DisassemblyViewModel disassembly)
		{
			Disassembly = disassembly;
			CpuType = cpuType;
			Id = "CallStack";
			Title = "Call Stack";
			CanPin = false;

			if(Design.IsDesignMode) {
				return;
			}

			UpdateCallStack();
		}

		public void UpdateCallStack()
		{
			_stackFrames = DebugApi.GetCallstack(CpuType);
			RefreshCallStack();
		}

		public void RefreshCallStack()
		{
			CallStackContent = GetStackInfo();
		}

		private List<StackInfo> GetStackInfo()
		{
			StackFrameInfo[] stackFrames = _stackFrames;

			List<StackInfo> stack = new List<StackInfo>();
			for(int i = 0; i < stackFrames.Length; i++) {
				bool isMapped = DebugApi.GetRelativeAddress(stackFrames[i].AbsSource, CpuType).Address >= 0;
				stack.Insert(0, new StackInfo() {
					EntryPoint = GetEntryPoint(i == 0 ? null : stackFrames[i - 1]),
					RelAddress = stackFrames[i].Source,
					Address = stackFrames[i].AbsSource,
					RowBrush = isMapped ? AvaloniaProperty.UnsetValue : Brushes.Gray,
					RowStyle = isMapped ? FontStyle.Normal : FontStyle.Italic
				});
			}

			//Add current location
			stack.Insert(0, new StackInfo() {
				EntryPoint = GetEntryPoint(stackFrames.Length > 0 ? stackFrames[^1] : null),
				RelAddress = DebugUtilities.GetProgramCounter(CpuType),
				Address = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)DebugUtilities.GetProgramCounter(CpuType), Type = CpuType.ToMemoryType() })
			});

			return stack;
		}

		private string GetEntryPoint(StackFrameInfo? stackFrame)
		{
			if(stackFrame == null) {
				return "[bottom of stack]";
			}

			StackFrameInfo entry = stackFrame.Value;

			string format = "X" + CpuType.GetAddressSize();
			CodeLabel? label = LabelManager.GetLabel(entry.AbsTarget);
			if(label != null) {
				return label.Label + " ($" + entry.Target.ToString(format) + ")";
			} else if(entry.Flags == StackFrameFlags.Nmi) {
				return "[nmi] $" + entry.Target.ToString(format);
			} else if(entry.Flags == StackFrameFlags.Irq) {
				return "[irq] $" + entry.Target.ToString(format);
			}

			return "$" + entry.Target.ToString(format);
		}

		public class StackInfo
		{
			public string EntryPoint { get; set; } = "";
			public UInt32 RelAddress { get; set; }
			public AddressInfo Address { get; set; }

			public object RowBrush { get; set; } = AvaloniaProperty.UnsetValue;
			public FontStyle RowStyle { get; set; }
		}
	}
}
