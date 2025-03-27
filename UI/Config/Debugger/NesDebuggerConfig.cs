using Avalonia;
using Avalonia.Media;
using Mesen.Debugger;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System.Reactive.Linq;
using System.Reactive;
using Mesen.ViewModels;

namespace Mesen.Config
{
	public class NesDebuggerConfig : ViewModelBase
	{
		[Reactive] public bool BreakOnBrk { get; set; } = false;
		[Reactive] public bool BreakOnUnofficialOpCode { get; set; } = false;
		[Reactive] public bool BreakOnUnstableOpCode { get; set; } = false;
		[Reactive] public bool BreakOnCpuCrash { get; set; } = false;
		
		[Reactive] public bool BreakOnBusConflict { get; set; } = false;
		[Reactive] public bool BreakOnDecayedOamRead { get; set; } = false;
		[Reactive] public bool BreakOnPpuScrollGlitch { get; set; } = false;
		[Reactive] public bool BreakOnExtOutputMode { get; set; } = true;
		[Reactive] public bool BreakOnInvalidVramAccess { get; set; } = false;
		[Reactive] public bool BreakOnInvalidOamWrite { get; set; } = false;
		[Reactive] public bool BreakOnDmaInputRead { get; set; } = false;
	}
}
