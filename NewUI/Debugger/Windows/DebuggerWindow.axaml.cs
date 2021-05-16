#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Disassembly;
using Mesen.Interop;

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
		}

		int frmCnt = 0;
		private void _listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.PpuFrameDone) {
				return;
			}
			frmCnt++;

			ConfigApi.SetDebuggerFlag(DebuggerFlags.NesDebuggerEnabled, true);

			if(frmCnt % 200 == 0) {
				DebugApi.RefreshDisassembly(CpuType.Nes);
				string test = DebugApi.GetExecutionTrace(10000);
			}

			_model.Disassembly.DataProvider = new CodeDataProvider(CpuType.Nes);
			_model.Disassembly.UpdateMaxScroll();
		}
	}
}
