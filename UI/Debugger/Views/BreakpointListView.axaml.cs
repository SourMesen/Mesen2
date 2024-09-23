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
			if(DataContext is BreakpointListViewModel bpList && cell.DataContext is BreakpointViewModel) {
				string? header = cell.Column?.Header?.ToString() ?? "";
				if(header == "E" || header == "M") {
					bool isEnabledColumn = header == "E";
					bool newValue = !bpList.Selection.SelectedItems.Any(bp => (isEnabledColumn ? bp?.Breakpoint.Enabled : bp?.Breakpoint.MarkEvent) == true);

					foreach(BreakpointViewModel? bp in bpList.Selection.SelectedItems) {
						if(bp != null) {
							if(isEnabledColumn) {
								bp.Breakpoint.Enabled = newValue;
							} else {
								if(!bp.Breakpoint.Forbid) {
									bp.Breakpoint.MarkEvent = newValue;
								}
							}
						}
					}

					DebugWorkspaceManager.AutoSave();
					BreakpointManager.RefreshBreakpoints();
				}
			}
		}

		private void OnCellDoubleClick(DataBoxCell cell)
		{
			if(cell.DataContext is BreakpointViewModel vm) {
				BreakpointEditWindow.EditBreakpoint(vm.Breakpoint, this);
			}
		}
	}
}
