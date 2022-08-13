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

namespace Mesen.Debugger.Windows
{
	public class TileEditorWindow : Window, INotificationHandler
	{
		private TileEditorViewModel _model;

		[Obsolete("For designer only")]
		public TileEditorWindow() : this(new()) { }

		public TileEditorWindow(TileEditorViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PictureViewer picViewer = this.GetControl<ScrollPictureViewer>("picViewer").InnerViewer;
			picViewer.PositionClicked += PicViewer_PositionClicked;
			_model = model;
			_model.InitActions(picViewer, this);
			DataContext = _model;

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
		}

		private void PicViewer_PositionClicked(object? sender, PositionClickedEventArgs e)
		{
			_model.UpdatePixel(e.Position, e.Properties.IsRightButtonPressed);
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
