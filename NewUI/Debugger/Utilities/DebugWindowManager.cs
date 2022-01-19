using Avalonia.Controls;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using System;
using System.Collections.Concurrent;
using System.Threading;

namespace Mesen.Debugger.Utilities
{
	public static class DebugWindowManager
	{
		private static int _debugWindowCounter = 0;
		private static ConcurrentDictionary<Window, bool> _openedWindows = new();

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
			_openedWindows.TryAdd(wnd, true);
			wnd.Show();
		}

		private static void CloseDebugWindow(Window wnd)
		{
			if(Interlocked.Decrement(ref _debugWindowCounter) == 0) {
				//Closed the last debug window, save the workspace and turn off the debugger
				DebugWorkspaceManager.Save(true);
				DebugApi.ReleaseDebugger();
			}

			_openedWindows.TryRemove(wnd, out _);
		}

		public static void CloseAllWindows()
		{
			//Iterate on a copy of the list since it will change during iteration
			foreach(Window window in _openedWindows.Keys) {
				window.Close();
			}
		}

		public static void ProcessNotification(NotificationEventArgs e)
		{
			if(_openedWindows.Count == 0) {
				return;
			}

			foreach(Window window in _openedWindows.Keys) {
				if(window is INotificationHandler handler) {
					handler.ProcessNotification(e);
				}
			}

			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					DebugWorkspaceManager.Load();
					break;

				case ConsoleNotificationType.EmulationStopped:
					DebugWindowManager.CloseAllWindows();
					break;
			}
		}
	}
}
