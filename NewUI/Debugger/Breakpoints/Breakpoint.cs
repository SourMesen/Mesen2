using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;
using System.Text;

namespace Mesen.Debugger
{
	public class Breakpoint : ReactiveObject
	{
		[Reactive] public bool BreakOnRead { get; set; }
		[Reactive] public bool BreakOnWrite { get; set; }
		[Reactive] public bool BreakOnExec { get; set; }

		[Reactive] public bool Enabled { get; set; } = true;
		[Reactive] public bool MarkEvent { get; set; }
		[Reactive] public SnesMemoryType MemoryType { get; set; }
		[Reactive] public UInt32 StartAddress { get; set; }
		[Reactive] public UInt32 EndAddress { get; set; }
		[Reactive] public CpuType CpuType { get; set; }
		[Reactive] public bool AnyAddress { get; set; } = false;
		[Reactive] public string Condition { get; set; } = "";

		public Breakpoint()
		{
		}

		public bool IsAbsoluteAddress { get { return !MemoryType.IsRelativeMemory(); } }
		public bool IsCpuBreakpoint { get { return Breakpoint.IsTypeCpuBreakpoint(MemoryType); } }

		public BreakpointTypeFlags Type
		{
			get
			{
				BreakpointTypeFlags type = BreakpointTypeFlags.None;
				if(BreakOnRead) {
					type |= BreakpointTypeFlags.Read;
				}
				if(BreakOnWrite) {
					type |= BreakpointTypeFlags.Write;
				}
				if(BreakOnExec && IsCpuBreakpoint) {
					type |= BreakpointTypeFlags.Execute;
				}
				return type;
			}
		}

		public static bool IsTypeCpuBreakpoint(SnesMemoryType type)
		{
			return type != SnesMemoryType.Register && !type.IsPpuMemory();
		}

		public bool Matches(UInt32 address, SnesMemoryType type, CpuType? cpuType)
		{
			if((cpuType.HasValue && cpuType.Value != CpuType)) {
				return false;
			}

			return type == MemoryType && address >= StartAddress && address <= EndAddress;
		}

		public int GetRelativeAddress()
		{
			if(IsCpuBreakpoint && this.IsAbsoluteAddress) {
				return DebugApi.GetRelativeAddress(new AddressInfo() { Address = (int)StartAddress, Type = this.MemoryType }, this.CpuType).Address;
			} else {
				return (int)StartAddress;
			}
		}

		private int GetRelativeAddressEnd()
		{
			if(StartAddress != EndAddress) {
				if(IsCpuBreakpoint && this.IsAbsoluteAddress) {
					return DebugApi.GetRelativeAddress(new AddressInfo() { Address = (int)this.EndAddress, Type = this.MemoryType }, this.CpuType).Address;
				} else {
					return (int)this.EndAddress;
				}
			}
			return -1;
		}

		public InteropBreakpoint ToInteropBreakpoint(int breakpointId)
		{
			InteropBreakpoint bp = new InteropBreakpoint() {
				Id = breakpointId,
				CpuType = CpuType,
				MemoryType = MemoryType,
				Type = Type,
				MarkEvent = MarkEvent,
				Enabled = Enabled,
				StartAddress = (Int32)StartAddress,
				EndAddress = (Int32)EndAddress
			};

			bp.Condition = new byte[1000];
			byte[] condition = Encoding.UTF8.GetBytes(Condition.Replace(Environment.NewLine, " "));
			Array.Copy(condition, bp.Condition, condition.Length);
			return bp;
		}

		public string GetAddressString(bool showLabel)
		{
			string addr = "";
			string format = "X" + CpuType.GetAddressSize();
			if(StartAddress == EndAddress) {
				addr += $"${StartAddress.ToString(format)}";
			} else {
				addr = $"${StartAddress.ToString(format)} - ${EndAddress.ToString(format)}";
			}

			if(showLabel) {
				string label = GetAddressLabel();
				if(!string.IsNullOrWhiteSpace(label)) {
					addr += " [" + label + "]";
				}
			}
			return addr;
		}

		public string GetAddressLabel()
		{
			if(IsCpuBreakpoint) {
				CodeLabel? label = LabelManager.GetLabel(new AddressInfo() { Address = (int)StartAddress, Type = MemoryType });
				return label?.Label ?? string.Empty;
			}
			return string.Empty;
		}

		public string ToReadableType()
		{
			string type = MemoryType.GetShortName();
			type += ":";
			type += BreakOnRead ? "R" : "‒";
			type += BreakOnWrite ? "W" : "‒";
			if(IsCpuBreakpoint) {
				type += BreakOnExec ? "X" : "‒";
			}
			return type;
		}

		public Breakpoint Clone()
		{
			return JsonHelper.Clone(this);
		}

		public void CopyFrom(Breakpoint copy)
		{
			StartAddress = copy.StartAddress;
			EndAddress = copy.EndAddress;
			MemoryType = copy.MemoryType;
			MarkEvent = copy.MarkEvent;
			Enabled = copy.Enabled;
			Condition = copy.Condition;
			BreakOnExec = copy.BreakOnExec;
			BreakOnRead = copy.BreakOnRead;
			BreakOnWrite = copy.BreakOnWrite;
			CpuType = copy.CpuType;
		}
	}

	[Flags]
	public enum BreakpointTypeFlags
	{
		None = 0,
		Execute = 1,
		Read = 2,
		Write = 4,
	}
}