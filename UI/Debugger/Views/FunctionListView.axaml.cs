using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Windows;
using System;
using Avalonia.Input;
using DataBoxControl;
using static Mesen.Debugger.ViewModels.FunctionListViewModel;

namespace Mesen.Debugger.Views
{
	public class FunctionListView : UserControl
	{
		public FunctionListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is FunctionListViewModel model) {
				model.InitContextMenu(this);
			}
			base.OnDataContextChanged(e);
		}

		private void OnCellDoubleClick(DataBoxCell cell)
		{
			if(DataContext is FunctionListViewModel listModel && cell.DataContext is FunctionViewModel vm && vm.RelAddress >= 0) {
				listModel.Debugger.ScrollToAddress(vm.RelAddress);
			}
		}
	}
}
