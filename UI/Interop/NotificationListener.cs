using Avalonia.Controls;
using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop
{
	public class NotificationListener : IDisposable
	{
		public delegate void NotificationCallback(int type, IntPtr parameter);
		public delegate void NotificationEventHandler(NotificationEventArgs e);
		public event NotificationEventHandler? OnNotification;

		//Need to keep a reference to this callback, or it will get garbage collected (since the only reference to it is on the native side)
		private NotificationCallback _callback;
		private IntPtr _notificationListener;

		private bool _forHistoryViewer;

		public NotificationListener(bool forHistoryViewer = false)
		{
			_forHistoryViewer = forHistoryViewer;
			_callback = (int type, IntPtr parameter) => {
				this.ProcessNotification(type, parameter);
			};

			if(Design.IsDesignMode) {
				return;
			}

			_notificationListener = _forHistoryViewer ? HistoryApi.HistoryViewerRegisterNotificationCallback(_callback) : EmuApi.RegisterNotificationCallback(_callback);
		}

		public void Dispose()
		{
			if(Design.IsDesignMode) {
				return;
			}

			if(_notificationListener != IntPtr.Zero) {
				if(_forHistoryViewer) {
					HistoryApi.HistoryViewerUnregisterNotificationCallback(_notificationListener);
				} else {
					EmuApi.UnregisterNotificationCallback(_notificationListener);
				}
				_notificationListener = IntPtr.Zero;
			}
		}

		public void ProcessNotification(int type, IntPtr parameter)
		{
			OnNotification?.Invoke(new NotificationEventArgs() {
				NotificationType = (ConsoleNotificationType)type,
				Parameter = parameter
			});
		}
	}

	public class NotificationEventArgs
	{
		public ConsoleNotificationType NotificationType;
		public IntPtr Parameter;
	}

	public enum ConsoleNotificationType
	{
		GameLoaded,
		StateLoaded,
		GameReset,
		GamePaused,
		GameResumed,
		CodeBreak,
		DebuggerResumed,
		PpuFrameDone,
		ResolutionChanged,
		ConfigChanged,
		ExecuteShortcut,
		ReleaseShortcut,
		EmulationStopped,
		BeforeEmulationStop,
		ViewerRefresh,
		EventViewerRefresh,
		MissingFirmware,
		SufamiTurboFilePrompt,
		BeforeGameUnload,
		BeforeGameLoad,
		GameLoadFailed,
		CheatsChanged,
		RequestConfigChange,
		RefreshSoftwareRenderer
	}

	public struct GameLoadedEventParams
	{
		[MarshalAs(UnmanagedType.I1)] public bool IsPaused;
		[MarshalAs(UnmanagedType.I1)] public bool IsPowerCycle;
	}
}
