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
	public class PceDebuggerConfig : ViewModelBase
	{
		[Reactive] public bool BreakOnBrk { get; set; } = false;
		[Reactive] public bool BreakOnUnofficialOpCode { get; set; } = false;

		public void ApplyConfig()
		{
			ConfigApi.SetDebuggerFlag(DebuggerFlags.PceBreakOnBrk, BreakOnBrk);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.PceBreakOnUnofficialOpCode, BreakOnUnofficialOpCode);
		}
	}
}
