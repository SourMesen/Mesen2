using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerConfigWindowViewModel : ViewModelBase
	{
		public DebuggerConfig Debugger { get; set; }
		public ScriptWindowConfig Script { get; set; }
		public DbgIntegrationConfig Integration { get; set; }
		[Reactive] public int SelectedIndex { get; set; }

		public DebuggerConfigWindowViewModel()
		{
			Debugger = ConfigManager.Config.Debug.Debugger;
			Script = ConfigManager.Config.Debug.ScriptWindow;
			Integration = ConfigManager.Config.Debug.DbgIntegration;
		}

		internal void SaveConfig()
		{
			throw new NotImplementedException();
		}
	}
}
