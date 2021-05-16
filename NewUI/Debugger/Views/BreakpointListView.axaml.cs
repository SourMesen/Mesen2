using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using Mesen.GUI.Debugger;
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
			BreakpointViewModel? bp = grid?.SelectedItem as BreakpointViewModel;
			if(bp != null && grid != null) {
				string? header = grid.CurrentColumn.Header.ToString();
				if(header == "E") {
					bp.Breakpoint.Enabled = !bp.Breakpoint.Enabled;
				} else if(header == "M") {
					bp.Breakpoint.MarkEvent = !bp.Breakpoint.MarkEvent;
					bp.Breakpoint.StartAddress++;
					bp.Breakpoint.EndAddress++;
					bp.Breakpoint.AddressType = BreakpointAddressType.AddressRange;
				}
			}
		}
	}
}
