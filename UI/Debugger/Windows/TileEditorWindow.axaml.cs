using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Avalonia.Interactivity;
using Mesen.Debugger.Utilities;
using Mesen.Utilities;
using System.Collections.Generic;
using Avalonia.Threading;
using System.Linq;
using Mesen.Config;
using Avalonia.Input;

namespace Mesen.Debugger.Windows
{
	public class TileEditorWindow : MesenWindow, INotificationHandler
	{
		private TileEditorViewModel _model;
		private PictureViewer _picViewer;
		private DynamicTooltip? _tileColorTooltip;
		private PixelPoint? _lastPosition;

		[Obsolete("For designer only")]
		public TileEditorWindow() : this(new()) { }

		public TileEditorWindow(TileEditorViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_picViewer = this.GetControl<ScrollPictureViewer>("picViewer").InnerViewer;
			_picViewer.PositionClicked += PicViewer_PositionClicked;
			_picViewer.PointerMoved += PicViewer_PointerMoved;
			_picViewer.PointerExited += PicViewer_PointerExited;
			_model = model;
			_model.InitActions(_picViewer, this);
			DataContext = _model;

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
		}

		private void PicViewer_PositionClicked(object? sender, PositionClickedEventArgs e)
		{
			if(e.OriginalEvent.KeyModifiers == KeyModifiers.Shift) {
				_model.SelectColor(e.Position);
			} else {
				_model.UpdatePixel(e.Position, e.Properties.IsRightButtonPressed);
			}
		}

		private void PicViewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			PixelPoint? p = _picViewer.GetGridPointFromMousePoint(e.GetCurrentPoint(_picViewer).Position);
			if(_lastPosition != p) {
				_lastPosition = p;

				if(e.KeyModifiers != KeyModifiers.Shift) {
					TooltipHelper.HideTooltip(_picViewer);
					return;
				}

				if(_lastPosition != null) {
					ShowColorTooltip();
				} else {
					TooltipHelper.HideTooltip(_picViewer);
				}
			}
		}

		private void ShowColorTooltip()
		{
			if(_lastPosition != null) {
				int colorIndex = _model.GetColorAtPosition(_lastPosition.Value);
				_tileColorTooltip = PaletteHelper.GetPreviewPanel(_model.PaletteColors, _model.RawPalette, _model.RawFormat, colorIndex, _tileColorTooltip, _model.GetColorsPerPalette());
				TooltipHelper.ShowTooltip(_picViewer, _tileColorTooltip, 15);
			}
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			if(e.Key == Key.LeftShift || e.Key == Key.RightShift) {
				ShowColorTooltip();
			}
		}

		protected override void OnKeyUp(KeyEventArgs e)
		{
			base.OnKeyUp(e);
			if(e.Key == Key.LeftShift || e.Key == Key.RightShift) {
				TooltipHelper.HideTooltip(_picViewer);
			}
		}

		private void PicViewer_PointerExited(object? sender, PointerEventArgs e)
		{
			TooltipHelper.HideTooltip(_picViewer);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public static void OpenAtTile(AddressInfo tileAddr, TileFormat tileFormat, int selectedPalette, Window parent)
		{
			OpenAtTile(new List<AddressInfo>() { tileAddr }, 1, tileFormat, selectedPalette, parent);
		}

		public static void OpenAtTile(List<AddressInfo> tileAddresses, int columnCount, TileFormat tileFormat, int selectedPalette, Window parent)
		{
			for(int i = 0; i < tileAddresses.Count; i++) {
				AddressInfo addr = tileAddresses[i];
				if(addr.Type.IsRelativeMemory()) {
					tileAddresses[i] = DebugApi.GetAbsoluteAddress(addr);
				}
			}

			if(tileAddresses.Any(x => x.Address < 0)) {
				return;
			}

			TileEditorViewModel model = new(tileAddresses, columnCount, tileFormat, selectedPalette);
			TileEditorWindow wnd = DebugWindowManager.CreateDebugWindow<TileEditorWindow>(() => new TileEditorWindow(model));
			wnd.ShowCentered((Control)parent);
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			if(e.NotificationType == ConsoleNotificationType.GameLoaded) {
				Dispatcher.UIThread.Post(() => {
					Close();
				});
			}
		}
	}
}
