using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using Mesen.Debugger;
using Mesen.Debugger.ViewModels;

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
	}
}
