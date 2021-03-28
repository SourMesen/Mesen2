using Mesen.Debugger.Controls;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels
{
	public class DisassemblyViewerViewModel : ViewModelBase
	{
		[Reactive] public ICodeDataProvider DataProvider { get; set; }
		[Reactive] public ILineStyleProvider StyleProvider { get; set; }
		[Reactive] public int ScrollPosition { get; set; }
		[Reactive] public int MaxScrollPosition { get; set; } = 1000;
	}
}
