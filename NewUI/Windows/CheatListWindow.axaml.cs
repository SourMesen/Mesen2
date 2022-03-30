using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using System.ComponentModel;
using Avalonia.Data;
using Mesen.Interop;
using Mesen.Config;
using Avalonia.Input;
using Mesen.ViewModels;
using Avalonia.Data.Converters;

namespace Mesen.Windows
{
	public class CheatListWindow : Window
	{
		private CheatListWindowViewModel _model;
		private NotificationListener _listener;

		public CheatListWindow()
		{
			InitializeComponent();

			_listener = new NotificationListener();
			_listener.OnNotification += OnNotification;

			_model = new CheatListWindowViewModel();
			_model.InitActions(this.FindControl<DataGrid>("DataGrid"));
			DataContext = _model;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			ConfigManager.Config.Cheats.LoadWindowSettings(this);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_listener.Dispose();
			ConfigManager.Config.Cheats.SaveWindowSettings(this);
			CheatCodes.ApplyCheats();
			base.OnClosing(e);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_model.SaveCheats();
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}

		private void OnGridClick(object sender, TappedEventArgs e)
		{
			DataGrid? grid = (sender as DataGrid);
			if(grid?.SelectedItem is CheatCode cheat && grid.CurrentColumn.DisplayIndex == 0) {
				cheat.Enabled = !cheat.Enabled;
				_model.ApplyCheats();
			}
		}

		private async void OnGridDoubleClick(object sender, TappedEventArgs e)
		{
			DataGrid grid = (DataGrid)sender;
			if(grid?.SelectedItem is CheatCode cheat) {
				await CheatEditWindow.EditCheat(cheat, this);
				_model.ApplyCheats();
			}
		}

		public void OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.BeforeGameUnload:
					_model.SaveCheats();
					break;

				case ConsoleNotificationType.GameLoaded:
					_model.LoadCheats();
					break;

				case ConsoleNotificationType.BeforeEmulationStop:
					_model.SaveCheats();
					Dispatcher.UIThread.Post(() => {
						Close();
					});
					break;
			}
		}
	}

	public class CodeStringConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(targetType == typeof(string) && value is string str) {
				return string.Join(", ", str.Split(Environment.NewLine, StringSplitOptions.RemoveEmptyEntries));
			}
			throw new NotSupportedException();
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			throw new NotSupportedException();
		}
	}
}