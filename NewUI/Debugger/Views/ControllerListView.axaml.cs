using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;

namespace Mesen.Debugger.Views
{
	public class ControllerListView : UserControl
	{
		public ControllerListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
