using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.Windows
{
	public class NesHeaderEditWindow : MesenWindow
	{
		NesHeaderEditViewModel _model;

		public NesHeaderEditWindow()
		{
			_model = new NesHeaderEditViewModel();
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

		private async void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			if(await _model.Save(this)) {
				Close();
			}
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}
	}
}
