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

		[ObservableAsProperty] public string? TypeDisplay { get; }
		[ObservableAsProperty] public string? AddressDisplay { get; }

		public Breakpoint()
		{
			this.WhenAnyValue(_ => _.MemoryType, _ => _.BreakOnRead, _ => _.BreakOnWrite, _ => _.BreakOnExec).Select(x => ToReadableType()).ToPropertyEx(this, x => x.TypeDisplay);
			this.WhenAnyValue(_ => _.MemoryType, _ => _.StartAddress, _ => _.EndAddress).Select(x => GetAddressString(true)).ToPropertyEx(this, x => x.AddressDisplay);
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
			string format = (MemoryType == SnesMemoryType.SpcMemory || MemoryType == SnesMemoryType.GameboyMemory) ? "X4" : "X6";
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
			UInt32 address = StartAddress;

			if(IsCpuBreakpoint) {
				CodeLabel? label;
				if(IsAbsoluteAddress) {
					label = LabelManager.GetLabel(address, this.MemoryType);
				} else {
					label = LabelManager.GetLabel(new AddressInfo() { Address = (int)address, Type = this.MemoryType });
				}
				return label?.Label ?? string.Empty;
			}
			return string.Empty;
		}

		public string ToReadableType()
		{
			string type;

			switch(MemoryType) {
				default: throw new Exception("invalid type");
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

				case SnesMemoryType.NesMemory: type = "CPU"; break;
				case SnesMemoryType.NesPrgRom: type = "PRG"; break;
				case SnesMemoryType.NesWorkRam: type = "WRAM"; break;
				case SnesMemoryType.NesSaveRam: type = "SRAM"; break;
				case SnesMemoryType.NesSpriteRam: type = "SPR"; break;
				case SnesMemoryType.NesPaletteRam: type = "PAL"; break;
				case SnesMemoryType.NesNametableRam: type = "NTRAM"; break;
				case SnesMemoryType.NesInternalRam: type = "RAM"; break;
				case SnesMemoryType.NesChrRom: type = "CHR"; break;
				case SnesMemoryType.NesChrRam: type = "CHR"; break;

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