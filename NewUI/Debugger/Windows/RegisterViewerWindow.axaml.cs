using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.ComponentModel;

namespace Mesen.Debugger.Windows
{
	public class RegisterViewerWindow : Window
	{
		private NotificationListener? _listener = null;
		private RegisterViewerWindowViewModel Model => ((RegisterViewerWindowViewModel)DataContext!);

		public RegisterViewerWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_listener?.Dispose();
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					Model.UpdateAvailableTabs(true);
					break;

				case ConsoleNotificationType.CodeBreak:
				case ConsoleNotificationType.PpuFrameDone:
					Dispatcher.UIThread.Post(() => {
						Model.UpdateAvailableTabs(false);
					});
					break;
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
