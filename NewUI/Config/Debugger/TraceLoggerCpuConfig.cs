using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TraceLoggerCpuConfig : BaseConfig<TraceLoggerCpuConfig>
	{
		[Reactive] public bool Enabled { get; set; } = true;

		[Reactive] public bool ShowRegisters { get; set; } = true;
		[Reactive] public bool ShowStatusFlags { get; set; } = true;
		[Reactive] public StatusFlagFormat StatusFormat { get; set; } = StatusFlagFormat.Text;

		[Reactive] public bool ShowEffectiveAddresses { get; set; } = true; 
		[Reactive] public bool ShowMemoryValues { get; set; } = true;
		[Reactive] public bool ShowByteCode { get; set; } = false;

		[Reactive] public bool ShowClockCounter { get; set; } = false;
		[Reactive] public bool ShowFrameCounter { get; set; } = false;
		[Reactive] public bool ShowFramePosition { get; set; } = true;

		[Reactive] public bool UseLabels { get; set; } = true;
		[Reactive] public bool IndentCode { get; set; } = false;

		[Reactive] public bool UseCustomFormat { get; set; } = false;
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
