using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using System.Collections.Generic;

namespace Mesen.Debugger.Controls
{
	public class DynamicTooltip : UserControl
	{
		public static readonly StyledProperty<List<TooltipEntry>> ItemsProperty = AvaloniaProperty.Register<DynamicTooltip, List<TooltipEntry>>(nameof(Items));

		public List<TooltipEntry> Items
		{
			get { return GetValue(ItemsProperty); }
			set { SetValue(ItemsProperty, value); }
		}

		public DynamicTooltip()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}

	public class TooltipEntry
	{
		public string Name { get; set; } = "";
		public string Value { get; set; } = "";

		public TooltipEntry(string name, string value)
		{
			Name = name;
			Value = value;
		}
	}
}
