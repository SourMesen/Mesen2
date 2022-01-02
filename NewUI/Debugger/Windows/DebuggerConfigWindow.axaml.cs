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
using Mesen.Utilities;
using Avalonia.Rendering;

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
			}
		}

		public static void Open(DebugConfigWindowTab tab, IRenderRoot? parent)
		{
			new DebuggerConfigWindow() { DataContext = new DebuggerConfigWindowViewModel(tab) }.ShowCentered(parent);
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