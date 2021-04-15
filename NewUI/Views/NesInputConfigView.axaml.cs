using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;
using Mesen.GUI;
using Mesen.ViewModels;
using Avalonia.Interactivity;
using Mesen.Windows;

namespace Mesen.Views
{
	public class NesInputConfigView : UserControl
	{
		public NesInputConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
