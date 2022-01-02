using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.LogicalTree;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Threading;
using AvaloniaEdit;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class ProfilerWindow : Window
	{
		private NotificationListener? _listener = null;
		private ProfilerTab _selectedTab = null!;
		
		private ProfilerWindowViewModel Model => ((ProfilerWindowViewModel)DataContext!);
		private int _refreshCounter = 0;

		public ProfilerWindow()
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

			_selectedTab = Model.ProfilerTabs[0];
			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_listener?.Dispose();
			DataContext = null;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					Model.UpdateAvailableTabs();
					break;

				case ConsoleNotificationType.PpuFrameDone:
					_refreshCounter++;
					if(_refreshCounter == 3) { 
						//Refresh every 3 frames, ~20fps
						_selectedTab.RefreshData();
						Dispatcher.UIThread.Post(() => {
							_selectedTab.RefreshGrid();
						});
						_refreshCounter = 0;
					}
					break;
			}
		}

		private void OnTabSelected(object? sender, SelectionChangedEventArgs e)
		{
			if(e.AddedItems.Count > 0) {
				_selectedTab = (ProfilerTab)e.AddedItems[0]!;
			}
		}

		private void OnGridSort(object sender, DataGridColumnEventArgs e)
		{
			((ProfilerTab)((MesenDataGrid)sender).DataContext!).Sort(e.Column.DisplayIndex);
			e.Handled = true;
		}

		private void OnGridRowLoaded(object sender, DataGridRowEventArgs e)
		{
			if(e.Row.DataContext is ProfiledFunctionViewModel row) {
				((ProfilerTab)((MesenDataGrid)sender).DataContext!).UpdateRow(e.Row.GetIndex(), row);
			}
		}

		private void OnGridRowUnloaded(object sender, DataGridRowEventArgs e)
		{
			((ProfilerTab)((MesenDataGrid)sender).DataContext!).UnloadRow(e.Row.GetIndex());
		}

		private void OnResetClick(object sender, RoutedEventArgs e)
		{
			_selectedTab.ResetData();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
