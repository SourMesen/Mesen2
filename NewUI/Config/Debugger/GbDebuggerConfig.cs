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
	public class GbDebuggerConfig : ViewModelBase
	{
		[Reactive] public bool GbBreakOnInvalidOamAccess { get; set; } = false;
		[Reactive] public bool GbBreakOnInvalidVramAccess { get; set; } = false;
		[Reactive] public bool GbBreakOnDisableLcdOutsideVblank { get; set; } = false;
		[Reactive] public bool GbBreakOnInvalidOpCode { get; set; } = false;
		[Reactive] public bool GbBreakOnNopLoad { get; set; } = false;
		[Reactive] public bool GbBreakOnOamCorruption { get; set; } = false;

		public void ApplyConfig()
		{
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnInvalidOamAccess, GbBreakOnInvalidOamAccess);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnInvalidVramAccess, GbBreakOnInvalidVramAccess);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnDisableLcdOutsideVblank, GbBreakOnDisableLcdOutsideVblank);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnInvalidOpCode, GbBreakOnInvalidOpCode);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnNopLoad, GbBreakOnNopLoad);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnOamCorruption, GbBreakOnOamCorruption);
		}
	}
}
