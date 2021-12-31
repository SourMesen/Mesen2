#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using System.Collections.Generic;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Mesen.Debugger.Utilities;

namespace Mesen.Debugger.Windows
{
	public class SpriteViewerWindow : Window
	{
		private NotificationListener _listener;
		private SpriteViewerViewModel _model;

		[Obsolete("For designer only")]
		public SpriteViewerWindow() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public SpriteViewerWindow(CpuType cpuType, ConsoleType consoleType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PictureViewer picViewer = this.FindControl<PictureViewer>("picViewer");
			Grid spriteGrid = this.FindControl<Grid>("spriteGrid");
			_model = new SpriteViewerViewModel(cpuType, consoleType, picViewer, spriteGrid, this);
			DataContext = _model;
		
			_model.Config.LoadWindowSettings(this);
			_listener = new NotificationListener();

			if(Design.IsDesignMode) {
				return;
			}

			_listener.OnNotification += listener_OnNotification;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_listener.OnNotification += listener_OnNotification;
			_model.RefreshData();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_listener?.Dispose();
			_model.Config.SaveWindowSettings(this);
		}

		private void ScreenPreview_PositionClicked(object? sender, PositionClickedEventArgs e)
		{
			Point p = e.Position;
			if(_model.CpuType == CpuType.Cpu) {
				p = p.WithX(p.X - 256);
			}

			foreach(SpritePreviewModel sprite in _model.SpritePreviews) {
				if(
					p.X >= sprite.X && p.X < sprite.X + sprite.Width &&
					p.Y >= sprite.Y && p.Y < sprite.Y + sprite.Height
				) {
					_model.SelectedSprite = sprite;
					_model.UpdateSelection(sprite);
					break;
				}
			}
			e.Handled = true;
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			ToolRefreshHelper.ProcessNotification(this, e, _model.Config.RefreshTiming, _model.CpuType, _model.RefreshData);
		}
	}
}
