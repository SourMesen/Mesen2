using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System.Linq;
using Mesen.Debugger.ViewModels;
using Avalonia.Threading;
using Avalonia.Input;
using Mesen.Debugger.Utilities;
using Mesen.Config;
using System;
using Mesen.Debugger.Controls;

namespace Mesen.Debugger.Views
{
	public class WatchListView : UserControl
	{
		private WatchListViewModel? _model;

		public WatchListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is WatchListViewModel model) {
				_model = model;
			}
			base.OnDataContextChanged(e);
		}

		protected override void OnInitialized()
		{
			base.OnInitialized();
			InitContextMenu();
		}

		private void InitContextMenu()
		{
			MesenDataGrid grid = this.FindControl<MesenDataGrid>("DataGrid");

			DebugShortcutManager.CreateContextMenu(this, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.WatchList_Delete,
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						_model!.DeleteWatch(grid.SelectedItems.Cast<WatchValueInfo>().ToList());
					}
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.MoveUp,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.WatchList_MoveUp,
					IsEnabled = () => grid.SelectedItems.Count == 1 && grid.SelectedIndex > 0,
					OnClick = () => {
						_model!.MoveUp(grid.SelectedIndex);
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.MoveDown,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.WatchList_MoveDown,
					IsEnabled = () => grid.SelectedItems.Count == 1 && grid.SelectedIndex < _model!.WatchEntries.Count - 2,
					OnClick = () => {
						_model!.MoveDown(grid.SelectedIndex);
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
