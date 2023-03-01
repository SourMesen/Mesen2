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
		[Reactive] public bool BreakOnCpuCrash { get; set; } = false;
		
		[Reactive] public bool BreakOnBusConflict { get; set; } = false;
		[Reactive] public bool BreakOnDecayedOamRead { get; set; } = false;
		[Reactive] public bool BreakOnPpu2000ScrollGlitch { get; set; } = false;
		[Reactive] public bool BreakOnPpu2006ScrollGlitch { get; set; } = false;
		[Reactive] public bool NesBreakOnExtOutputMode { get; set; } = true;
	}
}
