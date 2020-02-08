using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public class WindowRefreshManager : IDisposable
	{
		private IRefresh _window;
		private NotificationListener _notifListener;
		private Stopwatch _timer;
		private long _lastUpdate = 0;
		private long _minDelay = 0;

		public bool AutoRefresh { get; set; }
		public RefreshSpeed AutoRefreshSpeed { get; set; }

		public WindowRefreshManager(IRefresh window)
		{
			_window = window;
			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;
			_timer = Stopwatch.StartNew();
		}

		public void Dispose()
		{
			_notifListener.Dispose();
		}

		private void RefreshContent()
		{
			_lastUpdate = _timer.ElapsedTicks;
			_window.RefreshData();
			((Form)_window).BeginInvoke((Action)(() => {
				if(((Form)_window).IsDisposed) {
					return;
				}

				_window.RefreshViewer();

				//Limit FPS to 3x time it takes for a single update (rough estimate), and cap based on requested fps.
				int divider;
				switch(this.AutoRefreshSpeed) {
					default:
					case RefreshSpeed.Low: divider = 20; break;
					case RefreshSpeed.Normal: divider = 40; break;
					case RefreshSpeed.High: divider = 80; break;
				}

				_minDelay = Math.Max(Stopwatch.Frequency / divider, (long)((_timer.ElapsedTicks - _lastUpdate) * 2));
			}));
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					RefreshContent();
					break;

				case ConsoleNotificationType.EventViewerRefresh:
					if(_window.ScanlineCycleSelect == null && this.AutoRefresh && (_timer.ElapsedTicks - _lastUpdate) > _minDelay) {
						RefreshContent();
					}
					break;

				case ConsoleNotificationType.ViewerRefresh:
					if(_window.ScanlineCycleSelect != null && this.AutoRefresh && e.Parameter.ToInt32() == _window.ScanlineCycleSelect.ViewerId && (_timer.ElapsedTicks - _lastUpdate) > _minDelay) {
						RefreshContent();
					}
					break;

				case ConsoleNotificationType.GameLoaded:
					//Configuration is lost when debugger is restarted (when switching game or power cycling)
					_window.ScanlineCycleSelect?.RefreshSettings();
					break;
			}
		}
	}

	public interface IRefresh
	{
		void RefreshData();
		void RefreshViewer();

		ctrlScanlineCycleSelect ScanlineCycleSelect { get; }
	}
}
