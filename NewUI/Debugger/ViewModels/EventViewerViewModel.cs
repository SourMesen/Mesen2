using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.ViewModels
{
	public class EventViewerViewModel : ViewModelBase
	{
		[Reactive] public WriteableBitmap ViewerBitmap { get; private set; }
		
		public CpuType CpuType { get; }
		public EventViewerConfig Config { get; }
		public object ConsoleConfig { get; set; }
		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private PictureViewer _picViewer;

		//For designer
		public EventViewerViewModel() : this(CpuType.Nes, new PictureViewer(), null) { }

		public EventViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			CpuType = cpuType;
			_picViewer = picViewer;
			Config = ConfigManager.Config.Debug.EventViewer;
			InitBitmap(new FrameInfo() { Width = 1, Height = 1 });

			ConsoleConfig = cpuType switch {
				CpuType.Cpu => Config.SnesConfig,
				CpuType.Nes => Config.NesConfig,
				CpuType.Gameboy => Config.GbConfig,
				_ => throw new Exception("Invalid cpu type")
			};

			FileMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			};

			ViewMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshViewer()
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.AutoRefresh,
					OnClick = () => Config.AutoRefresh = !Config.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshOnBreakPause,
					OnClick = () => Config.RefreshOnBreakPause = !Config.RefreshOnBreakPause
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.ZoomIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomIn),
					OnClick = () => _picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => _picViewer.ZoomOut()
				},
			};

			if(Design.IsDesignMode) {
				return;
			}

			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		public void SaveConfig()
		{
			ConfigManager.Config.Save();
		}

		private void InitBitmap()
		{
			InitBitmap(DebugApi.GetEventViewerDisplaySize(CpuType));
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap(FrameInfo size)
		{
			if(ViewerBitmap == null || ViewerBitmap.Size.Width != size.Width || ViewerBitmap.Size.Height != size.Height) {
				ViewerBitmap = new WriteableBitmap(new PixelSize((int)size.Width, (int)size.Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		public void RefreshViewer()
		{
			DebugApi.TakeEventSnapshot(CpuType);

			Dispatcher.UIThread.Post(() => {
				InitBitmap();
				using(var framebuffer = ViewerBitmap.Lock()) {
					DebugApi.GetEventViewerOutput(CpuType, framebuffer.Address, (uint)(ViewerBitmap.Size.Width * ViewerBitmap.Size.Height * sizeof(UInt32)));
				}

				_picViewer.InvalidateVisual();
			});
		}
	}
}
