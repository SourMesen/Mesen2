using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System.Linq;
using Mesen.Debugger.ViewModels;
using Avalonia.Threading;
using Avalonia.Input;
using Mesen.Debugger.Utilities;
using Mesen.Config;

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

		protected override void OnInitialized()
		{
			base.OnInitialized();
			InitContextMenu();
		}

		private void InitContextMenu()
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");

			DebugShortcutManager.CreateContextMenu(this, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.WatchList_Delete,
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						((WatchListViewModel)DataContext!).DeleteWatch(grid.SelectedItems.Cast<WatchValueInfo>().ToList());
					}
				}
			});
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
			} else if(e.Key >= Key.A && e.Key <= Key.Z || e.Key >= Key.D0 && e.Key <= Key.D9) {
				((DataGrid)sender).CurrentColumn = ((DataGrid)sender).Columns[0];
				((DataGrid)sender).BeginEdit();
			}
		}
	}
}
