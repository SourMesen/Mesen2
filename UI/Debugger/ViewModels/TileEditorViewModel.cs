using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels;

public class TileEditorViewModel : DisposableViewModel
{
	[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

	[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();
	[Reactive] public UInt32[] RawPalette { get; set; } = Array.Empty<UInt32>();
	[Reactive] public RawPaletteFormat RawFormat { get; set; }
	[Reactive] public int PaletteColumnCount { get; private set; } = 16;
	[Reactive] public int SelectedColor { get; set; } = 0;

	public TileEditorConfig Config { get; }

	public List<object> FileMenuActions { get; private set; } = new();
	public List<object> ViewMenuActions { get; private set; } = new();

	private CpuType _cpuType;
	private List<AddressInfo> _tileAddresses;
	private int _columnCount = 1;
	private int _rowCount = 1;
	private TileFormat _tileFormat;
	private byte[] _sourceData = Array.Empty<byte>();
	private UInt32[] _tileBuffer = Array.Empty<UInt32>();

	[Obsolete("For designer only")]
	public TileEditorViewModel() : this(new() { new() }, 1, TileFormat.Bpp4, 0) { }

	public TileEditorViewModel(List<AddressInfo> tileAddresses, int columnCount, TileFormat format, int initialPalette)
	{
		Config = ConfigManager.Config.Debug.TileEditor;
		_tileAddresses = tileAddresses;
		_columnCount = columnCount;
		_rowCount = tileAddresses.Count / columnCount;
		_cpuType = tileAddresses[0].Type.ToCpuType();
		_tileFormat = format;
		SelectedColor = initialPalette * GetColorsPerPalette(_tileFormat);

		PixelSize size = format.GetTileSize();
		_tileBuffer = new UInt32[size.Width * size.Height];
		ViewerBitmap = new DynamicBitmap(new PixelSize(size.Width * _columnCount, size.Height * _rowCount), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
		
		if(Design.IsDesignMode) {
			return;
		}

		AddDisposable(this.WhenAnyValue(x => x.Config.Background).Subscribe(x => RefreshViewer()));
		AddDisposable(this.WhenAnyValue(x => x.SelectedColor).Subscribe(x => RefreshViewer()));
		AddDisposable(this.WhenAnyValue(x => x.Config.ImageScale).Subscribe(x => {
			if(Config.ImageScale < 4) {
				Config.ImageScale = 4;
			}
		}));

		RefreshViewer();
	}

	public void InitActions(PictureViewer picViewer, Window wnd)
	{
		FileMenuActions = AddDisposables(new List<object>() {
			new ContextMenuAction() {
				ActionType = ActionType.ExportToPng,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveAsPng),
				OnClick = () => picViewer.ExportToPng()
			},
			new ContextMenuSeparator(),
			new ContextMenuAction() {
				ActionType = ActionType.Exit,
				OnClick = () => wnd.Close()
			}
		});

		ViewMenuActions = AddDisposables(new List<object>() {
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

		DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
		DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
	}

	private record TilePixelPositionInfo(int Column, int Row, int TileX, int TileY);
	private TilePixelPositionInfo GetPositionInfo(PixelPoint position)
	{
		PixelSize tileSize = _tileFormat.GetTileSize();

		int column = position.X / tileSize.Width;
		int row = position.Y / tileSize.Height;
		int tileX = position.X % tileSize.Width;
		int tileY = position.Y % tileSize.Height;
		return new(column, row, tileX, tileY);
	}

	public int GetColorAtPosition(PixelPoint position)
	{
		TilePixelPositionInfo pos = GetPositionInfo(position);
		int paletteColorIndex = DebugApi.GetTilePixel(_tileAddresses[pos.Column + pos.Row * _columnCount], _tileFormat, pos.TileX, pos.TileY);
		return SelectedColor - (SelectedColor % GetColorsPerPalette(_tileFormat)) + paletteColorIndex;
	}

	public void SelectColor(PixelPoint position)
	{
		SelectedColor = GetColorAtPosition(position);
	}

	public void UpdatePixel(PixelPoint position, bool clearPixel)
	{
		int pixelColor = clearPixel ? 0 : SelectedColor % GetColorsPerPalette(_tileFormat);
		TilePixelPositionInfo pos = GetPositionInfo(position);
		DebugApi.SetTilePixel(_tileAddresses[pos.Column+pos.Row*_columnCount], _tileFormat, pos.TileX, pos.TileY, pixelColor);
		RefreshViewer();
	}

	private unsafe void RefreshViewer()
	{
		Dispatcher.UIThread.Post((Action)(() => {
			DebugPaletteInfo palette = DebugApi.GetPaletteInfo(_cpuType, new GetPaletteInfoOptions() { Format = _tileFormat });
			PaletteColors = palette.GetRgbPalette();
			RawPalette = palette.GetRawPalette();
			RawFormat = palette.RawFormat;
			PaletteColumnCount = PaletteColors.Length > 16 ? 16 : 4;

			_sourceData = DebugApi.GetMemoryState(_tileAddresses[0].Type);
			PixelSize tileSize = _tileFormat.GetTileSize();

			using(var framebuffer = ViewerBitmap.Lock()) {
				for(int y = 0; y < _rowCount; y++) {
					for(int x = 0; x < _columnCount; x++) {
						fixed(UInt32* ptr = _tileBuffer) {
							DebugApi.GetTileView(_cpuType, GetOptions(x, y), _sourceData, _sourceData.Length, PaletteColors, (IntPtr)ptr);
							UInt32* viewer = (UInt32*)framebuffer.FrameBuffer.Address;
							int rowPitch = ViewerBitmap.PixelSize.Width;
							int baseOffset = x * tileSize.Width + y * tileSize.Height * rowPitch;
							for(int j = 0; j < tileSize.Height; j++) {
								for(int i = 0; i < tileSize.Width; i++) {
									viewer[baseOffset + j * rowPitch + i] = ptr[j * tileSize.Width + i];
								}
							}
						}
					}
				}
			}
		}));
	}

	public int GetColorsPerPalette()
	{
		return GetColorsPerPalette(_tileFormat);
	}

	private int GetColorsPerPalette(TileFormat format)
	{
		return format.GetBitsPerPixel() switch {
			2 => 4, //4-color palettes
			4 => 16, //16-color palettes
			_ => 256
		};
	}

	private GetTileViewOptions GetOptions(int column, int row)
	{
		int tileIndex = row * _columnCount + column;

		return new GetTileViewOptions() {
			MemType = _tileAddresses[tileIndex].Type,
			Format = _tileFormat,
			Width = _tileFormat.GetTileSize().Width / 8,
			Height = _tileFormat.GetTileSize().Height / 8,
			Palette = SelectedColor / GetColorsPerPalette(_tileFormat),
			Layout = TileLayout.Normal,
			Filter = TileFilter.None,
			StartAddress = _tileAddresses[tileIndex].Address,
			Background = Config.Background,
			UseGrayscalePalette = false
		};
	}
}
