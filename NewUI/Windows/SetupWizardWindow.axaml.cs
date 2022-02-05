using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;

namespace Mesen.Windows
{
	public class SetupWizardWindow : Window
	{
		private SetupWizardViewModel _model;

		public SetupWizardWindow()
		{
			_model = new SetupWizardViewModel();
			DataContext = _model;
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void btnOk_OnClick(object? sender, RoutedEventArgs e)
		{
			if(_model.Confirm(this)) {
				Close();
			}
		}

		private void lblCancel_Tapped(object? sender, TappedEventArgs e)
		{
			Close();
		}

		private void XboxIcon_Tapped(object? sender, TappedEventArgs e)
		{
			_model.EnableXboxMappings = !_model.EnableXboxMappings;
		}

		private void PsIcon_Tapped(object? sender, TappedEventArgs e)
		{
			_model.EnablePsMappings = !_model.EnablePsMappings;
		}

		private void WasdIcon_Tapped(object? sender, TappedEventArgs e)
		{
			_model.EnableWasdMappings = !_model.EnableWasdMappings;
		}

		private void ArrowIcon_Tapped(object? sender, TappedEventArgs e)
		{
			_model.EnableArrowMappings = !_model.EnableArrowMappings;
		}
	}
}