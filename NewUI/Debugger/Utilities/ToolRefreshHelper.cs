using Avalonia.Controls;
using System;
using Mesen.Interop;
using Mesen.Config;
using System.Collections.Generic;
using System.Threading;
using Avalonia.Threading;
using System.Linq;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;

namespace Mesen.Debugger.Utilities
{
	public static class ToolRefreshHelper
	{
		class ToolInfo
		{
			internal int ViewerId { get; set; } = 0;
			internal int Scanline { get; set; } = 0;
			internal int Cycle { get; set; } = 0;
			internal CpuType CpuType { get; set; }
		}

		public class LastRefreshInfo
		{
			public WeakReference<Window> Host;
			public DateTime Stamp;

			public LastRefreshInfo(Window host)
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

			_activeWindows.Add(wnd, new ToolInfo() { ViewerId = newId, Scanline = cfg.RefreshScanline, Cycle = cfg.RefreshCycle, CpuType = cpuType });

			DebugApi.SetViewerUpdateTiming(newId, cfg.RefreshScanline, cfg.RefreshCycle, cpuType);

			return newId;
		}

		private static void OnWindowClosedHandler(object? sender, EventArgs e)
		{
			if(sender is Window wnd) {
				if(_activeWindows.TryGetValue(wnd, out ToolInfo? toolInfo)) {
					DebugApi.RemoveViewerId(toolInfo.ViewerId, toolInfo.CpuType);
				}
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
					toolInfo.CpuType = cpuType;
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
					model.OnGameLoaded();

					cfg.UpdateMinMaxValues(model.CpuType);

					if(_activeWindows.TryGetValue(wnd, out ToolInfo? toolInfo)) {
						toolInfo.CpuType = model.CpuType;
						DebugApi.SetViewerUpdateTiming(toolInfo.ViewerId, cfg.Config.RefreshScanline, cfg.Config.RefreshCycle, model.CpuType);
					}
					break;
			}
		}

		public static bool LimitFps(Window wnd, int maxFps)
		{
			if(_lastRefreshStamp.Count > 5) {
				//Reduce FPS by 10% for each active tool above 5 (down to 30% refresh speed)
				maxFps = Math.Max(5, (int)(maxFps * Math.Max(0.3, 1.0 - (_lastRefreshStamp.Count - 5) * 0.1)));
			}

			DateTime now = DateTime.Now;
			for(int i = _lastRefreshStamp.Count - 1; i >= 0; i--) {
				if(_lastRefreshStamp[i].Host.TryGetTarget(out Window? target)) {
					if(!DebugWindowManager.IsDebugWindow(target)) {
						//Window was closed, remove from list
						_lastRefreshStamp.RemoveAt(i);
						continue;
					}

					if(target == wnd) {
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

			_lastRefreshStamp.Add(new LastRefreshInfo(wnd) { Stamp = now });
			return false;
		}
	}
}
