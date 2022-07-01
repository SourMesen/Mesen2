using Avalonia;
using Avalonia.Media;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class ScriptWindowConfig : BaseWindowConfig<ScriptWindowConfig>
	{
		private const int MaxRecentScripts = 10;

		[Reactive] public List<string> RecentScripts { get; set; } = new List<string>();

		[Reactive] public int Zoom { get; set; } = 100;

		[Reactive] public double LogWindowHeight { get; set; } = 100;

		[Reactive] public ScriptStartupBehavior ScriptStartupBehavior { get; set; } = ScriptStartupBehavior.ShowTutorial;
		[Reactive] public bool SaveScriptBeforeRun { get; set; } = true;
		[Reactive] public bool AutoStartScriptOnLoad { get; set; } = true;
		[Reactive] public bool AutoReloadScriptWhenFileChanges { get; set; } = true;
		[Reactive] public bool AutoRestartScriptAfterPowerCycle { get; set; } = true;
		
		[Reactive] public bool AllowIoOsAccess { get; set; } = false;
		[Reactive] public bool AllowNetworkAccess { get; set; } = false;

		[Reactive] public UInt32 ScriptTimeout { get; set; } = 1;

		public void AddRecentScript(string scriptFile)
		{
			string? existingItem = RecentScripts.Where((file) => file == scriptFile).FirstOrDefault();
			if(existingItem != null) {
				RecentScripts.Remove(existingItem);
			}

			RecentScripts.Insert(0, scriptFile);
			if(RecentScripts.Count > ScriptWindowConfig.MaxRecentScripts) {
				RecentScripts.RemoveAt(ScriptWindowConfig.MaxRecentScripts);
			}
		}
	}

	public enum ScriptStartupBehavior
	{
		ShowTutorial = 0,
		ShowBlankWindow = 1,
		LoadLastScript = 2
	}
}
