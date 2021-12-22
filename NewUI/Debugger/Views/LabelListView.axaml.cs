using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using Mesen.Debugger;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Windows;
using Mesen.Utilities;
using Avalonia.Input;
using System.Linq;
using Mesen.Debugger.Utilities;
using Avalonia.Data;
using Mesen.Config;

namespace Mesen.Debugger.Views
{
	public class LabelListView : UserControl
	{
		public LabelListView()
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
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.LabelList_Add,
					OnClick = () => LabelEditWindow.EditLabel(this, new CodeLabel())
				},
				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.LabelList_Edit,
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						CodeLabel? label = grid.SelectedItem as CodeLabel;
						if(label != null && grid != null) {
							LabelEditWindow.EditLabel(this, label);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.LabelList_Delete,
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
							if(item is CodeLabel label) {
								LabelManager.DeleteLabel(label, true);
							}
						}
						((LabelListViewModel)DataContext!).UpdateLabelList();
					}
				}
			});
		}

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (DataGrid)sender;
			CodeLabel? label = grid.SelectedItem as CodeLabel;
			if(label != null && grid != null) {
				LabelEditWindow.EditLabel(this, label);
			}
		}
	}
}
