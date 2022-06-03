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
using static Mesen.Debugger.ViewModels.BreakpointListViewModel;
using Avalonia.Input;
using DataBoxControl;

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
				vm.InitContextMenu(this);
			}
			base.OnDataContextChanged(e);
		}

		private void OnCellClick(DataBoxCell cell)
		{
			if(cell.DataContext is BreakpointViewModel vm) {
				Breakpoint bp = vm.Breakpoint;
				string? header = cell.Column?.Header?.ToString() ?? "";
				if(header == "E" || header == "M") {
					BreakpointManager.RefreshBreakpoints(bp);
				}
			}
		}

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataBox grid = (DataBox)sender;
			if(grid?.Selection.SelectedItem is BreakpointViewModel vm) {
				BreakpointEditWindow.EditBreakpoint(vm.Breakpoint, this);
			}
		}
	}
}
