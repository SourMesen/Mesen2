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
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class TileViewerViewModel : ViewModelBase, IDisposable
	{
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		public TileViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();
		[Reactive] public PaletteSelectionMode PaletteSelectionMode { get; private set; }

		[Reactive] public int AddressIncrement { get; private set; }
		[Reactive] public int MaximumAddress { get; private set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();
		[Reactive] public Enum[] AvailableFormats { get; set; } = Array.Empty<Enum>();
		[Reactive] public bool ShowFormatDropdown { get; set; }

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		[Obsolete("For designer only")]
		public TileViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes, new PictureViewer(), null) { }

		public TileViewerViewModel(CpuType cpuType, ConsoleType consoleType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TileViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;
			ConsoleType = consoleType;

			InitBitmap();

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

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
					OnClick = () => picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => picViewer.ZoomOut()
				},
			};

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);

			AvailableMemoryTypes = Enum.GetValues<SnesMemoryType>().Where(t => DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();

			if(!AvailableMemoryTypes.Contains(Config.Source)) {
				Config.Source = cpuType.GetVramMemoryType();
			}

			AvailableFormats = cpuType switch {
				CpuType.Cpu => new Enum[] { TileFormat.Bpp2, TileFormat.Bpp4, TileFormat.Bpp8, TileFormat.DirectColor, TileFormat.Mode7, TileFormat.Mode7DirectColor },
				CpuType.Nes => new Enum[] { TileFormat.NesBpp2 },
				CpuType.Gameboy => new Enum[] { TileFormat.Bpp2 },
				_ => throw new Exception("Unsupported CPU type")
			};

			ShowFormatDropdown = AvailableFormats.Length > 1;

			this.WhenAnyValue(x => x.Config.Format).Subscribe(x => {
				PaletteSelectionMode = x switch {
					TileFormat.Bpp2 or TileFormat.NesBpp2 => PaletteSelectionMode.FourColors,
					TileFormat.Bpp4 => PaletteSelectionMode.SixteenColors,
					_ => PaletteSelectionMode.None
				};
			});

			this.WhenAnyValue(x => x.Config.ColumnCount, x => x.Config.RowCount, x => x.Config.Format).Subscribe(x => {
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
			});

			this.WhenAnyValue(x => x.Config.Source).Subscribe(memType => {
				MaximumAddress = DebugApi.GetMemorySize(memType) - 1;
			});
		}

		public void RefreshData()
		{
			UpdatePaletteColors();

			byte[] source = DebugApi.GetMemoryState(Config.Source);

			Dispatcher.UIThread.Post(() => {
				InitBitmap();

				using(var framebuffer = ViewerBitmap.Lock()) {
					DebugApi.GetTileView(CpuType.Cpu, GetOptions(), source, source.Length, PaletteColors, framebuffer.FrameBuffer.Address);
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

		public void UpdatePaletteColors()
		{
			PaletteColors = PaletteHelper.GetConvertedPalette(CpuType, ConsoleType);
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

		public void Dispose()
		{
		}
	}
}
