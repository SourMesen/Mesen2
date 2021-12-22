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

		private void mnuAddBreakpoint_Click(object sender, RoutedEventArgs e)
		{
			Breakpoint bp = new Breakpoint() { BreakOnRead = true, BreakOnWrite = true, BreakOnExec = true };
			BreakpointEditWindow.EditBreakpoint(bp, this);
		}

		private void mnuEditBreakpoint_Click(object sender, RoutedEventArgs e)
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");
			Breakpoint? bp = grid.SelectedItem as Breakpoint;
			if(bp != null && grid != null) {
				BreakpointEditWindow.EditBreakpoint(bp, this);
			}
		}

		private void mnuDeleteBreakpoint_Click(object sender, RoutedEventArgs e)
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");
			foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
				if(item is Breakpoint bp) {
					BreakpointManager.RemoveBreakpoint(bp);
				}
			}
		}
	}
}
