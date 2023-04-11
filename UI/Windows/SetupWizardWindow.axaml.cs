using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Platform;
using Mesen.ViewModels;
using System;

namespace Mesen.Windows
{
	public class SetupWizardWindow : MesenWindow
	{
		private SetupWizardViewModel _model;

		public SetupWizardWindow()
		{
			_model = new SetupWizardViewModel();
			DataContext = _model;
			InitializeComponent();

#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			//Manually center the window (Avalonia on Linux doesn't seem to
			//work with "CenterScreen" as startup location
			Screen? screen = Screens.ScreenFromVisual(this);
			if(screen != null) {
				PixelPoint pos = new PixelPoint(
					(int)((screen.Bounds.Width - Bounds.Width) / 2),
					(int)((screen.Bounds.Height - Bounds.Height) / 2)
				);
				Position = pos;
			}
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