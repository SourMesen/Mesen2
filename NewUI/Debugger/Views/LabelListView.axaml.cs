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
using Mesen.Interop;
using System;

namespace Mesen.Debugger.Views
{
	public class LabelListView : UserControl
	{
		private LabelListViewModel? _model;

		public LabelListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is LabelListViewModel model) {
				_model = model;
			}
			base.OnDataContextChanged(e);
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
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Add),
					OnClick = () => LabelEditWindow.EditLabel(this, new CodeLabel())
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Edit),
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						CodeLabel? label = grid.SelectedItem as CodeLabel;
						if(label != null) {
							LabelEditWindow.EditLabel(this, label);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Delete),
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
							if(item is CodeLabel label) {
								LabelManager.DeleteLabel(label, true);
							}
						}
					}
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.AddBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_AddBreakpoint),
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						CodeLabel? label = grid.SelectedItem as CodeLabel;
						if(label != null) {
							AddressInfo addr = label.GetAbsoluteAddress();
							BreakpointManager.AddBreakpoint(addr, _model!.CpuType);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.AddWatch,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_AddToWatch),
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						CodeLabel? label = grid.SelectedItem as CodeLabel;
						if(label != null) {
							AddressInfo addr = label.GetRelativeAddress(_model!.CpuType);
							if(addr.Address >= 0) {
								WatchManager.GetWatchManager(_model.CpuType).AddWatch("[$" + addr.Address.ToString("X2") + "]");
							}
						}
					}
				},
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
