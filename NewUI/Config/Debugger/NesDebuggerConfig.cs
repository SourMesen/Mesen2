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
		[Reactive] public bool BreakOnPpu2006ScrollGlitch { get; set; } = false;

		public void ApplyConfig()
		{
			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesBreakOnBrk, BreakOnBrk);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesBreakOnUnofficialOpCode, BreakOnUnofficialOpCode);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesBreakOnCpuCrash, BreakOnCpuCrash);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesBreakOnBusConflict, BreakOnBusConflict);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesBreakOnDecayedOamRead, BreakOnDecayedOamRead);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesBreakOnPpu2006ScrollGlitch, BreakOnPpu2006ScrollGlitch);
		}
	}
}
