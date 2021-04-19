using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.GUI;
using ReactiveUI;
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.ComponentModel;
using Mesen.GUI.Config;

namespace Mesen.Windows
{
	public class ColorPickerWindow : Window
	{
		public ColorPickerWindow()
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
	}
}