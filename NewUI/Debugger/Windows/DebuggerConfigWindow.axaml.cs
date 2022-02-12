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
		private DebuggerConfigWindowViewModel _model;

		[Obsolete("For designer only")]
		public DebuggerConfigWindow() : this(new())
		{
		}

		public DebuggerConfigWindow(DebuggerConfigWindowViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = model;
			DataContext = model;
		}

		public static void Open(DebugConfigWindowTab tab, IRenderRoot? parent)
		{
			new DebuggerConfigWindow(new DebuggerConfigWindowViewModel(tab)).ShowCenteredDialog(parent);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_model.SaveConfig();
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}
	}
}