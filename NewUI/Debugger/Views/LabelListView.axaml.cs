using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Windows;
using System;
using static Mesen.Debugger.ViewModels.LabelListViewModel;
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

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is LabelListViewModel model) {
				model.InitContextMenu(this, this.FindControl<DataGrid>("DataGrid"));
			}
			base.OnDataContextChanged(e);
		}

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (DataGrid)sender;
			if(DataContext is LabelListViewModel listModel && grid.SelectedItem is LabelViewModel label) {
				LabelEditWindow.EditLabel(listModel.CpuType, this, label.Label);
			}
		}
	}
}
