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
					ActionType = ActionType.Add,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.BreakpointList_Add,
					OnClick = () => {
						Breakpoint bp = new Breakpoint() { BreakOnRead = true, BreakOnWrite = true, BreakOnExec = true };
						BreakpointEditWindow.EditBreakpoint(bp, this);
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.BreakpointList_Edit,
					IsEnabled = () => grid.SelectedItem is Breakpoint,
					OnClick = () => {
						Breakpoint? bp = grid.SelectedItem as Breakpoint;
						if(bp != null && grid != null) {
							BreakpointEditWindow.EditBreakpoint(bp, this);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.BreakpointList_Delete,
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
							if(item is Breakpoint bp) {
								BreakpointManager.RemoveBreakpoint(bp);
							}
						}
					}
				}
			});
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
