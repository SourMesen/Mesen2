using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TraceLoggerCpuConfig : BaseConfig<TraceLoggerCpuConfig>
	{
		[Reactive] public bool Enabled { get; set; }

		[Reactive] public bool ShowByteCode { get; set; }
		[Reactive] public bool ShowRegisters { get; set; }
		[Reactive] public bool ShowCpuCycles { get; set; }

		[Reactive] public bool ShowPpuCycles { get; set; }
		[Reactive] public bool ShowPpuScanline { get; set; }
		[Reactive] public bool ShowPpuFrames { get; set; }

		[Reactive] public bool IndentCode { get; set; }
		[Reactive] public bool ShowEffectiveAddresses { get; set; }
		[Reactive] public bool ShowMemoryValues { get; set; }
		[Reactive] public bool UseLabels { get; set; }

		[Reactive] public StatusFlagFormat StatusFormat { get; set; }

		[Reactive] public bool OverrideFormat { get; set; }
		[Reactive] public string Format { get; set; } = "";
		[Reactive] public string Condition { get; set; } = "";
	}

	public enum StatusFlagFormat
	{
		Hexadecimal = 0,
		Text = 1,
		CompactText = 2
	}
}
