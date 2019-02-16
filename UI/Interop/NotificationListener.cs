using System;

namespace Mesen.GUI
{
	public class NotificationListener : IDisposable
	{
		public delegate void NotificationCallback(int type, IntPtr parameter);
		public delegate void NotificationEventHandler(NotificationEventArgs e);
		public event NotificationEventHandler OnNotification;

		//Need to keep a reference to this callback, or it will get garbage collected (since the only reference to it is on the native side)
		NotificationCallback _callback;
		IntPtr _notificationListener;

		public NotificationListener()
		{
			_callback = (int type, IntPtr parameter) => {
				this.ProcessNotification(type, parameter);
			};
			_notificationListener = EmuApi.RegisterNotificationCallback(_callback);
		}

		public void Dispose()
		{
			EmuApi.UnregisterNotificationCallback(_notificationListener);
		}

		public void ProcessNotification(int type, IntPtr parameter)
		{
			if(this.OnNotification != null) {
				this.OnNotification(new NotificationEventArgs() {
					NotificationType = (ConsoleNotificationType)type,
					Parameter = parameter
				});
			}
		}
	}

	public class NotificationEventArgs
	{
		public ConsoleNotificationType NotificationType;
		public IntPtr Parameter;
	}

	public enum ConsoleNotificationType
	{
		GameLoaded = 0,
		StateLoaded = 1,
		GameReset = 2,
		GamePaused = 3,
		GameResumed = 4,
		GameStopped = 5,
		CodeBreak = 6,
		PpuFrameDone = 7,
		ResolutionChanged = 8,
		ConfigChanged = 9,
		ExecuteShortcut = 10,
		EmulationStopped = 11,
		BeforeEmulationStop = 12,
	}
}
