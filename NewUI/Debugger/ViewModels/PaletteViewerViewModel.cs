using Avalonia;
using Avalonia.Controls;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mesen.Debugger.ViewModels
{
	public class PaletteViewerViewModel : DisposableViewModel, ICpuTypeModel
	{
		[Reactive] public CpuType CpuType { get; set; }

		public PaletteViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();
		[Reactive] public UInt32[]? PaletteValues { get; set; } = null;
		[Reactive] public int PaletteColumnCount { get; private set; } = 16;

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }
		[Reactive] public int SelectedPalette { get; set; } = 0;
		[Reactive] public int BlockSize { get; set; } = 8;

		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public int ViewerMouseOverPalette { get; set; } = -1;

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private RefStruct<DebugPaletteInfo>? _palette = null;

		[Obsolete("For designer only")]
		public PaletteViewerViewModel() : this(CpuType.Snes, null) { }

		public PaletteViewerViewModel(CpuType cpuType, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.PaletteViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;
			
			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			FileMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			});

			ViewMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.RefreshTiming.AutoRefresh,
					OnClick = () => Config.RefreshTiming.AutoRefresh = !Config.RefreshTiming.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshTiming.RefreshOnBreakPause,
					OnClick = () => Config.RefreshTiming.RefreshOnBreakPause = !Config.RefreshTiming.RefreshOnBreakPause
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.ZoomIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomIn),
					OnClick = ZoomIn
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = ZoomOut
				},
			});

			AddDisposable(this.WhenAnyValue(x => x.CpuType).Subscribe(_ => RefreshData()));
			AddDisposable(this.WhenAnyValue(x => x.Config.Zoom).Subscribe(x => BlockSize = Math.Max(16, 16 + (x - 1) * 4)));
			AddDisposable(this.WhenAnyValue(x => x.SelectedPalette).Subscribe(x => UpdatePreviewPanel()));

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		public void ZoomOut()
		{
			Config.Zoom = Math.Min(20, Math.Max(1, Config.Zoom - 1));
		}

		public void ZoomIn()
		{
			Config.Zoom = Math.Min(20, Math.Max(1, Config.Zoom + 1));
		}

		public void RefreshData()
		{
			DebugPaletteInfo paletteInfo = DebugApi.GetPaletteInfo(CpuType);
			PaletteColors = paletteInfo.GetRgbPalette();
			if(paletteInfo.RawFormat == RawPaletteFormat.Indexed) {
				PaletteValues = paletteInfo.GetRawPalette();
			} else {
				PaletteValues = null;
			}
			PaletteColumnCount = (int)paletteInfo.ColorsPerPalette;

			_palette = new(paletteInfo);

			Dispatcher.UIThread.Post(() => {
				UpdatePreviewPanel();
			});
		}

		private void UpdatePreviewPanel()
		{
			PreviewPanel = GetPreviewPanel(SelectedPalette, PreviewPanel);

			if(ViewerTooltip != null && ViewerMouseOverPalette >= 0) {
				GetPreviewPanel(ViewerMouseOverPalette, ViewerTooltip);
			}
		}

		public DynamicTooltip? GetPreviewPanel(int index, DynamicTooltip? tooltipToUpdate)
		{
			if(_palette == null) {
				return null;
			}

			DebugPaletteInfo palette = _palette.Get();
			if(index >= palette.ColorCount) {
				return null;
			}

			UInt32[] rgbPalette = palette.GetRgbPalette();
			UInt32[] rawPalette = palette.GetRawPalette();

			TooltipEntries entries = tooltipToUpdate?.Items ?? new();
			entries.StartUpdate();

			entries.AddEntry("Color", new TooltipColorEntry(rgbPalette[index]));
			entries.AddEntry("Index", "$" + index.ToString("X2"));
			if(palette.RawFormat == RawPaletteFormat.Rgb555) {
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X4"));
				entries.AddEntry("R", "$" + (rawPalette[index] & 0x1F).ToString("X2"));
				entries.AddEntry("G", "$" + ((rawPalette[index] >> 5) & 0x1F).ToString("X2"));
				entries.AddEntry("B", "$" + (rawPalette[index] >> 10).ToString("X2"));
			} else {
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X2"));
			}
			entries.AddEntry("Color Code (Hex)", "#" + rgbPalette[index].ToString("X8").Substring(2));
			entries.AddEntry("Color Code (RGB)",
				((rgbPalette[index] >> 16) & 0xFF).ToString() + ", " +
				((rgbPalette[index] >> 8) & 0xFF).ToString() + ", " +
				(rgbPalette[index] & 0xFF).ToString()
			);

			entries.EndUpdate();
			if(tooltipToUpdate != null) {
				return tooltipToUpdate;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}
	}
}
