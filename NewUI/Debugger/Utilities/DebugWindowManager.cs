using Avalonia.Controls;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Threading;

namespace Mesen.Debugger.Utilities
{
	public static class DebugWindowManager
	{
		private static int _debugWindowCounter = 0;
		private static List<Window> _openedWindows = new();

		public static void OpenDebugWindow<T>(Func<T> createWindow) where T : Window
		{
			if(Interlocked.Increment(ref _debugWindowCounter) == 1) {
				//Opened a debug window and nothing else was opened, load the saved workspace
				DebugWorkspaceManager.Load();
			}

			T wnd = createWindow();
			wnd.Closed += (s, e) => {
				if(s is Window window) {
					CloseDebugWindow(window);
				}
			};
			_openedWindows.Add(wnd);
			wnd.Show();
		}

		private static void CloseDebugWindow(Window wnd)
		{
			if(Interlocked.Decrement(ref _debugWindowCounter) == 0) {
				//Closed the last debug window, save the workspace and turn off the debugger
				DebugWorkspaceManager.Save(true);
				DebugApi.ReleaseDebugger();
			}

			_openedWindows.Remove(wnd);
		}

		public static void CloseAllWindows()
		{
			//Iterate on a copy of the list since it will change during iteration
			foreach(Window window in _openedWindows.ToArray()) {
				window.Close();
			}
		}
	}
}
