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
		[Reactive] public bool AutoRefresh { get; set; } = true;
		[Reactive] public bool RefreshOnBreakPause { get; set; } = true;
		[Reactive] public bool ShowToolbar { get; set; } = true;

		[Reactive] public TraceLoggerCpuConfig SnesConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig SpcConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig NecDspConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig Sa1Config { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig GsuConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig Cx4Config { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig St018Config { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig GbConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig NesConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig PceConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig SmsConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig GbaConfig { get; set; } = new();
		[Reactive] public TraceLoggerCpuConfig WsConfig { get; set; } = new();

		public TraceLoggerConfig()
		{
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
				CpuType.St018 => St018Config,
				CpuType.Gameboy => GbConfig,
				CpuType.Nes => NesConfig,
				CpuType.Pce => PceConfig,
				CpuType.Sms => SmsConfig,
				CpuType.Gba => GbaConfig,
				CpuType.Ws => WsConfig,
				_ => throw new NotImplementedException("Unsupport cpu type")
			};
		}
	}
}
