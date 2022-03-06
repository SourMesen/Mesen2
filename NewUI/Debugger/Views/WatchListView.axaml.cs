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
using Mesen.Utilities;
using Mesen.Interop;
using Mesen.Debugger.Windows;
using Mesen.Debugger.Disassembly;

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

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is WatchListViewModel model) {
				MesenDataGrid grid = this.FindControl<MesenDataGrid>("DataGrid");
				model.InitContextMenu(this, grid);
			}
			base.OnDataContextChanged(e);
		}

		protected override void OnInitialized()
		{
			base.OnInitialized();
			
			MesenDataGrid grid = this.FindControl<MesenDataGrid>("DataGrid");
			grid.PreparingCellForEdit += Grid_PreparingCellForEdit;
		}

		private void Grid_PreparingCellForEdit(object? sender, DataGridPreparingCellForEditEventArgs e)
		{
			Dispatcher.UIThread.Post(() => {
				//This needs to be done as a post, otherwise the focus doesn't work properly
				//and the user can't type when using BeginEdit(). DataGrid bug?
				e.EditingElement?.Focus();
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

		private static bool IsTextKey(Key key)
		{
			return key >= Key.A && key <= Key.Z || key >= Key.D0 && key <= Key.D9 || key >= Key.NumPad0 && key <= Key.Divide || key >= Key.OemSemicolon && key <= Key.Oem102;
		}

		private void OnGridKeyDown(object sender, KeyEventArgs e)
		{
			if(e.Key == Key.Escape) {
				((DataGrid)sender).CancelEdit();
			} else if(IsTextKey(e.Key)) {
				((DataGrid)sender).CurrentColumn = ((DataGrid)sender).Columns[0];
				((DataGrid)sender).BeginEdit();
			}
		}
	}
}
