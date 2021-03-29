using Dock.Model.ReactiveUI.Controls;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class PpuStatusToolViewModel : Tool
	{
		public SnesPpuViewModel SnesPpuViewModel { get; set; }

		public PpuStatusToolViewModel(SnesPpuViewModel snesPpuViewModel)
		{
			Id = "PpuStatusTool";
			Title = "PPU Status";
			SnesPpuViewModel = snesPpuViewModel;
		}
	}

}