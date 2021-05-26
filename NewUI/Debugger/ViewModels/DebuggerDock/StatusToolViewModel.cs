using Dock.Model.ReactiveUI.Controls;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class StatusToolViewModel : Tool
	{
		[Reactive] public object? StatusViewModel { get; set; } = null;

		public StatusToolViewModel()
		{
			Id = "CpuStatusTool";
			Title = "CPU Status";
		}
	}
}