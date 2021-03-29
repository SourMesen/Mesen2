using Dock.Model.ReactiveUI.Controls;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class CpuStatusToolViewModel : Tool
	{
		public SnesCpuViewModel SnesCpuViewModel { get; set; }

		public CpuStatusToolViewModel(SnesCpuViewModel snesCpuViewModel)
		{
			Id = "CpuStatusTool";
			Title = "CPU Status";
			SnesCpuViewModel = snesCpuViewModel;
		}
	}

}