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
	public class SmsDebuggerConfig : ViewModelBase
	{
		[Reactive] public bool BreakOnNopLoad { get; set; } = false;
	}
}
