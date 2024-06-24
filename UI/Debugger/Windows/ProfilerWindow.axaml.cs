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
	public class ProfilerWindow : MesenWindow, INotificationHandler
	{
		private ProfilerWindowViewModel _model;

		public ProfilerWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = new ProfilerWindowViewModel(this);
			DataContext = _model;
			
			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					Dispatcher.UIThread.Post(() => {
						_model.UpdateAvailableTabs();
					});
					break;

				case ConsoleNotificationType.CodeBreak:
					if(_model.Config.RefreshOnBreakPause) {
						_model.RefreshData();
					}
					break;

				case ConsoleNotificationType.PpuFrameDone:
					if(_model.Config.AutoRefresh && !ToolRefreshHelper.LimitFps(this, 10)) {
						_model.RefreshData();
					}
					break;
			}
		}

		private void OnCellDoubleClick(DataBoxCell cell)
		{
			if(_model.SelectedTab == null) {
				return;
			}

			int index = _model.SelectedTab.Selection.SelectedIndex;
			if(index >= 0) {
				ProfiledFunction? funcData = _model.SelectedTab.GetRawData(index);
				if(funcData != null) {
					AddressInfo relAddr = DebugApi.GetRelativeAddress(funcData.Value.Address, _model.SelectedTab.CpuType);
					if(relAddr.Address >= 0) {
						DebuggerWindow.OpenWindowAtAddress(_model.SelectedTab.CpuType, relAddr.Address);
					}
				}
			}
		}

		private void OnResetClick(object sender, RoutedEventArgs e)
		{
			_model.SelectedTab?.ResetData();
		}

		private void OnRefreshClick(object sender, RoutedEventArgs e)
		{
			_model.RefreshData();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
