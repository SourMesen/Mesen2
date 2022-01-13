using Dock.Model.Core;
using Dock.Model.ReactiveUI.Controls;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class DisassemblyToolViewModel : Tool
	{
		public DisassemblyViewModel DisassemblyViewerViewModel { get; set; }

		public DisassemblyToolViewModel(DisassemblyViewModel disassemblyViewerViewModel)
		{
			Id = "DiassemblyTool";
			Title = "Disassembly";
			CanClose = false;
			CanPin = false;
			CanFloat = false;
			DisassemblyViewerViewModel = disassemblyViewerViewModel;
			DisassemblyViewerViewModel.ParentTool = this;
		}
	}

}