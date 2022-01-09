using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;

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

		private void OnGridDoubleClick(object sender, RoutedEventArgs e)
		{
			DataGrid grid = (DataGrid)sender;
			if(grid.SelectedItem is CallStackViewModel.StackInfo stack && DataContext is CallStackViewModel model) {
				bool isMapped = DebugApi.GetRelativeAddress(stack.Address, model.CpuType).Address >= 0;
				if(isMapped) {
					model.Disassembly.ScrollToAddress(stack.RelAddress);
				}
			}
		}
	}
}
