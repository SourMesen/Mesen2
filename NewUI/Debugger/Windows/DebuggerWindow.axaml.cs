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
				_model.Config.LoadWindowSettings(this);
				if(Design.IsDesignMode) {
					return;
				}

				AllPropertiesObserver observer = new(() => {
					_model.Config.ApplyConfig();

					Dispatcher.UIThread.Post(() => {
						UpdateDebugger();
					});
				});

				_model.Config.Changed.Subscribe(observer);
				_model.Config.Snes.Changed.Subscribe(observer);
				_model.Config.Nes.Changed.Subscribe(observer);
				_model.Config.Gameboy.Changed.Subscribe(observer);
			} else {
				throw new Exception("Invalid model");
			}
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
			_listener?.Dispose();
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

		private void Step(Int32 instructionCount, StepType type = StepType.Step)
		{
			DebugApi.Step(_model.CpuType, instructionCount, type);
			if(_model.Disassembly.StyleProvider is BaseStyleProvider p) {
				p.ActiveAddress = null;
				_model.Disassembly.DataProvider = new CodeDataProvider(_model.CpuType);
			}
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowOptionPanel = !_model.Config.ShowOptionPanel;
		}

		private void OnContinueClick(object sender, RoutedEventArgs e)
		{
			DebugApi.ResumeExecution();
			if(_model.Disassembly.StyleProvider is BaseStyleProvider p) {
				p.ActiveAddress = null;
				_model.Disassembly.DataProvider = new CodeDataProvider(_model.CpuType);
			}
		}

		private void OnStepIntoClick(object sender, RoutedEventArgs e)
		{
			Step(1, StepType.Step);
		}

		private void OnStepOutClick(object sender, RoutedEventArgs e)
		{
			Step(1, StepType.StepOut);
		}

		private void OnStepOverClick(object sender, RoutedEventArgs e)
		{
			Step(1, StepType.StepOver);
		}

		private void OnStepBackClick(object sender, RoutedEventArgs e)
		{
			//TODO
		}

		private void OnRunOnePpuCycleClick(object sender, RoutedEventArgs e)
		{
			Step(1, StepType.PpuStep);
		}

		private void OnRunOnePpuScanlineClick(object sender, RoutedEventArgs e)
		{
			//TODO
		}

		private void OnRunOnePpuFrameClick(object sender, RoutedEventArgs e)
		{
			//TODO
		}
	}
}
