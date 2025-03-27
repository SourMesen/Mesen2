using ReactiveUI.Fody.Helpers;
using Mesen.ViewModels;

namespace Mesen.Config
{
	public class SnesDebuggerConfig : ViewModelBase
	{
		[Reactive] public bool BreakOnBrk { get; set; } = false;
		[Reactive] public bool BreakOnCop { get; set; } = false;
		[Reactive] public bool BreakOnWdm { get; set; } = false;
		[Reactive] public bool BreakOnStp { get; set; } = false;
		[Reactive] public bool BreakOnInvalidPpuAccess { get; set; } = false;
		[Reactive] public bool BreakOnReadDuringAutoJoy { get; set; } = false;

		[Reactive] public bool SpcBreakOnBrk { get; set; } = false;
		[Reactive] public bool SpcBreakOnStpSleep { get; set; } = false;
		
		[Reactive] public bool UseAltSpcOpNames { get; set; } = false;
		[Reactive] public bool IgnoreDspReadWrites { get; set; } = true;
	}
}
