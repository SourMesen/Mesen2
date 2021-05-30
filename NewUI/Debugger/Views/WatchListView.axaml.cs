using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using System.Linq;
using Mesen.ViewModels;
using Mesen.Debugger;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;
using Mesen.Utilities;
using Avalonia.Threading;
using Avalonia.Input;

namespace Mesen.Debugger.Views
{
	public class WatchListView : UserControl
	{
		public WatchListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnCellEditEnded(object? sender, DataGridCellEditEndedEventArgs e)
		{
			if(e.EditAction == DataGridEditAction.Commit) {
				Dispatcher.UIThread.Post(() => {
					int index = e.Row.GetIndex();
					((WatchListViewModel)DataContext!).EditWatch(index, ((WatchValueInfo)e.Row.DataContext!).Expression);
				});
			}
		}

		private void OnGridKeyDown(object sender, KeyEventArgs e)
		{
			if(e.Key == Key.Escape) {
				((DataGrid)sender).CancelEdit();
			} else if(e.Key == Key.Delete) {
				DeleteSelectedItems((DataGrid)sender);
			} else if(e.Key >= Key.A && e.Key <= Key.Z || e.Key >= Key.D0 && e.Key <= Key.D9) {
				((DataGrid)sender).CurrentColumn = ((DataGrid)sender).Columns[0];
				((DataGrid)sender).BeginEdit();
			}
		}

		private void mnuDeleteWatch_Click(object sender, RoutedEventArgs e)
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");
			DeleteSelectedItems(grid);
		}

		private void DeleteSelectedItems(DataGrid grid)
		{
			((WatchListViewModel)DataContext!).DeleteWatch(grid.SelectedItems.Cast<WatchValueInfo>().ToList());
		}
	}
}
