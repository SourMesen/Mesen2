using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.ComponentModel;

namespace Mesen.Debugger.Windows
{
	public class ProfilerWindow : Window, INotificationHandler
	{
		private ProfilerWindowViewModel _model;

		[Obsolete("For designer only")]
		public ProfilerWindow() : this(new ProfilerWindowViewModel()) { }

		public ProfilerWindow(ProfilerWindowViewModel model)
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
					_model.UpdateAvailableTabs();
					break;

				case ConsoleNotificationType.PpuFrameDone:
					if(ToolRefreshHelper.LimitFps(this, 10)) {
						_model.RefreshData();
					}
					break;
			}
		}

		private void OnGridSort(object sender, DataGridColumnEventArgs e)
		{
			_model.SelectedTab.Sort(e.Column.DisplayIndex);
			e.Handled = true;
		}

		private void OnGridRowLoaded(object sender, DataGridRowEventArgs e)
		{
			if(e.Row.DataContext is ProfiledFunctionViewModel row) {
				_model.SelectedTab.UpdateRow(e.Row.GetIndex(), row);
			}
		}

		private void OnGridRowUnloaded(object sender, DataGridRowEventArgs e)
		{
			_model.SelectedTab.UnloadRow(e.Row.GetIndex());
		}

		private void OnResetClick(object sender, RoutedEventArgs e)
		{
			_model.SelectedTab.ResetData();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
