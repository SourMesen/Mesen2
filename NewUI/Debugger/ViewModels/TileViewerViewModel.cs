using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform;
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
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class TileViewerViewModel : DisposableViewModel, ICpuTypeModel
	{
		[Reactive] public CpuType CpuType { get; set; }

		public TileViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();
		[Reactive] public PaletteSelectionMode PaletteSelectionMode { get; private set; }
		[Reactive] public int PaletteColumnCount { get; private set; } = 16;

		[Reactive] public int AddressIncrement { get; private set; }
		[Reactive] public int MaximumAddress { get; private set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();
		[Reactive] public Enum[] AvailableFormats { get; set; } = Array.Empty<Enum>();
		[Reactive] public bool ShowFormatDropdown { get; set; }

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private byte[] _sourceData = Array.Empty<byte>();

		[Obsolete("For designer only")]
		public TileViewerViewModel() : this(CpuType.Snes, new PictureViewer(), null) { }

		public TileViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TileViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;

			InitBitmap();

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
					ActionType = ActionType.ZoomIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomIn),
					OnClick = () => picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => picViewer.ZoomOut()
				},
			});

			DebugShortcutManager.CreateContextMenu(picViewer, new List<object> {
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					IsEnabled = () => false,
					OnClick = () => { }
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ExportToPng,
					OnClick = () => picViewer.ExportToPng()
				}
			});

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);

			InitForCpuType();

			AddDisposable(this.WhenAnyValue(x => x.CpuType).Subscribe(_ => {
				InitForCpuType();
				RefreshData();
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Format).Subscribe(x => {
				PaletteSelectionMode = x switch {
					TileFormat.Bpp2 or TileFormat.NesBpp2 => PaletteSelectionMode.FourColors,
					TileFormat.Bpp4 => PaletteSelectionMode.SixteenColors,
					_ => PaletteSelectionMode.None
				};
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.ColumnCount, x => x.Config.RowCount, x => x.Config.Format).Subscribe(x => {
				int bpp = Config.Format switch {
					TileFormat.Bpp2 => 2,
					TileFormat.Bpp4 => 4,
					TileFormat.DirectColor => 8,
					TileFormat.Mode7 => 16,
					TileFormat.Mode7DirectColor => 16,
					TileFormat.NesBpp2 => 2,
					_ => 8,
				};

				AddressIncrement = Config.ColumnCount * Config.RowCount * 8 * 8 * bpp / 8;
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Source).Subscribe(memType => {
				MaximumAddress = DebugApi.GetMemorySize(memType) - 1;
				RefreshData();
			}));

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, Config_PropertyChanged));
		}

		private void InitForCpuType()
		{
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.Source)) {
				Config.Source = CpuType.GetVramMemoryType();
			}

			AvailableFormats = CpuType switch {
				CpuType.Snes => new Enum[] { TileFormat.Bpp2, TileFormat.Bpp4, TileFormat.Bpp8, TileFormat.DirectColor, TileFormat.Mode7, TileFormat.Mode7DirectColor },
				CpuType.Nes => new Enum[] { TileFormat.NesBpp2 },
				CpuType.Gameboy => new Enum[] { TileFormat.Bpp2 },
				_ => throw new Exception("Unsupported CPU type")
			};

			if(AvailableFormats.Contains(Config.Format)) {
				Config.Format = (TileFormat)AvailableFormats[0];
			}
			ShowFormatDropdown = AvailableFormats.Length > 1;
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			RefreshTab();
		}

		public void RefreshData()
		{
			PaletteColors = DebugApi.GetPaletteInfo(CpuType).GetRgbPalette();
			PaletteColumnCount = PaletteColors.Length > 16 ? 16 : 4;
			_sourceData = DebugApi.GetMemoryState(Config.Source);

			RefreshTab();
		}

		private void RefreshTab()
		{
			Dispatcher.UIThread.Post(() => {
				InitBitmap();

				using(var framebuffer = ViewerBitmap.Lock()) {
					DebugApi.GetTileView(CpuType, GetOptions(), _sourceData, _sourceData.Length, PaletteColors, framebuffer.FrameBuffer.Address);
				}
			});
		}

		private GetTileViewOptions GetOptions()
		{
			return new GetTileViewOptions() {
				Format = Config.Format,
				Width = Config.ColumnCount,
				Height = Config.RowCount,
				Palette = Config.SelectedPalette,
				Layout = Config.Layout,
				StartAddress = Config.StartAddress,
				Background = Config.Background
			};
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap()
		{
			int width = Config.ColumnCount * 8;
			int height = Config.RowCount * 8;
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new DynamicBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}
	}
}
