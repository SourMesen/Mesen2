#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.ViewModels;
using Mesen.Windows;
using System;
using Avalonia.Media;
using System.Threading.Tasks;
using Avalonia.Interactivity;

namespace Mesen.Views
{
	public class PceConfigView : UserControl
	{
		public PceConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
