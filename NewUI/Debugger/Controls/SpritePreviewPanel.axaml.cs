using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Avalonia.Data;
using Mesen.Debugger.ViewModels;

namespace Mesen.Debugger.Controls
{
	public class SpritePreviewPanel : UserControl
	{
		public SpritePreviewPanel()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
