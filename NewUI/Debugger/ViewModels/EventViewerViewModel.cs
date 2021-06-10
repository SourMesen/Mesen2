using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using System;

namespace Mesen.Debugger.ViewModels
{
	public class EventViewerViewModel : ViewModelBase
	{
		public CpuType CpuType { get; }
		public EventViewerConfig Config { get; }
		public object ConsoleConfig { get; set; }

		//For designer
		public EventViewerViewModel() : this(CpuType.Cpu) { }

		public EventViewerViewModel(CpuType cpuType)
		{
			CpuType = cpuType;

			Config = ConfigManager.Config.Debug.EventViewer.Clone();

			ConsoleConfig = cpuType switch {
				CpuType.Cpu => Config.SnesConfig,
				CpuType.Nes => Config.NesConfig,
				CpuType.Gameboy => Config.GbConfig,
				_ => throw new Exception("Invalid cpu type")
			};
		}

		public void SaveConfig()
		{
			ConfigManager.Config.Debug.EventViewer = Config;
			ConfigManager.Config.Save();
		}
	}
}
