using Avalonia.Controls;
using System;
using Mesen.Interop;
using Mesen.Config;
using System.Collections.Generic;
using System.Threading;
using Avalonia.Threading;
using System.Linq;
using Mesen.Debugger.ViewModels;

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

		public class LastRefreshInfo
		{
			public WeakReference<object> Host;
			public DateTime Stamp;

			public LastRefreshInfo(object host)
			{
				Host = new(host);
			}
		}

		private static List<LastRefreshInfo> _lastRefreshStamp = new();
		private static Dictionary<Window, ToolInfo> _activeWindows = new();
		private static int _nextId = 0;

		private static int RegisterWindow(Window wnd, RefreshTimingConfig cfg, CpuType cpuType)
		{
			if(_activeWindows.ContainsKey(wnd)) {
				throw new Exception("Register window called twice");
			}

			int newId = Interlocked.Increment(ref _nextId);
			wnd.Closed += OnWindowClosedHandler;

			_activeWindows.Add(wnd, new ToolInfo() { ViewerId = newId, Scanline = cfg.RefreshScanline, Cycle = cfg.RefreshCycle });

			DebugApi.SetViewerUpdateTiming(newId, cfg.RefreshScanline, cfg.RefreshCycle, cpuType);

			return newId;
		}

		private static void OnWindowClosedHandler(object? sender, EventArgs e)
		{
			if(sender is Window wnd) {
				_activeWindows.Remove(wnd);
				wnd.Closed -= OnWindowClosedHandler;
			}
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

		public static void ProcessNotification(Window wnd, NotificationEventArgs e, RefreshTimingViewModel cfg, ICpuTypeModel model, Action refresh)
		{
			int viewerId = GetViewerId(wnd, cfg.Config, model.CpuType);

			switch(e.NotificationType) {
				case ConsoleNotificationType.ViewerRefresh:
					if(cfg.Config.AutoRefresh && e.Parameter.ToInt32() == viewerId && !LimitFps(wnd, 80)) {
						refresh();
					}
					break;

				case ConsoleNotificationType.CodeBreak:
					if(cfg.Config.RefreshOnBreakPause) {
						refresh();
					}
					break;

				case ConsoleNotificationType.GameLoaded:
					RomInfo romInfo = EmuApi.GetRomInfo();
					if(!romInfo.CpuTypes.Contains(model.CpuType)) {
						model.CpuType = romInfo.ConsoleType.GetMainCpuType();
					}

					cfg.UpdateMinMaxValues();

					if(_activeWindows.TryGetValue(wnd, out ToolInfo? toolInfo)) {
						DebugApi.SetViewerUpdateTiming(toolInfo.ViewerId, cfg.Config.RefreshScanline, cfg.Config.RefreshCycle, model.CpuType);
					}
					break;
			}
		}

		public static bool LimitFps(object host, int maxFps)
		{
			DateTime now = DateTime.Now;
			for(int i = _lastRefreshStamp.Count - 1; i >= 0; i--) {
				if(_lastRefreshStamp[i].Host.TryGetTarget(out object? target)) {
					if(object.ReferenceEquals(target, host)) {
						if((now - _lastRefreshStamp[i].Stamp).TotalMilliseconds < 1000.0 / maxFps) {
							return true;
						} else {
							_lastRefreshStamp[i].Stamp = now;
							return false;
						}
					}
				} else {
					_lastRefreshStamp.RemoveAt(i);
				}
			}

			_lastRefreshStamp.Add(new LastRefreshInfo(host) { Stamp = now });
			return false;
		}
	}
}
