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

namespace Mesen.Controls
{
	public class OverscanInput : UserControl
	{
		public static readonly StyledProperty<OverscanConfig> OverscanProperty = AvaloniaProperty.Register<OverscanInput, OverscanConfig>(nameof(Overscan), new OverscanConfig(), defaultBindingMode: BindingMode.TwoWay);

		public OverscanConfig Overscan
		{
			get { return GetValue(OverscanProperty); }
			set { SetValue(OverscanProperty, value); }
		}

		public OverscanInput()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
