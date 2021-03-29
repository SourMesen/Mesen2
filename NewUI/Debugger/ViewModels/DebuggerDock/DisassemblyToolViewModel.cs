using Dock.Model.ReactiveUI.Controls;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class DisassemblyToolViewModel : Tool
	{
		public DisassemblyViewerViewModel DisassemblyViewerViewModel { get; set; }

		public DisassemblyToolViewModel(DisassemblyViewerViewModel disassemblyViewerViewModel)
		{
			Id = "DiassemblyTool";
			Title = "Disassembly";
			CanClose = false;
			CanPin = false;
			CanFloat = false;
			DisassemblyViewerViewModel = disassemblyViewerViewModel;
		}
	}

}