using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels
{
	public class DisassemblyViewerViewModel : ViewModelBase
	{
		[Reactive] public ICodeDataProvider? DataProvider { get; set; } = null;
		[Reactive] public BaseStyleProvider? StyleProvider { get; set; } = null;
		[Reactive] public int ScrollPosition { get; set; }
		[Reactive] public int MaxScrollPosition { get; set; } = 0;
		
		public DebuggerConfig Config { get; private set; }

		//For designer
		public DisassemblyViewerViewModel(): this(new DebuggerConfig()) { }

		public DisassemblyViewerViewModel(DebuggerConfig config)
		{
			Config = config;
		}

		public void UpdateMaxScroll()
		{
			MaxScrollPosition = DataProvider?.GetLineCount() - 1 ?? 0;
		}
	}
}
