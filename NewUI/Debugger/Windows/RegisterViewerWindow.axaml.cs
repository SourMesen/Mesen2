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
	public class RegisterViewerWindow : Window, INotificationHandler
	{
		private RegisterViewerWindowViewModel _model;

		[Obsolete("For designer only")]
		public RegisterViewerWindow() : this(new()) { }

		public RegisterViewerWindow(RegisterViewerWindowViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = model;
			DataContext = model;

			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
			DataContext = null;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					_model.UpdateAvailableTabs(true);
					break;

				case ConsoleNotificationType.CodeBreak:
				case ConsoleNotificationType.PpuFrameDone:
					Dispatcher.UIThread.Post(() => {
						_model.UpdateAvailableTabs(false);
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
