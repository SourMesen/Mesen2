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
using Mesen.Debugger.Utilities;
using Mesen.Config;
using System;

namespace Mesen.Debugger.Views
{
	public class BreakpointListView : UserControl
	{
		public BreakpointListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is BreakpointListViewModel vm) {
				vm.InitContextMenu(this, this.FindControl<DataGrid>("DataGrid"));
			}
			base.OnDataContextChanged(e);
		}

		private void OnGridClick(object sender, RoutedEventArgs e)
		{
			DataGrid? grid = (sender as DataGrid);
			Breakpoint? bp = grid?.SelectedItem as Breakpoint;
			if(bp != null && grid != null) {
				string? header = grid.CurrentColumn.Header.ToString();
				if(header == "E") {
					bp.Enabled = !bp.Enabled;
					BreakpointManager.RefreshBreakpoints(bp);
				} else if(header == "M") {
					bp.MarkEvent = !bp.MarkEvent;
					BreakpointManager.RefreshBreakpoints(bp);
				}
			}
		}

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (DataGrid)sender;
			Breakpoint? bp = grid.SelectedItem as Breakpoint;
			if(bp != null && grid != null) {
				BreakpointEditWindow.EditBreakpoint(bp, this);
			}
		}
	}
}
