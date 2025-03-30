using ReactiveUI.Fody.Helpers;
using Mesen.ViewModels;

namespace Mesen.Config;

public class GbaDebuggerConfig : ViewModelBase
{
	[Reactive] public bool BreakOnInvalidOpCode { get; set; } = false;
	[Reactive] public bool BreakOnNopLoad { get; set; } = false;
	[Reactive] public bool BreakOnUnalignedMemAccess { get; set; } = false;
	
	[Reactive] public GbaDisassemblyMode DisassemblyMode { get; set; } = GbaDisassemblyMode.Default;
}

public enum GbaDisassemblyMode : byte
{
	Default,
	Arm,
	Thumb
}