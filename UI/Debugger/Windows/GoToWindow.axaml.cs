using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Metadata;
using Mesen.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Views;
using ReactiveUI.Fody.Helpers;
using System;
using System.Globalization;

namespace Mesen.Debugger.Windows
{
	public class GoToWindow : MesenWindow
	{
		public static readonly StyledProperty<string> AddressProperty = AvaloniaProperty.Register<GoToWindow, string>(nameof(Address), "");

		public string Address
		{
			get { return GetValue(AddressProperty); }
			set { SetValue(AddressProperty, value); }
		}

		public string HelpTooltip { get; } = ResourceHelper.GetMessage("GoToWindowHint");

		private static string _lastAddress = "";
		private int _maximum = 0;
		private CpuType _cpuType;
		private MemoryType _memType;

		[Obsolete("For designer only")]
		public GoToWindow() : this(CpuType.Snes, MemoryType.SnesMemory, 0) { }

		public GoToWindow(CpuType cpuType, MemoryType memType, int maximum)
		{
			Address = _lastAddress;
			_maximum = maximum;
			_cpuType = cpuType;
			_memType = memType;

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
			this.GetControl<TextBox>("txtAddress").FocusAndSelectAll();
		}

		private int GetEffectiveAddress()
		{
			string textValue = Address.Trim();

			CodeLabel? label = LabelManager.GetLabel(textValue);
			if(label != null) {
				if(label.GetAbsoluteAddress().Type == _memType) {
					return label.GetAbsoluteAddress().Address;
				} else {
					AddressInfo relAddr = label.GetRelativeAddress(_cpuType);
					if(relAddr.Address > 0 && relAddr.Type == _memType) {
						return relAddr.Address;
					}
				}
			}

			if(int.TryParse(textValue, out int numericValue)) {
				//numeric-only value, interpret as hex string (otherwise EvaluateExpression will parse it as a decimal string)
				if(!long.TryParse(textValue, NumberStyles.HexNumber, null, out long parsedValue) || parsedValue < 0 || parsedValue > _maximum) {
					return -1;
				}
				return (int)parsedValue;
			}

			long value = DebugApi.EvaluateExpression(textValue, _cpuType, out EvalResultType resultType, false);
			if(resultType != EvalResultType.Numeric || value < 0 || value > _maximum) {
				if(!long.TryParse(textValue, NumberStyles.HexNumber, null, out long parsedValue) || parsedValue < 0 || parsedValue > _maximum) {
					return -1;
				}
				return (int)parsedValue;
			}
			return (int)value;
		}

		[DependsOn(nameof(Address))]
		public bool CanOkClick(object parameter)
		{
			return GetEffectiveAddress() >= 0;
		}

		public void OkClick(object parameter)
		{
			int effectiveAddr = GetEffectiveAddress();
			if(effectiveAddr >= 0) {
				_lastAddress = Address;
				Close(effectiveAddr);
			}
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(null!);
		}
	}
}
