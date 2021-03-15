using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using Mesen.GUI.Debugger;

namespace Mesen.Views
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

		private void Tapped(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (sender as DataGrid);
			BreakpointViewModel bp = (BreakpointViewModel)grid.SelectedItem;
			if(bp != null) {
				if(grid.CurrentColumn.Header.ToString().Length == 1) {
					if(grid.CurrentColumn.Header.ToString() == "E") {
						bp.Breakpoint.Enabled = !bp.Breakpoint.Enabled;
					} else if(grid.CurrentColumn.Header.ToString() == "M") {
						bp.Breakpoint.MarkEvent = !bp.Breakpoint.MarkEvent;
						bp.Breakpoint.StartAddress++;
						bp.Breakpoint.EndAddress++;
						bp.Breakpoint.AddressType = BreakpointAddressType.AddressRange;
					}
				}
			}
		}

		private void DoubleTapped(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (sender as DataGrid);
			Breakpoint bp = grid.SelectedItem as Breakpoint;
			if(bp != null) {
				if(grid.CurrentColumn.Header.ToString().Length > 1) {

				}
			}
		}
	}
}
