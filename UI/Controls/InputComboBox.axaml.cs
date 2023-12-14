using Avalonia;
using Avalonia.Controls;
using Avalonia.Data;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.VisualTree;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using System;

namespace Mesen.Controls
{
	public class InputComboBox : UserControl
	{
		public static readonly StyledProperty<ControllerType> ControllerTypeProperty = AvaloniaProperty.Register<InputComboBox, ControllerType>(nameof(ControllerType), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<ControllerConfig> ConfigProperty = AvaloniaProperty.Register<InputComboBox, ControllerConfig>(nameof(Config), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> PortProperty = AvaloniaProperty.Register<InputComboBox, int>(nameof(Port), 0);

		public static readonly StyledProperty<Enum[]> AvailableValuesProperty = AvaloniaProperty.Register<InputComboBox, Enum[]>(nameof(AvailableValues));
		public static readonly StyledProperty<bool> SetupEnabledProperty = AvaloniaProperty.Register<InputComboBox, bool>(nameof(SetupEnabled));

		public ControllerType ControllerType
		{
			get { return GetValue(ControllerTypeProperty); }
			set { SetValue(ControllerTypeProperty, value); }
		}

		public ControllerConfig Config
		{
			get { return GetValue(ConfigProperty); }
			set { SetValue(ConfigProperty, value); }
		}

		public bool SetupEnabled
		{
			get { return GetValue(SetupEnabledProperty); }
			set { SetValue(SetupEnabledProperty, value); }
		}
		
		public int Port
		{
			get { return GetValue(PortProperty); }
			set { SetValue(PortProperty, value); }
		}

		public Enum[] AvailableValues
		{
			get { return GetValue(AvailableValuesProperty); }
			set { SetValue(AvailableValuesProperty, value); }
		}

		static InputComboBox()
		{
			ControllerTypeProperty.Changed.AddClassHandler<InputComboBox>((x, e) => {
				x.SetupEnabled = x.ControllerType.CanConfigure();
			});
		}

		public InputComboBox()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private async void btnSetup_Click(object sender, RoutedEventArgs e)
		{
			Button btn = (Button)sender;

			PixelPoint startPosition = btn.PointToScreen(new Point(-7, btn.Bounds.Height));
			
			ControllerConfigWindow wnd = new ControllerConfigWindow();
			ControllerConfig cfg = Config.Clone();
			wnd.DataContext = new ControllerConfigViewModel(ControllerType, cfg, Config, Port);
			
			if(await wnd.ShowDialogAtPosition<bool>(btn.GetVisualRoot() as Visual, startPosition)) {
				Config = cfg;
			}
		}
	}
}
