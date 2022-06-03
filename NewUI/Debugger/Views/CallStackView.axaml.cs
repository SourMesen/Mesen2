using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using DataBoxControl;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;

namespace Mesen.Debugger.Views
{
	public class CallStackView : UserControl
	{
		public CallStackView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is CallStackViewModel model) {
				model.InitContextMenu(this);
			}
			base.OnDataContextChanged(e);
		}

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataBox grid = (DataBox)sender;
			if(grid.Selection.SelectedItem is CallStackViewModel.StackInfo stack && DataContext is CallStackViewModel model) {
				model.GoToLocation(stack);
			}
		}
	}
}
