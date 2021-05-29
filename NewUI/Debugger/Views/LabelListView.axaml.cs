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

		private void OnCellPointerPressed(object sender, DataGridCellPointerPressedEventArgs e)
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");
			grid.SelectedIndex = e.Row.GetIndex();
		}

		private void OnGridClick(object sender, RoutedEventArgs e)
		{

		}

		private async void mnuAddLabel_Click(object sender, RoutedEventArgs e)
		{
			CodeLabel newLabel = new CodeLabel();
			LabelEditWindow wnd = new LabelEditWindow() {
				DataContext = new LabelEditViewModel(newLabel)
			};

			bool result = await wnd.ShowCenteredDialog<bool>(this);
			if(result) {
				LabelManager.SetLabel(newLabel, true);
				((LabelListViewModel)DataContext!).UpdateLabelList();
			}
		}

		private void mnuEditLabel_Click(object sender, RoutedEventArgs e)
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");
			CodeLabel ? label = grid.SelectedItem as CodeLabel;
			if(label != null && grid != null) {
				EditLabel(label);
			}
		}

		private void mnuDeleteLabel_Click(object sender, RoutedEventArgs e)
		{
			DataGrid grid = this.FindControl<DataGrid>("DataGrid");
			CodeLabel? label = grid.SelectedItem as CodeLabel;
			if(label != null && grid != null) {
				LabelManager.DeleteLabel(label, true);
				((LabelListViewModel)DataContext!).UpdateLabelList();
			}
		}

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (DataGrid)sender;
			CodeLabel? label = grid.SelectedItem as CodeLabel;
			if(label != null && grid != null) {
				EditLabel(label);
			}
		}

		private async void EditLabel(CodeLabel label)
		{
			CodeLabel copy = label.Clone();
			LabelEditWindow wnd = new LabelEditWindow() {
				DataContext = new LabelEditViewModel(copy, label)
			};

			bool result = await wnd.ShowCenteredDialog<bool>(this);
			if(result) {
				label.CopyFrom(copy);
				LabelManager.SetLabel(label, true);
			}
		}
	}
}
