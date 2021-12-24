using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using Mesen.Config;
using Mesen.Debugger.ViewModels;

namespace Mesen.Debugger.Windows
{
	public class DebuggerConfigWindow : Window
	{
		private DebuggerConfigWindowViewModel? _model;

		public DebuggerConfigWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is DebuggerConfigWindowViewModel model) {
				_model = model;
			} else {
				throw new Exception("Invalid model");
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_model?.SaveConfig();
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}
	}
}