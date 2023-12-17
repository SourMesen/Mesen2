using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Mesen.Utilities;

public class DisplayMessageHelper
{
	private static int _taskId = 0;

	public static void DisplayMessage(string title, string message, string? param1 = null)
	{
		if(EmuApi.IsRunning() || ConfigManager.Config.Preferences.GameSelectionScreenMode == GameSelectionMode.Disabled) {
			EmuApi.DisplayMessage(title, message, param1);
		} else {
			//Temporarily hide selection screen to allow displaying messages
			MainWindowViewModel.Instance.RecentGames.Visible = false;

			EmuApi.DisplayMessage(title, message, param1);

			//Prevent multiple calls from causing the game selection screen from appearing too quickly
			int counter = Interlocked.Increment(ref _taskId);

			Task.Run(async () => {
				await Task.Delay(3100);

				//Show game selection screen after ~3 seconds
				//This allows the message to be visible to the user
				Dispatcher.UIThread.Post(() => {
					if(_taskId == counter && !EmuApi.IsRunning()) {
						MainWindowViewModel.Instance.RecentGames.Visible = true;
					}
				});
			});
		}
	}
}
