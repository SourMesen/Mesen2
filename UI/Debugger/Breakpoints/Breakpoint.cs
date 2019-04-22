using System;
using System.Text;

namespace Mesen.GUI.Debugger
{
	public class Breakpoint
	{
		private SnesMemoryType _memoryType = SnesMemoryType.CpuMemory;
		private bool _isCpuBreakpoint = true;

		public bool BreakOnRead = false;
		public bool BreakOnWrite = false;
		public bool BreakOnExec = true;

		public bool Enabled = true;
		public bool MarkEvent = false;
		public UInt32 Address = UInt32.MaxValue;
		public UInt32 StartAddress;
		public UInt32 EndAddress;
		public BreakpointAddressType AddressType = BreakpointAddressType.SingleAddress;
		public string Condition = "";
		
		public SnesMemoryType MemoryType
		{
			get { return _memoryType; }
			set
			{
				_memoryType = value;
				_isCpuBreakpoint = IsTypeCpuBreakpoint(value);
			}
		}

		public string GetAddressString(bool showLabel)
		{
			string addr = "";
			string format = _memoryType == SnesMemoryType.SpcMemory ? "X4" : "X6";
			switch(AddressType) {
				case BreakpointAddressType.AnyAddress:
					return "<any>";
				case BreakpointAddressType.SingleAddress:
					if(IsAbsoluteAddress) {
						addr += $"[${Address.ToString(format)}]";
					} else {
						addr = $"${Address.ToString(format)}";
					}
					break;

				case BreakpointAddressType.AddressRange:
					if(IsAbsoluteAddress) {
						addr = $"[${StartAddress.ToString(format)}] - [${EndAddress.ToString(format)}]";
					} else {
						addr = $"${StartAddress.ToString(format)} - ${EndAddress.ToString(format)}";
					}
					break;
			}

			//TODO LABELS
			/*string label = GetAddressLabel();
			if(showLabel && !string.IsNullOrWhiteSpace(label)) {
				addr = label + $", {addr}";
			}*/
			return addr;
		}

		public static bool IsTypeCpuBreakpoint(SnesMemoryType type)
		{
			return (
				type == SnesMemoryType.CpuMemory ||
				type == SnesMemoryType.SpcMemory ||
				type == SnesMemoryType.WorkRam ||
				type == SnesMemoryType.SaveRam ||
				type == SnesMemoryType.PrgRom
			);
		}

		public void SetEnabled(bool enabled)
		{
			Enabled = enabled;
			BreakpointManager.RefreshBreakpoints(this);
		}

		public void SetMarked(bool marked)
		{
			MarkEvent = marked;
			BreakpointManager.RefreshBreakpoints(this);
		}

		public bool IsAbsoluteAddress { get { return MemoryType != SnesMemoryType.CpuMemory && MemoryType != SnesMemoryType.SpcMemory; } }
		public bool IsCpuBreakpoint { get { return this._isCpuBreakpoint; } }

		private BreakpointTypeFlags Type
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

		public string ToReadableType()
		{
			string type;

			switch(MemoryType) {
				default:	throw new Exception("invalid type");
				case SnesMemoryType.CpuMemory: type = "CPU"; break;
				case SnesMemoryType.SpcMemory: type = "SPC"; break;
				case SnesMemoryType.PrgRom: type = "PRG"; break;
				case SnesMemoryType.WorkRam: type = "WRAM"; break;
				case SnesMemoryType.SaveRam: type = "SRAM"; break;
				case SnesMemoryType.VideoRam: type = "VRAM"; break;
				case SnesMemoryType.SpriteRam: type = "OAM"; break;
				case SnesMemoryType.CGRam: type = "CG"; break;
			}

			type += ":";
			type += BreakOnRead ? "R" : "‒";
			type += BreakOnWrite ? "W" : "‒";
			if(IsCpuBreakpoint) {
				type += BreakOnExec ? "X" : "‒";
			}
			return type;
		}

		public int GetRelativeAddress()
		{
			UInt32 address = AddressType == BreakpointAddressType.SingleAddress ? this.Address : this.StartAddress;
			if(IsCpuBreakpoint && this.IsAbsoluteAddress) {
				//TODO
				//return InteropEmu.DebugGetRelativeAddress(address, this.MemoryType.ToAddressType());
				return -1;
			} else {
				return (int)address;
			}
		}

		private int GetRelativeAddressEnd()
		{
			if(this.AddressType == BreakpointAddressType.AddressRange){
				if(IsCpuBreakpoint && this.IsAbsoluteAddress) {
					//TODO
					//return InteropEmu.DebugGetRelativeAddress(this.EndAddress, this.MemoryType.ToAddressType());
				} else {
					return (int)this.EndAddress;
				}
			}
			return -1;
		}

		public CpuType GetCpuType()
		{
			return _memoryType == SnesMemoryType.SpcMemory ? CpuType.Spc : CpuType.Cpu;
		}

		public bool Matches(CpuType type)
		{
			return GetCpuType() == type;		
		}

		public bool Matches(UInt32 address, SnesMemoryType type)
		{
			if(IsTypeCpuBreakpoint(type) != this.IsCpuBreakpoint) {
				return false;
			}

			if(this.AddressType == BreakpointAddressType.SingleAddress) {
				return address == this.Address && type == this.MemoryType;
			} else if(this.AddressType == BreakpointAddressType.AddressRange) {
				return address >= this.StartAddress && address <= this.EndAddress && type == this.MemoryType;
			}

			return false;
		}

		public InteropBreakpoint ToInteropBreakpoint(int breakpointId)
		{
			InteropBreakpoint bp = new InteropBreakpoint() {
				Id = breakpointId,
				MemoryType = MemoryType,
				Type = Type,
				MarkEvent = MarkEvent,
				Enabled = Enabled
			};
			switch(AddressType) {
				case BreakpointAddressType.AnyAddress:
					bp.StartAddress = -1;
					bp.EndAddress = -1;
					break;

				case BreakpointAddressType.SingleAddress:
					bp.StartAddress = (Int32)Address;
					bp.EndAddress = -1;
					break;

				case BreakpointAddressType.AddressRange:
					bp.StartAddress = (Int32)StartAddress;
					bp.EndAddress = (Int32)EndAddress;
					break;
			}

			bp.Condition = new byte[1000];
			byte[] condition = Encoding.UTF8.GetBytes(Condition.Replace(Environment.NewLine, " "));
			Array.Copy(condition, bp.Condition, condition.Length);
			return bp;
		}
	}

	public enum BreakpointAddressType
	{
		AnyAddress,
		SingleAddress,
		AddressRange,
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