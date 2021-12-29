using Avalonia.Controls;
using System;
using Mesen.Interop;
using Mesen.Config;
using System.Collections.Generic;
using System.Threading;

namespace Mesen.Debugger.Utilities
{
	public static class ToolRefreshHelper
	{
		class ToolInfo
		{
			internal int ViewerId { get; set; } = 0;
			internal int Scanline { get; set; } = 0;
			internal int Cycle { get; set; } = 0;
		}

		private static Dictionary<object, DateTime> _lastRefreshStamp = new();
		private static Dictionary<Window, ToolInfo> _activeWindows = new();
		private static int _nextId = 0;

		private static int RegisterWindow(Window wnd, RefreshTimingConfig cfg, CpuType cpuType)
		{
			if(_activeWindows.ContainsKey(wnd)) {
				throw new Exception("Register window called twice");
			}

			int newId = Interlocked.Increment(ref _nextId);
			wnd.Closed += (s, e) => {
				_activeWindows.Remove(wnd);
				//TODO
				//DebugApi.RemoveViewer(newId);
			};
			_activeWindows.Add(wnd, new ToolInfo() { ViewerId = newId, Scanline = cfg.RefreshScanline, Cycle = cfg.RefreshCycle });

			DebugApi.SetViewerUpdateTiming(newId, cfg.RefreshScanline, cfg.RefreshCycle, cpuType);

			return newId;
		}

		private static int GetViewerId(Window wnd, RefreshTimingConfig cfg, CpuType cpuType)
		{
			if(_activeWindows.TryGetValue(wnd, out ToolInfo? toolInfo)) {
				if(cfg.RefreshScanline != toolInfo.Scanline || cfg.RefreshCycle != toolInfo.Cycle) {
					toolInfo.Scanline = cfg.RefreshScanline;
					toolInfo.Cycle = cfg.RefreshCycle;
					DebugApi.SetViewerUpdateTiming(toolInfo.ViewerId, cfg.RefreshScanline, cfg.RefreshCycle, cpuType);
				}
				return toolInfo.ViewerId;
			} else {
				return RegisterWindow(wnd, cfg, cpuType);
			}
		}

		public static void ProcessNotification(Window wnd, NotificationEventArgs e, RefreshTimingConfig cfg, CpuType cpuType, Action refresh)
		{
			int viewerId = GetViewerId(wnd, cfg, cpuType);

			switch(e.NotificationType) {
				case ConsoleNotificationType.ViewerRefresh:
					if(cfg.AutoRefresh && e.Parameter.ToInt32() == viewerId) {
						refresh();
					}
					break;

				case ConsoleNotificationType.CodeBreak:
					if(cfg.RefreshOnBreakPause) {
						refresh();
					}
					break;

				case ConsoleNotificationType.GameLoaded:
					if(_activeWindows.TryGetValue(wnd, out ToolInfo? toolInfo)) {
						DebugApi.SetViewerUpdateTiming(toolInfo.ViewerId, cfg.RefreshScanline, cfg.RefreshCycle, cpuType);
					}
					break;
			}
		}

		public static bool LimitFps(object host, int maxFps)
		{
			DateTime now = DateTime.Now;
			if(_lastRefreshStamp.TryGetValue(host, out DateTime stamp)) {
				if((now - stamp).TotalMilliseconds < 1000.0 / maxFps) {
					return true;
				}
			}

			_lastRefreshStamp[host] = now;
			return false;
		}
	}
}
