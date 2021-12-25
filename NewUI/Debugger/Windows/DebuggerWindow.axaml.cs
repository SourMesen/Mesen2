#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Disassembly;
using Mesen.Interop;
using Avalonia.Interactivity;
using System.ComponentModel;
using Mesen.Debugger.Labels;
using Avalonia.Threading;
using Mesen.Utilities;
using Mesen.Config;
using Mesen.Debugger.Utilities;

namespace Mesen.Debugger.Windows
{
	public class DebuggerWindow : Window
	{
		private DebuggerWindowViewModel _model;
		private NotificationListener? _listener;

		public DebuggerWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(this.DataContext is DebuggerWindowViewModel model) {
				_model = model;
				_model.Disassembly.StyleProvider = new BaseStyleProvider();
				_model.InitializeMenu(this);
				_model.Config.LoadWindowSettings(this);
				if(Design.IsDesignMode) {
					return;
				}

				_model.Config.PropertyChanged += Config_PropertyChanged;
				_model.Config.Gameboy.PropertyChanged += Config_PropertyChanged;
				_model.Config.Nes.PropertyChanged += Config_PropertyChanged;
				_model.Config.Snes.PropertyChanged += Config_PropertyChanged;
			} else {
				throw new Exception("Invalid model");
			}
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			_model.Config.ApplyConfig();

			Dispatcher.UIThread.Post(() => {
				UpdateDebugger();
			});
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			_listener = new NotificationListener();
			_listener.OnNotification += _listener_OnNotification;
			UpdateDebugger();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_listener?.Dispose();
			_model.Cleanup();
			_model.Config.SaveWindowSettings(this);
			ConfigManager.SaveConfig();
			base.OnClosing(e);
		}

		private void UpdateDebugger()
		{
			_model.UpdateCpuPpuState();
			_model.UpdateDisassembly();
			_model.WatchList.UpdateWatch();
			_model.CallStack.UpdateCallStack();
		}

		private void _listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType == ConsoleNotificationType.CodeBreak) {
				Dispatcher.UIThread.Post(() => {
					UpdateDebugger();
				});
			}
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}
	}
}
