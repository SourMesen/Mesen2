using Mesen.GUI.Controls;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class ScriptWindowConfig
	{
		private const int MaxRecentScripts = 10;

		public List<string> RecentScripts = new List<string>();

		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public string FontFamily = BaseControl.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Regular;
		public float FontSize = BaseControl.DefaultFontSize;
		public int Zoom = 100;

		public int CodeWindowHeight = 0;
		public ScriptStartupBehavior ScriptStartupBehavior = ScriptStartupBehavior.ShowTutorial;
		public bool SaveScriptBeforeRun = true;
		public bool AutoLoadLastScript = true;
		public bool AutoRestartScript = true;
		public UInt32 ScriptTimeout = 1000;

		public void AddRecentScript(string scriptFile)
		{
			string existingItem = RecentScripts.Where((file) => file == scriptFile).FirstOrDefault();
			if(existingItem != null) {
				RecentScripts.Remove(existingItem);
			}

			RecentScripts.Insert(0, scriptFile);
			if(RecentScripts.Count > ScriptWindowConfig.MaxRecentScripts) {
				RecentScripts.RemoveAt(ScriptWindowConfig.MaxRecentScripts);
			}
			ConfigManager.ApplyChanges();
		}
	}

	public enum ScriptStartupBehavior
	{
		ShowTutorial = 0,
		ShowBlankWindow = 1,
		LoadLastScript = 2
	}
}
