using Avalonia.Controls;
using Avalonia.Threading;
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
		private static object _windowNotifLock = new();

		public static T OpenDebugWindow<T>(Func<T> createWindow) where T : Window
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
			return wnd;
		}

		public static T GetOrOpenDebugWindow<T>(Func<T> createWindow) where T : Window
		{
			foreach(Window wnd in _openedWindows.Keys) {
				if(wnd is T) {
					return (T)wnd;
				}
			}
			return OpenDebugWindow<T>(createWindow);
		}

		private static void CloseDebugWindow(Window wnd)
		{
			//Remove window from list first, to ensure no more notifications are sent to it
			_openedWindows.TryRemove(wnd, out _);

			if(Interlocked.Decrement(ref _debugWindowCounter) == 0) {
				//Closed the last debug window, save the workspace and turn off the debugger
				//Run any jobs pending on the UI thread, to ensure the debugger
				//doesn't get restarted by a pending job from the window that was closed
				//Lock to prevent this from running while ProcessNotification is sending
				//notifications out to the debug windows
				lock(_windowNotifLock) {
					Dispatcher.UIThread.RunJobs();
				}
				DebugWorkspaceManager.Save(true);
				DebugApi.ReleaseDebugger();
			}
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

			lock(_windowNotifLock) {
				foreach(Window window in _openedWindows.Keys) {
					if(window is INotificationHandler handler) {
						handler.ProcessNotification(e);
					}
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
