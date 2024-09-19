using Avalonia.Controls;
using Avalonia.Controls.Selection;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.RegisterViewer;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.ViewModels
{
	public class RegisterViewerWindowViewModel : DisposableViewModel, ICpuTypeModel
	{
		[Reactive] public List<RegisterViewerTab> Tabs { get; set; } = new List<RegisterViewerTab>();
		
		public RegisterViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public List<object> FileMenuActions { get; private set; } = new();
		[Reactive] public List<object> ViewMenuActions { get; private set; } = new();

		private BaseState? _state = null;
		
		public CpuType CpuType
		{
			get => _romInfo.ConsoleType.GetMainCpuType();
			set { }
		}

		private RomInfo _romInfo = new RomInfo();
		private byte _snesReg4210;
		private byte _snesReg4211;
		private byte _snesReg4212;

		public RegisterViewerWindowViewModel()
		{
			Config = ConfigManager.Config.Debug.RegisterViewer.Clone();
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming, CpuType);

			if(Design.IsDesignMode) {
				return;
			}

			UpdateRomInfo();
			RefreshTiming.UpdateMinMaxValues(CpuType);
			RefreshData();
		}

		public void InitMenu(Window wnd)
		{
			FileMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			});

			ViewMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.RefreshTiming.AutoRefresh,
					OnClick = () => Config.RefreshTiming.AutoRefresh = !Config.RefreshTiming.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshTiming.RefreshOnBreakPause,
					OnClick = () => Config.RefreshTiming.RefreshOnBreakPause = !Config.RefreshTiming.RefreshOnBreakPause
				}
			});

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		public void UpdateRomInfo()
		{
			_romInfo = EmuApi.GetRomInfo();
		}

		public void RefreshData()
		{
			if(_romInfo.ConsoleType == ConsoleType.Snes) {
				_snesReg4210 = DebugApi.GetMemoryValue(MemoryType.SnesMemory, 0x4210);
				_snesReg4211 = DebugApi.GetMemoryValue(MemoryType.SnesMemory, 0x4211);
				_snesReg4212 = DebugApi.GetMemoryValue(MemoryType.SnesMemory, 0x4212);
				_state = DebugApi.GetConsoleState<SnesState>(ConsoleType.Snes);
			} else if(_romInfo.ConsoleType == ConsoleType.Nes) {
				_state = DebugApi.GetConsoleState<NesState>(ConsoleType.Nes);
			} else if(_romInfo.ConsoleType == ConsoleType.Gameboy) {
				_state = DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy);
			} else if(_romInfo.ConsoleType == ConsoleType.PcEngine) {
				_state = DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine);
			} else if(_romInfo.ConsoleType == ConsoleType.Sms) {
				_state = DebugApi.GetConsoleState<SmsState>(ConsoleType.Sms);
			} else if(_romInfo.ConsoleType == ConsoleType.Gba) {
				_state = DebugApi.GetConsoleState<GbaState>(ConsoleType.Gba);
			} else if(_romInfo.ConsoleType == ConsoleType.Ws) {
				_state = DebugApi.GetConsoleState<WsState>(ConsoleType.Ws);
			}

			Dispatcher.UIThread.Post(() => {
				RefreshTabs();
			});
		}

		public void RefreshTabs()
		{
			if(_state == null) {
				return;
			}

			List<RegisterViewerTab> tabs = new List<RegisterViewerTab>();
			BaseState lastState = _state;

			if(lastState is SnesState snesState) {
				tabs = SnesRegisterViewer.GetTabs(ref snesState, _romInfo.CpuTypes, _snesReg4210, _snesReg4211, _snesReg4212);
			} else if(lastState is NesState nesState) {
				tabs = NesRegisterViewer.GetTabs(ref nesState);
			} else if(lastState is GbState gbState) {
				tabs = GbRegisterViewer.GetTabs(ref gbState);
			} else if(lastState is PceState pceState) {
				tabs = PceRegisterViewer.GetTabs(ref pceState);
			} else if(lastState is SmsState smsState) {
				tabs = SmsRegisterViewer.GetTabs(ref smsState, _romInfo.Format);
			} else if(lastState is GbaState gbaState) {
				tabs = GbaRegisterViewer.GetTabs(ref gbaState);
			} else if(lastState is WsState wsState) {
				tabs = WsRegisterViewer.GetTabs(ref wsState);
			}

			foreach(RegisterViewerTab tab in tabs) {
				tab.ColumnWidths = Config.ColumnWidths;
			}

			if(Tabs.Count != tabs.Count) {
				Tabs = tabs;
			} else {
				for(int i = 0; i < Tabs.Count; i++) {
					Tabs[i].SetData(tabs[i].Data);
					Tabs[i].TabName = tabs[i].TabName;
				}
			}
		}

		public void OnGameLoaded()
		{
			UpdateRomInfo();
			RefreshData();
		}
	}

	public class RegisterViewerTab : ReactiveObject
	{
		private string _name;
		private List<RegEntry> _data;

		public string TabName { get => _name; set => this.RaiseAndSetIfChanged(ref _name, value); }
		public List<RegEntry> Data { get => _data; set => this.RaiseAndSetIfChanged(ref _data, value); }
		public SelectionModel<RegEntry?> Selection { get; set; } = new();
		public List<int> ColumnWidths { get; set; } = new();

		public CpuType? CpuType { get; }
		public MemoryType? MemoryType { get; }

		public RegisterViewerTab(string name, List<RegEntry> data, CpuType? cpuType = null, MemoryType? memoryType = null)
		{
			_name = name;
			_data = data;
			CpuType = cpuType;
			MemoryType = memoryType;
		}

		public void SetData(List<RegEntry> rows)
		{
			if(Data.Count != rows.Count) {
				Data = rows;
			} else {
				for(int i = 0; i < rows.Count; i++) {
					Data[i].Value = rows[i].Value;
					Data[i].ValueHex = rows[i].ValueHex;
				}
			}
		}
	}

	public class RegEntry : INotifyPropertyChanged
	{
		private static ISolidColorBrush HeaderBgBrush = new SolidColorBrush(0x40B0B0B0);

		public string Address { get; private set; }
		public string Name { get; private set; }
		public bool IsEnabled { get; private set; }
		public IBrush Background { get; private set; }

		private string _value;
		private string _valueHex;

		public event PropertyChangedEventHandler? PropertyChanged;

		public string Value
		{ 
			get => _value;
			set {
				if(_value != value) {
					_value = value;
					PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Value)));
				}
			}
		}

		public string ValueHex
		{
			get => _valueHex;
			set
			{
				if(_valueHex != value) {
					_valueHex = value;
					PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ValueHex)));
				}
			}
		}

		public RegEntry(string reg, string name, ISpanFormattable? value, Format format = Format.None)
		{
			Init(reg, name, value, format);
		}

		public RegEntry(string reg, string name, bool value)
		{
			Init(reg, name, value, Format.None);
		}

		public RegEntry(string reg, string name, Enum value)
		{
			Init(reg, name, value, Format.X8);
		}

		public RegEntry(string reg, string name)
		{
			Init(reg, name, null, Format.None);
		}

		public RegEntry(string reg, string name, string textValue, IConvertible? rawValue)
		{
			Init(reg, name, textValue, Format.None);
			if(rawValue != null) {
				_valueHex = GetHexValue(rawValue, Format.X8);
			}
		}

		[MemberNotNull(nameof(Address), nameof(Name), nameof(Background), nameof(_value), nameof(_valueHex))]
		private void Init(string reg, string name, object? value, Format format)
		{
			if(format == Format.None && !(value is bool)) {
				//Display hex values for everything except booleans
				format = Format.X8;
			}

			Address = reg;
			Name = name;

			_value = GetValue(value);

			if(value is Enum) {
				_valueHex = GetHexValue(Convert.ToInt64(value), Format.X8);
			} else {
				_valueHex = GetHexValue(value, format);
			}

			Background = value == null ? RegEntry.HeaderBgBrush : Brushes.Transparent;
			IsEnabled = value != null;
		}

		private string GetValue(object? value)
		{
			if(value is string str) {
				return str;
			} else if(value is bool) {
				return (bool)value ? "☑ true" : "☐ false";
			} else if(value is IFormattable formattable) {
				return formattable.ToString() ?? "";
			} else if(value == null) {
				return "";
			}
			throw new Exception("Unsupported type");
		}

		private string GetHexValue(object? value, Format format)
		{
			if(value == null || value is string) {
				return "";
			}

			if(value is bool boolValue && format != Format.None) {
				return boolValue ? "$01" : "$00";
			}

			switch(format) {
				default: return "";
				case Format.X8: return "$" + ((IFormattable)value).ToString("X2", null);
				case Format.X16: return "$" + ((IFormattable)value).ToString("X4", null);

				case Format.X24: {
					string str = ((IFormattable)value).ToString("X6", null);
					return "$" + (str.Length > 7 ? str.Substring(str.Length - 6) : str);
				}

				case Format.X28: {
					string str = ((IFormattable)value).ToString("X7", null);
					return "$" + (str.Length > 7 ? str.Substring(str.Length - 7) : str);
				}
				case Format.X32: return "$" + ((IFormattable)value).ToString("X8", null);
			}
		}

		public int StartAddress
		{
			get
			{
				string addr = Address;
				if(addr.StartsWith("$")) {
					addr = addr.Substring(1);
				}
				int separator = addr.IndexOfAny(new char[] { '.', '-', '/' });
				if(separator >= 0) {
					addr = addr.Substring(0, separator);
				}

				if(Int32.TryParse(addr.Trim(), System.Globalization.NumberStyles.HexNumber, null, out int startAddress)) {
					return startAddress;
				}

				return -1;
			}
		}

		public int EndAddress
		{
			get
			{
				string addr = Address;
				int separator = addr.IndexOfAny(new char[] { '.', '-', '/' });
				if(separator >= 0) {
					if(addr[separator] == '.') {
						//First separator is a dot for bits, assume there is no end address
						return StartAddress;
					}
					addr = addr.Substring(separator + 1).Trim();
				} else {
					return StartAddress;
				}

				int lastRangeAddr = addr.LastIndexOf('/');
				if(lastRangeAddr >= 0) {
					addr = addr.Substring(lastRangeAddr + 1);
				}

				if(addr.StartsWith("$")) {
					addr = addr.Substring(1);
				}

				separator = addr.IndexOfAny(new char[] { '.', '-', '/' });
				if(separator >= 0) {
					addr = addr.Substring(0, separator);
				}

				if(Int32.TryParse(addr.Trim(), System.Globalization.NumberStyles.HexNumber, null, out int endAddress)) {
					if(addr.Length == 1) {
						return (StartAddress & ~0xF) | endAddress;
					} else {
						return endAddress;
					}
				}

				return StartAddress;
			}
		}

		public enum Format
		{
			None,
			X8,
			X16,
			X24,
			X28,
			X32
		}
	}
}
