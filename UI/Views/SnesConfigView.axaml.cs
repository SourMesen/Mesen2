using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Config;
using Avalonia.Interactivity;
using System;
using Mesen.ViewModels;

namespace Mesen.Views
{
	public class SnesConfigView : UserControl
	{
		public SnesConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void nudGsuSpeed_LostFocus(object? sender, RoutedEventArgs e)
		{
			if(DataContext is SnesConfigViewModel model) {
				//Clock speed must be a multiple of 100
				model.Config.GsuClockSpeed = (uint)Math.Round((double)model.Config.GsuClockSpeed / 100) * 100;
			}
		}
	}
}
