using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Avalonia;
using Avalonia.Media;
using Mesen.Debugger;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TraceLoggerConfig : BaseWindowConfig<TraceLoggerConfig>
	{
		//TODO
		[Reactive] public bool AutoRefresh { get; set; } = true;
		[Reactive] public int TextZoom { get; set; } = 100;

		[Reactive] public TraceLoggerCpuConfig SnesConfig { get; set; }
		[Reactive] public TraceLoggerCpuConfig SpcConfig { get; set; }
		[Reactive] public TraceLoggerCpuConfig NecDspConfig { get; set; }
		[Reactive] public TraceLoggerCpuConfig Sa1Config { get; set; }
		[Reactive] public TraceLoggerCpuConfig GsuConfig { get; set; }
		[Reactive] public TraceLoggerCpuConfig Cx4Config { get; set; }
		[Reactive] public TraceLoggerCpuConfig GbConfig { get; set; }
		[Reactive] public TraceLoggerCpuConfig NesConfig { get; set; }

		public TraceLoggerConfig()
		{
			SnesConfig = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][EffectiveAddress] [MemoryValue,h][Align,38] A:[A,4h] X:[X,4h] Y:[Y,4h] S:[SP,4h] D:[D,4h] DB:[DB,2h] P:[P,8] H:[HClock,3] V:[Scanline,3]"
			};

			SpcConfig = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][EffectiveAddress] [MemoryValue,h][Align,38] A:[A,2h] X:[X,2h] Y:[Y,2h] S:[SP,2h] P:[P,8] H:[HClock,3] V:[Scanline,3]"
			};

			Sa1Config = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][EffectiveAddress] [MemoryValue,h][Align,38] A:[A,4h] X:[X,4h] Y:[Y,4h] S:[SP,4h] D:[D,4h] DB:[DB,2h] P:[P,8] H:[HClock,3] V:[Scanline,3]"
			};

			GsuConfig = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][Align,38] SRC:[SRC,2h] DST:[DST,2h] R0:[R0,4h] R1:[R1,4h] R2:[R2,4h] R3:[R3,4h] R4:[R4,4h] R5:[R5,4h] R6:[R6,4h] R7:[R7,4h] R8:[R8,4h] R9:[R9,4h] R10:[R10,4h] R11:[R11,4h] R12:[R12,4h] R13:[R13,4h] R14:[R14,4h] R15:[R15,4h] H:[Cycle,3] V:[Scanline,3]"
			};

			Cx4Config = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][Align,38] A:[A,6h] MAR:[MAR,6h] MDR:[MDR,6h] DPR:[DPR,6h] ML:[ML,6h] MH:[MH,6h] P:[P,4h] PB:[PB,4h] PS:[PS] R0:[R0,6h] R1:[R1,6h] R2:[R2,6h] R3:[R3,6h] R4:[R4,6h] R5:[R5,6h] R6:[R6,6h] R7:[R7,6h] R8:[R8,6h] R9:[R9,6h] R10:[R10,6h] R11:[R11,6h] R12:[R12,6h] R13:[R13,6h] R14:[R14,6h] R15:[R15,6h] H:[Cycle,3] V:[Scanline,3]"
			};

			NecDspConfig = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][Align,38] A:[A,4h] [FlagsA] B:[B,4h] [FlagsB] K:[K,4h] L:[L,4h] M:[M,4h] N:[N,4h] RP:[RP,4h] DP:[DP,4h] DR:[DR,4h] SR:[SR,4h] TR:[TR,4h] TRB:[TRB,4h] H:[Cycle,3] V:[Scanline,3]"
			};

			NesConfig = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][EffectiveAddress] [MemoryValue,h][Align,38] A:[A,2h] X:[X,2h] Y:[Y,2h] S:[SP,2h] P:[P,8] V:[Scanline,3] H:[Cycle,3]"
			};

			GbConfig = new TraceLoggerCpuConfig() {
				Enabled = true,
				Format = "[Disassembly][EffectiveAddress] [MemoryValue,h][Align,38] A:[A,2h] B:[B,2h] C:[C,2h] D:[D,2h] E:[E,2h] F:[PS,4] HL:[H,2h][L,2h] V:[Scanline,3] H:[Cycle,3]"
			};
		}

		public TraceLoggerCpuConfig GetCpuConfig(CpuType type)
		{
			return type switch {
				CpuType.Snes => SnesConfig,
				CpuType.Spc => SpcConfig,
				CpuType.NecDsp => NecDspConfig,
				CpuType.Sa1 => Sa1Config,
				CpuType.Gsu => GsuConfig,
				CpuType.Cx4 => Cx4Config,
				CpuType.Gameboy => GbConfig,
				CpuType.Nes => NesConfig,
				_ => throw new NotImplementedException("Unsupport cpu type")
			};
		}
	}
}
