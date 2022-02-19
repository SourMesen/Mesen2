using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.Labels;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;

namespace Mesen.Debugger.Windows
{
	public class LabelEditWindow : Window
	{
		public LabelEditWindow()
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

		public static async void EditLabel(CpuType cpuType, Control parent, CodeLabel label)
		{
			CodeLabel copy = label.Clone();
			LabelEditViewModel model = new LabelEditViewModel(cpuType, copy, label);
			LabelEditWindow wnd = new LabelEditWindow() { DataContext = model };

			bool result = await wnd.ShowCenteredDialog<bool>(parent);
			if(result) {
				model.Commit();
				label.CopyFrom(copy);
				LabelManager.SetLabel(label, true);
			}

			model.Dispose();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}
