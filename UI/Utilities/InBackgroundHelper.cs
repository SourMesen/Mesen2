using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Utilities
{
	class InBackgroundHelper
	{
		private static Timer _tmrUpdateBackground;
		private static bool _needResume = false;

		public static void StartBackgroundTimer()
		{
			_tmrUpdateBackground = new Timer();
			_tmrUpdateBackground.Start();
			_tmrUpdateBackground.Tick += tmrUpdateBackground_Tick;
		}

		public static void StopBackgroundTimer()
		{
			_tmrUpdateBackground?.Stop();
		}

		private static void tmrUpdateBackground_Tick(object sender, EventArgs e)
		{
			Form focusedForm = null;
			foreach(Form form in Application.OpenForms) {
				if(form.ContainsFocus) {
					focusedForm = form;
					break;
				}
			}

			PreferencesConfig cfg = ConfigManager.Config.Preferences;

			bool needPause = focusedForm == null && cfg.PauseWhenInBackground;
			if(focusedForm != null) {
				needPause |= cfg.PauseWhenInMenusAndConfig && focusedForm is BaseForm && (((BaseForm)focusedForm).InMenu || ((BaseForm)focusedForm).IsConfigForm);
				needPause |= cfg.PauseWhenInMenusAndConfig && !(focusedForm is BaseInputForm) && !focusedForm.GetType().FullName.Contains("Debugger");
				needPause |= cfg.PauseWhenInDebuggingTools && focusedForm.GetType().FullName.Contains("Debugger");
			}

			if(needPause) {
				if(!EmuApi.IsPaused()) {
					_needResume = true;
					EmuApi.Pause();
				}
			} else if(_needResume) {
				EmuApi.Resume();
				_needResume = false;
			}

			ConfigApi.SetEmulationFlag(EmulationFlags.InBackground, focusedForm == null);
		}
	}
}
