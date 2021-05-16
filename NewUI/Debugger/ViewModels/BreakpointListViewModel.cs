using Avalonia.Controls;
using Mesen.GUI;
using Mesen.GUI.Debugger;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Reactive.Linq;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointListViewModel : ViewModelBase
	{
		public ObservableCollection<BreakpointViewModel> Breakpoints { get; }

		private IEnumerable<BreakpointViewModel> GenerateBreakpoints()
		{
			var breakpoints = new List<BreakpointViewModel>() {
					 new BreakpointViewModel(new Breakpoint()
					 {
						  StartAddress = 0x8000,
						  Enabled = true,
						  MarkEvent = false,
						  AddressType = BreakpointAddressType.SingleAddress,
						  BreakOnExec = true,
						  CpuType = CpuType.Cpu,
						  MemoryType = SnesMemoryType.CpuMemory
					 }),
					 new BreakpointViewModel(new Breakpoint()
					 {
						  StartAddress = 0xA8000,
						  EndAddress = 0xB8000,
						  Enabled = true,
						  MarkEvent = true,
						  AddressType = BreakpointAddressType.AddressRange,
						  BreakOnExec = true,
						  CpuType = CpuType.Cpu,
						  MemoryType = SnesMemoryType.CpuMemory
					 })
				};

			return breakpoints;
		}

		public BreakpointListViewModel()
		{
			this.Breakpoints = new ObservableCollection<BreakpointViewModel>(GenerateBreakpoints());
		}
	}

	public class BreakpointViewModel : ViewModelBase
	{
		public Breakpoint Breakpoint { get; set; }
		[ObservableAsProperty] public string? TypeDisplay { get; }
		[ObservableAsProperty] public string? AddressDisplay { get; }

		public BreakpointViewModel(Breakpoint bp)
		{
			this.Breakpoint = bp;

			this.WhenAnyValue(
				_ => _.Breakpoint.MemoryType,
				_ => _.Breakpoint.BreakOnRead,
				_ => _.Breakpoint.BreakOnWrite,
				_ => _.Breakpoint.BreakOnExec
			).Select(BreakpointViewModel.ToReadableType).ToPropertyEx(this, x => x.TypeDisplay);

			this.WhenAnyValue(
				_ => _.Breakpoint.MemoryType,
				_ => _.Breakpoint.AddressType,
				_ => _.Breakpoint.StartAddress, 
				_ => _.Breakpoint.EndAddress
			).Select((t) => BreakpointViewModel.GetAddressString(t)).ToPropertyEx(this, x => x.AddressDisplay);
		}

		private static string GetAddressString((SnesMemoryType memoryType, BreakpointAddressType addressType, UInt32 startAddress, UInt32 endAddress) bp) 
		{
			string addr = "";
			string format = (bp.memoryType == SnesMemoryType.SpcMemory || bp.memoryType == SnesMemoryType.GameboyMemory) ? "X4" : "X6";
			switch(bp.addressType) {
				case BreakpointAddressType.AnyAddress:
					return "<any>";
				case BreakpointAddressType.SingleAddress:
					addr += $"${bp.startAddress.ToString(format)}";
					break;

				case BreakpointAddressType.AddressRange:
					addr = $"${bp.startAddress.ToString(format)} - ${bp.endAddress.ToString(format)}";
					break;
			}

			/*if(showLabel) {
				string label = ""; //GetAddressLabel();
				if(!string.IsNullOrWhiteSpace(label)) {
					addr += " [" + label + "]";
				}
			}*/
			return addr;
		}

		private static string ToReadableType((SnesMemoryType memoryType, bool breakOnRead, bool breakOnWrite, bool breakOnExec) bp)
		{
			string type;

			switch(bp.memoryType) {
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

				case SnesMemoryType.Register: type = "REG"; break;
			}

			type += ":";
			type += bp.breakOnRead ? "R" : "‒";
			type += bp.breakOnWrite ? "W" : "‒";
			if(Breakpoint.IsTypeCpuBreakpoint(bp.memoryType)) {
				type += bp.breakOnExec ? "X" : "‒";
			}
			return type;
		}
	}
}
