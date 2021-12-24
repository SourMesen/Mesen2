using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels
{
	public class ScriptWindowViewModel : ViewModelBase
	{
		public FontConfig Font { get; } = ConfigManager.Config.Debug.Font;

		public ScriptWindowViewModel(CpuType cpuType, string code)
		{
		}
	}
}
