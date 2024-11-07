using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mesen.Debugger.ViewModels
{
	public class PaletteViewerViewModel : DisposableViewModel, ICpuTypeModel
	{
		public CpuType CpuType { get; set; }
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

		[Reactive] public List<object> FileMenuActions { get; private set; } = new();
		[Reactive] public List<object> ViewMenuActions { get; private set; } = new();

		private RefStruct<DebugPaletteInfo>? _palette = null;

		[Obsolete("For designer only")]
		public PaletteViewerViewModel() : this(CpuType.Snes) { }

		public PaletteViewerViewModel(CpuType cpuType)
		{
			Config = ConfigManager.Config.Debug.PaletteViewer.Clone();
			CpuType = cpuType;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming, cpuType);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(this.WhenAnyValue(x => x.Config.Zoom).Subscribe(x => BlockSize = Math.Max(16, 16 + (x - 1) * 4)));
			AddDisposable(this.WhenAnyValue(x => x.SelectedPalette).Subscribe(x => UpdatePreviewPanel()));
		}

		public void InitActions(Window wnd, PaletteSelector palSelector)
		{
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
				new ContextMenuSeparator(),
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
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ShowSettingsPanel,
					Shortcut =  () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ToggleSettingsPanel),
					IsSelected = () => Config.ShowSettingsPanel,
					OnClick = () => Config.ShowSettingsPanel = !Config.ShowSettingsPanel
				},
				new ContextMenuSeparator(),
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

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);

			AddDisposables(DebugShortcutManager.CreateContextMenu(palSelector, new List<object> {
				new ContextMenuAction() {
					ActionType = ActionType.EditColor,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.PaletteViewer_EditColor),
					IsEnabled = () => SelectedPalette >= 0,
					OnClick = () => {
						Dispatcher.UIThread.Post(() => {
							EditColor(wnd, SelectedPalette);
						});
					}
				},
				new ContextMenuSeparator() { IsVisible = () => _palette != null && _palette.Get().HasMemType },
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.PaletteViewer_ViewInMemoryViewer),
					IsVisible = () => _palette != null && _palette.Get().HasMemType,
					IsEnabled = () => SelectedPalette >= 0,
					OnClick = () => {
						if(_palette != null) {
							DebugPaletteInfo pal = _palette.Get();
							int memSize = DebugApi.GetMemorySize(pal.PaletteMemType) - (int)pal.PaletteMemOffset;
							if(memSize > 0) {
								int bytesPerColor = memSize / (int)pal.ColorCount;
								MemoryToolsWindow.ShowInMemoryTools(pal.PaletteMemType, (int)pal.PaletteMemOffset + SelectedPalette * bytesPerColor);
							}
						}
					}
				},
			}));
		}

		private async void EditColor(Window wnd, int selectedPalette)
		{
			if(_palette == null) {
				return;
			}

			DebugPaletteInfo palette = _palette.Get();
			if(selectedPalette < 0 || selectedPalette >= palette.ColorCount) {
				return;
			}

			if(palette.RawFormat == RawPaletteFormat.Rgb555 || palette.RawFormat == RawPaletteFormat.Rgb444 || palette.RawFormat == RawPaletteFormat.Bgr444) {
				ColorPickerViewModel model = new ColorPickerViewModel() { Color = Color.FromUInt32(palette.GetRgbPalette()[selectedPalette]) };
				ColorPickerWindow colorPicker = new ColorPickerWindow() { DataContext = model };
				bool success = await colorPicker.ShowCenteredDialog<bool>(wnd);
				if(success) {
					DebugApi.SetPaletteColor(CpuType, selectedPalette, model.Color.ToUInt32());
					RefreshData();
				}
			} else {
				//Show palette and let user pick a color
				ColorIndexPickerWindow colorPicker = new ColorIndexPickerWindow(CpuType, selectedPalette);
				int? colorIndex = await colorPicker.ShowCenteredDialog<int?>(wnd);
				if(colorIndex.HasValue) {
					DebugApi.SetPaletteColor(CpuType, selectedPalette, (uint)colorIndex.Value);
					RefreshData();
				}
			}
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
			uint[] paletteColors = paletteInfo.GetRgbPalette();
			uint[]? paletteValues;
			if(paletteInfo.RawFormat == RawPaletteFormat.Indexed) {
				paletteValues = paletteInfo.GetRawPalette();
			} else {
				paletteValues = null;
			}
			int paletteColumnCount = (int)paletteInfo.ColorsPerPalette;

			Dispatcher.UIThread.Post(() => {
				PaletteColors = paletteColors;
				PaletteValues = paletteValues;
				PaletteColumnCount = paletteColumnCount;
				_palette = new(paletteInfo);

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

			return PaletteHelper.GetPreviewPanel(rgbPalette, rawPalette, palette.RawFormat, index, tooltipToUpdate);
		}

		public void OnGameLoaded()
		{
			RefreshData();
		}
	}
}
