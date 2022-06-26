using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using DataBoxControl;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.ComponentModel;

namespace Mesen.Debugger.Windows
{
	public class WatchWindow : Window, INotificationHandler
	{
		private WatchWindowViewModel _model;

		[Obsolete("For designer only")]
		public WatchWindow() : this(new WatchWindowViewModel()) { }

		public WatchWindow(WatchWindowViewModel model)
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
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					_model.UpdateAvailableTabs();
					break;

				case ConsoleNotificationType.PpuFrameDone:
					if(FocusManager.Instance?.Current is TextBox) {
						return;
					}

					if(!ToolRefreshHelper.LimitFps(this, 30)) {
						_model.RefreshData();
					}
					break;
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
