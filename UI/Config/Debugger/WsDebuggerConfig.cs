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
	public class WsDebuggerConfig : ViewModelBase
	{
		[Reactive] public bool BreakOnUndefinedOpCode { get; set; } = false;
	}
}
