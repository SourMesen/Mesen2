using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Controls;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.Windows
{
	public class BreakInWindow : MesenWindow
	{
		public static int _lastValue { get; set; } = 0;
		public static StepType _lastStepType { get; set; } = StepType.Step;
		
		public static readonly StyledProperty<int> ValueProperty = AvaloniaProperty.Register<BreakInWindow, int>(nameof(Value), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<StepType> StepTypeProperty = AvaloniaProperty.Register<BreakInWindow, StepType>(nameof(StepType));

		public int Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public StepType StepType
		{
			get { return GetValue(StepTypeProperty); }
			set { SetValue(StepTypeProperty, value); }
		}

		public bool ShowCpuCycles { get; } = false;

		private CpuType _cpuType;

		[Obsolete("For designer only")]
		public BreakInWindow() : this(CpuType.Snes) { }

		public BreakInWindow(CpuType cpuType)
		{
			_cpuType = cpuType;
			StepType = _lastStepType;
			Value = _lastValue;

			if(!Design.IsDesignMode) {
				ShowCpuCycles = DebugApi.GetDebuggerFeatures(cpuType).CpuCycleStep;
				if(!ShowCpuCycles && StepType == StepType.CpuCycleStep) {
					StepType = StepType.Step;
				}
			}

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			this.GetControl<MesenNumericTextBox>("txtValue").FocusAndSelectAll();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_lastValue = Value;
			_lastStepType = StepType;

			DebugApi.Step(_cpuType, Value, StepType);

			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}
	}
}
