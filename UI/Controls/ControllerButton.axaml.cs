using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using System;

namespace Mesen.Controls
{
	public class ControllerButton : UserControl
	{
		public static readonly StyledProperty<UInt32> KeyBindingProperty = AvaloniaProperty.Register<ControllerButton, UInt32>(nameof(KeyBinding), 0, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<UInt32> TurboKeyBindingProperty = AvaloniaProperty.Register<ControllerButton, UInt32>(nameof(TurboKeyBinding), 0, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<string> LabelProperty = AvaloniaProperty.Register<ControllerButton, string>(nameof(Label));
		public static readonly StyledProperty<bool> HasTurboProperty = AvaloniaProperty.Register<ControllerButton, bool>(nameof(HasTurbo), true);

		public UInt32 KeyBinding
		{
			get { return GetValue(KeyBindingProperty); }
			set { SetValue(KeyBindingProperty, value); }
		}

		public UInt32 TurboKeyBinding
		{
			get { return GetValue(TurboKeyBindingProperty); }
			set { SetValue(TurboKeyBindingProperty, value); }
		}

		public string Label
		{
			get { return GetValue(LabelProperty); }
			set { SetValue(LabelProperty, value); }
		}

		public bool HasTurbo
		{
			get { return GetValue(HasTurboProperty); }
			set { SetValue(HasTurboProperty, value); }
		}

		public ControllerButton()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
