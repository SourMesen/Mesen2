using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;

namespace Mesen.Debugger
{
	public class Breakpoint : ReactiveObject
	{
		[Reactive] public bool BreakOnRead { get; set; }
		[Reactive] public bool BreakOnWrite { get; set; }
		[Reactive] public bool BreakOnExec { get; set; }

		[Reactive] public bool Enabled { get; set; }
		[Reactive] public bool MarkEvent { get; set; }
		[Reactive] public SnesMemoryType MemoryType { get; set; }
		[Reactive] public UInt32 StartAddress { get; set; }
		[Reactive] public UInt32 EndAddress { get; set; }
		[Reactive] public CpuType CpuType { get; set; }
		[Reactive] public BreakpointAddressType AddressType { get; set; }
		[Reactive] public string Condition { get; set; }

		public Breakpoint()
		{
			this.Condition = "";
		}

		public static bool IsTypeCpuBreakpoint(SnesMemoryType type)
		{
			return type != SnesMemoryType.Register && !type.IsPpuMemory();
		}
	}

	public class OldBreakpoint
	{
		private SnesMemoryType _memoryType = SnesMemoryType.CpuMemory;
		private bool _isCpuBreakpoint = true;

		public bool BreakOnRead = false;
		public bool BreakOnWrite = false;
		public bool BreakOnExec = true;

		public bool Enabled { get; set; } = true;
		public bool MarkEvent { get; set; } = false;
		public UInt32 Address = UInt32.MaxValue;
		public UInt32 StartAddress;
		public UInt32 EndAddress;
		public CpuType CpuType;
		public BreakpointAddressType AddressType = BreakpointAddressType.SingleAddress;
		public string Condition = "";
		public bool IsAssert = false;

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
			string format = (_memoryType == SnesMemoryType.SpcMemory || _memoryType == SnesMemoryType.GameboyMemory) ? "X4" : "X6";
			switch(AddressType) {
				case BreakpointAddressType.AnyAddress:
					return "<any>";
				case BreakpointAddressType.SingleAddress:
					addr += $"${Address.ToString(format)}";
					break;

				case BreakpointAddressType.AddressRange:
					addr = $"${StartAddress.ToString(format)} - ${EndAddress.ToString(format)}";
					break;
			}

			if(showLabel) {
				string label = GetAddressLabel();
				if(!string.IsNullOrWhiteSpace(label)) {
					addr += " [" + label + "]";
				}
			}
			return addr;
		}

		public static bool IsTypeCpuBreakpoint(SnesMemoryType type)
		{
			return type != SnesMemoryType.Register && !type.IsPpuMemory();
		}

		/*public void SetEnabled(bool enabled)
		{
			Enabled = enabled;
			BreakpointManager.RefreshBreakpoints(this);
		}

		public void SetMarked(bool marked)
		{
			MarkEvent = marked;
			BreakpointManager.RefreshBreakpoints(this);
		}*/

		public bool IsAbsoluteAddress { get { return !MemoryType.IsRelativeMemory(); } }
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

		public string ReadableAddress => GetAddressString(true);
		public string ReadableType => ToReadableType();

		public string ToReadableType()
		{
			string type;

			switch(MemoryType) {
				default:	throw new Exception("invalid type");
				case SnesMemoryType.CpuMemory: type = "CPU"; break;
				case SnesMemoryType.SpcMemory: type = "SPC"; break;
				case SnesMemoryType.Sa1Memory: type = "SA1"; break;
				case SnesMemoryType.GsuMemory: type = "GSU"; break;
				case SnesMemoryType.NecDspMemory: type = "DSP"; break;
				
				case SnesMemoryType.PrgRom: type = "PRG"; break;
				case SnesMemoryType.WorkRam: type = "WRAM"; break;
				case SnesMemoryType.SaveRam: type = "SRAM"; break;
				case SnesMemoryType.VideoRam: type = "VRAM"; break;
				case SnesMemoryType.SpriteRam: type = "OAM"; break;
				case SnesMemoryType.CGRam: type = "CG"; break;

				case SnesMemoryType.SpcRam: type = "RAM"; break;
				case SnesMemoryType.SpcRom: type = "ROM"; break;

				case SnesMemoryType.DspProgramRom: type = "DSP"; break;
				case SnesMemoryType.Sa1InternalRam: type = "IRAM"; break;
				case SnesMemoryType.GsuWorkRam: type = "GWRAM"; break;
				
				case SnesMemoryType.BsxPsRam: type = "PSRAM"; break;
				case SnesMemoryType.BsxMemoryPack: type = "MPACK"; break;
				
				case SnesMemoryType.GameboyMemory: type = "CPU"; break;
				case SnesMemoryType.GbPrgRom: type = "PRG"; break;
				case SnesMemoryType.GbWorkRam: type = "WRAM"; break;
				case SnesMemoryType.GbCartRam: type = "SRAM"; break;
				case SnesMemoryType.GbHighRam: type = "HRAM"; break;
				case SnesMemoryType.GbBootRom: type = "BOOT"; break;
				case SnesMemoryType.GbVideoRam: type = "VRAM"; break;
				case SnesMemoryType.GbSpriteRam: type = "OAM"; break;

				case SnesMemoryType.Register: type = "REG"; break;
			}

			type += ":";
			type += BreakOnRead ? "R" : "‒";
			type += BreakOnWrite ? "W" : "‒";
			if(IsCpuBreakpoint) {
				type += BreakOnExec ? "X" : "‒";
			}
			return type;
		}
		
		public string GetAddressLabel()
		{
			UInt32 address = AddressType == BreakpointAddressType.SingleAddress ? this.Address : this.StartAddress;

			if(IsCpuBreakpoint) {
				/*CodeLabel label;
				if(this.IsAbsoluteAddress) {
					label = LabelManager.GetLabel(address, this.MemoryType);
				} else {
					label = LabelManager.GetLabel(new AddressInfo() { Address = (int)address, Type = this.MemoryType });
				}
				if(label != null) {
					return label.Label;
				}*/
			}
			return string.Empty;
		}

		public int GetRelativeAddress()
		{
			UInt32 address = AddressType == BreakpointAddressType.SingleAddress ? this.Address : this.StartAddress;
			if(IsCpuBreakpoint && this.IsAbsoluteAddress) {
				return DebugApi.GetRelativeAddress(new AddressInfo() { Address = (int)address, Type = this.MemoryType }, this.CpuType).Address;
			} else {
				return (int)address;
			}
		}

		private int GetRelativeAddressEnd()
		{
			if(this.AddressType == BreakpointAddressType.AddressRange){
				if(IsCpuBreakpoint && this.IsAbsoluteAddress) {
					return DebugApi.GetRelativeAddress(new AddressInfo() { Address = (int)this.EndAddress, Type = this.MemoryType }, this.CpuType).Address;
				} else {
					return (int)this.EndAddress;
				}
			}
			return -1;
		}
		
		public bool Matches(UInt32 address, SnesMemoryType type, CpuType? cpuType)
		{
			if((cpuType.HasValue && cpuType.Value != this.CpuType) || IsTypeCpuBreakpoint(type) != this.IsCpuBreakpoint) {
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
				CpuType = CpuType,
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