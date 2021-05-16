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
	public class OverscanConfig : UserControl
	{
		public static readonly StyledProperty<int> OverscanTopProperty = AvaloniaProperty.Register<OverscanConfig, int>(nameof(OverscanTop), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> OverscanBottomProperty = AvaloniaProperty.Register<OverscanConfig, int>(nameof(OverscanBottom), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> OverscanLeftProperty = AvaloniaProperty.Register<OverscanConfig, int>(nameof(OverscanLeft), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> OverscanRightProperty = AvaloniaProperty.Register<OverscanConfig, int>(nameof(OverscanRight), defaultBindingMode: BindingMode.TwoWay);

		public int OverscanTop
		{
			get { return GetValue(OverscanTopProperty); }
			set { SetValue(OverscanTopProperty, value); }
		}

		public int OverscanBottom
		{
			get { return GetValue(OverscanBottomProperty); }
			set { SetValue(OverscanBottomProperty, value); }
		}

		public int OverscanLeft
		{
			get { return GetValue(OverscanLeftProperty); }
			set { SetValue(OverscanLeftProperty, value); }
		}

		public int OverscanRight
		{
			get { return GetValue(OverscanRightProperty); }
			set { SetValue(OverscanRightProperty, value); }
		}

		public OverscanConfig()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
