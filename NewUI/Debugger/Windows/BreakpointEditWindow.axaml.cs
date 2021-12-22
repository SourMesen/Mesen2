using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.ViewModels;
using Mesen.Utilities;

namespace Mesen.Debugger.Windows
{
	public class BreakpointEditWindow : Window
	{
		public BreakpointEditWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}

		public static async void EditBreakpoint(Breakpoint bp, Control parent)
		{
			Breakpoint copy = bp.Clone();
			BreakpointEditWindow wnd = new BreakpointEditWindow() {
				DataContext = new BreakpointEditViewModel(copy)
			};

			bool result = await wnd.ShowCenteredDialog<bool>(parent);
			if(result) {
				bp.CopyFrom(copy);
				BreakpointManager.AddBreakpoint(bp);
			}
		}
	}
}
