using Avalonia.Controls;
using Avalonia.Controls.Selection;
using Avalonia.Threading;
using DataBoxControl;
using DynamicData;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Linq;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels;

public class MemorySearchViewModel : DisposableViewModel
{
	public MemorySearchConfig Config { get; }

	[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();

	[Reactive] public MemoryType MemoryType { get; set; } = MemoryType.SnesMemory;
	[Reactive] public MemorySearchFormat Format { get; set; } = MemorySearchFormat.Hex;
	[Reactive] public MemorySearchValueSize ValueSize { get; set; } = MemorySearchValueSize.Byte;

	[Reactive] public MemorySearchCompareTo CompareTo { get; set; } = MemorySearchCompareTo.PreviousRefreshValue;
	[Reactive] public MemorySearchOperator Operator { get; set; } = MemorySearchOperator.Equal;

	[Reactive] public MesenList<MemoryAddressViewModel> ListData { get; private set; } = new();
	[Reactive] public SelectionModel<MemoryAddressViewModel> Selection { get; set; } = new();
	[Reactive] public SortState SortState { get; set; } = new();
	public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.MemorySearch.ColumnWidths;

	[Reactive] public int SpecificAddress { get; set; } = 0;
	[Reactive] public int SpecificValue { get; set; } = 0;

	[Reactive] public bool IsValueHex { get; set; }

	[Reactive] public string MinValue { get; set; } = "";
	[Reactive] public string MaxValue { get; set; } = "";

	[Reactive] public int MaxAddress { get; set; } = 0;
	
	[Reactive] public bool IsUndoEnabled { get; set; } = false;
	[Reactive] public bool IsSpecificValueEnabled { get; set; } = false;
	[Reactive] public bool IsSpecificAddressEnabled { get; set; } = false;
	
	public int[] AddressLookup => _addressLookup;
	public byte[] MemoryState => _memoryState;
	public byte[] PrevMemoryState => _prevMemoryState;

	private List<MemoryAddressViewModel> _innerData = new();

	private int[] _addressLookup = Array.Empty<int>();
	
	private byte[] _lastSearchSnapshot = Array.Empty<byte>();
	private HashSet<int> _hiddenAddresses = new();	
	
	private List<SearchHistory> _undoHistory = new();

	private byte[] _memoryState = Array.Empty<byte>();
	private byte[] _prevMemoryState = Array.Empty<byte>();

	private bool _isRefreshing;

	public MemorySearchViewModel()
	{
		Config = ConfigManager.Config.Debug.MemorySearch;

		if(Design.IsDesignMode) {
			return;
		}

		OnGameLoaded();
		SortState.SetColumnSort("Address", ListSortDirection.Ascending, false);

		AddDisposable(this.WhenAnyValue(x => x.MemoryType).Subscribe(x => {
			ResetSearch();
		}));
		
		AddDisposable(this.WhenAnyValue(x => x.Operator, x => x.CompareTo, x => x.ValueSize, x => x.Format).Subscribe(x => {
			IsSpecificValueEnabled = CompareTo == MemorySearchCompareTo.SpecificValue;
			IsSpecificAddressEnabled = CompareTo == MemorySearchCompareTo.SpecificAddress;

			IsValueHex = Format == MemorySearchFormat.Hex;
			bool isSigned = Format == MemorySearchFormat.Signed;
			switch(ValueSize) {
				case MemorySearchValueSize.Byte:
					MinValue = isSigned ? sbyte.MinValue.ToString() : byte.MinValue.ToString();
					MaxValue = isSigned ? sbyte.MaxValue.ToString() : byte.MaxValue.ToString();
					break;

				case MemorySearchValueSize.Word:
					MinValue = isSigned ? Int16.MinValue.ToString() : UInt16.MinValue.ToString();
					MaxValue = isSigned ? Int16.MaxValue.ToString() : UInt16.MaxValue.ToString();
					break;

				case MemorySearchValueSize.Dword:
					MinValue = isSigned ? Int32.MinValue.ToString() : UInt32.MinValue.ToString();
					MaxValue = isSigned ? Int32.MaxValue.ToString() : UInt32.MaxValue.ToString();
					break;
			}
			RefreshList(true);
		}));

		AddDisposable(this.WhenAnyValue(x => x.SpecificValue, x => x.SpecificAddress).Subscribe(x => {
			RefreshList(false);
		}));
	}

	public void SortCommand()
	{
		RefreshList(true);
	}

	public void OnGameLoaded()
	{
		AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => t.SupportsMemoryViewer() && !t.IsRelativeMemory() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
		MemoryType = EmuApi.GetRomInfo().ConsoleType.GetMainCpuType().GetSystemRamType();
	}

	public void RefreshData(bool forceSort)
	{
		_prevMemoryState = _memoryState;
		_memoryState = DebugApi.GetMemoryState(MemoryType);

		Dispatcher.UIThread.Post(() => {
			RefreshList(forceSort);
		});
	}

	private void RefreshList(bool forceSort)
	{
		byte[] memoryState = _memoryState;
		byte[] prevMemoryState = _prevMemoryState;

		if(memoryState.Length != prevMemoryState.Length) {
			//Mismatching arrays - happens when changing game or mem type
			return;
		}

		if(_isRefreshing) {
			return;
		}

		_isRefreshing = true;
		List<Tuple<string, ListSortDirection>> sortOrder = new(SortState.SortOrder);

		Task.Run(() => {
			bool isDefaultSort = sortOrder.Count == 1 && sortOrder[0].Item2 == ListSortDirection.Ascending && sortOrder[0].Item1 == "Address";
			if(!isDefaultSort || forceSort) {
				Sort(sortOrder);
			}

			Dispatcher.UIThread.Post(() => RefreshUiList(memoryState));
		});
	}

	private void RefreshUiList(byte[] memoryState)
	{
		if(Disposed) {
			return;
		}

		MemoryType memType = MemoryType;

		if(_innerData.Count < memoryState.Length) {
			_innerData.AddRange(Enumerable.Range(_innerData.Count, memoryState.Length - _innerData.Count).Select(i => new MemoryAddressViewModel(i, this)));
		} else if(_innerData.Count > memoryState.Length) {
			_innerData.RemoveRange(memoryState.Length, _innerData.Count - memoryState.Length);
		}

		int visibleCount = memoryState.Length - _hiddenAddresses.Count;
		if(ListData.Count != visibleCount) {
			ListData.Replace(_innerData.GetRange(0, visibleCount));
		}

		List<MemoryAddressViewModel> list = ListData.GetInnerList();
		for(int i = 0, len = list.Count; i < len; i++) {
			list[i].Update();
		}

		_isRefreshing = false;
	}

	public void Sort(List<Tuple<string, ListSortDirection>> sortOrder)
	{
		byte[] mem = _memoryState;
		byte[] prevMem = _prevMemoryState;

		AddressCounters[] counters = Array.Empty<AddressCounters>();
		foreach((string column, ListSortDirection order) in SortState.SortOrder) {
			if(column.Contains("Read") || column.Contains("Write") || column.Contains("Exec")) {
				//Only get counters if user sorted on a counter column
				counters = DebugApi.GetMemoryAccessCounts(MemoryType);
				break;
			}
		}

		Dictionary<string, Func<int, int, int>> comparers = new() {
			{ "Address", (a, b) => a.CompareTo(b) },
			{ "Value", (a, b) => GetValue(a, mem).CompareTo(GetValue(b, mem)) },
			{ "PrevValue", (a, b) => GetValue(a, prevMem).CompareTo(GetValue(b, prevMem)) },
			{ "ReadCount", (a, b) => counters[a].ReadCounter.CompareTo(counters[b].ReadCounter) },
			{ "LastRead", (a, b) => -counters[a].ReadStamp.CompareTo(counters[b].ReadStamp) },
			{ "WriteCount", (a, b) => counters[a].WriteCounter.CompareTo(counters[b].WriteCounter) },
			{ "LastWrite", (a, b) => -counters[a].WriteStamp.CompareTo(counters[b].WriteStamp) },
			{ "ExecCount", (a, b) => counters[a].ExecCounter.CompareTo(counters[b].ExecCounter) },
			{ "LastExec", (a, b) => -counters[a].ExecStamp.CompareTo(counters[b].ExecStamp) },
		};

		SortHelper.SortArray(_addressLookup, sortOrder, comparers, "Address");
	}

	public long GetValue(int address, byte[] memoryState)
	{
		uint value = 0;
		for(int i = 0; i < (int)ValueSize && address + i < memoryState.Length; i++) {
			value |= (uint)memoryState[address + i] << (i * 8);
		}

		switch(Format) {
			default:
			case MemorySearchFormat.Hex: {
				return value;
			}

			case MemorySearchFormat.Signed:
				switch(ValueSize) {
					default:
					case MemorySearchValueSize.Byte: return (sbyte)value;
					case MemorySearchValueSize.Word: return (short)value;
					case MemorySearchValueSize.Dword: return (int)value;
				}

			case MemorySearchFormat.Unsigned:
				switch(ValueSize) {
					default:
					case MemorySearchValueSize.Byte: return (byte)value;
					case MemorySearchValueSize.Word: return (ushort)value;
					case MemorySearchValueSize.Dword: return (uint)value;
				}
		}
	}

	public bool IsMatch(int address)
	{
		long value = GetValue(address, _memoryState);
		
		long compareValue = CompareTo switch {
			MemorySearchCompareTo.PreviousSearchValue => GetValue(address, _lastSearchSnapshot),
			MemorySearchCompareTo.PreviousRefreshValue => GetValue(address, _prevMemoryState),
			MemorySearchCompareTo.SpecificAddress => GetValue(SpecificAddress, _memoryState),
			MemorySearchCompareTo.SpecificValue => SpecificValue,
			_ => throw new Exception("Unsupported compare type")
		};

		return Operator switch {
			MemorySearchOperator.Equal => value == compareValue,
			MemorySearchOperator.NotEqual => value != compareValue,
			MemorySearchOperator.LessThan => value < compareValue,
			MemorySearchOperator.LessThanOrEqual => value <= compareValue,
			MemorySearchOperator.GreaterThan => value > compareValue,
			MemorySearchOperator.GreaterThanOrEqual => value >= compareValue,
			_ => throw new Exception("Unsupported operator")
		};
	}

	public void AddFilter()
	{
		_undoHistory.Add(new SearchHistory(_hiddenAddresses, _lastSearchSnapshot));
		IsUndoEnabled = true;

		int count = _lastSearchSnapshot.Length;
		for(int i = 0; i < count; i++) {
			if(_hiddenAddresses.Contains(i)) {
				continue;
			} else if(!IsMatch(i)) {
				_hiddenAddresses.Add(i);
			}
		}

		UpdateAddressLookup();

		_lastSearchSnapshot = DebugApi.GetMemoryState(MemoryType);
		RefreshList(true);
	}

	private void UpdateAddressLookup()
	{
		int count = _lastSearchSnapshot.Length;
		int[] addressLookup = new int[count];
		int visibleRowCounter = 0;

		for(int i = 0; i < count; i++) {
			if(!_hiddenAddresses.Contains(i)) {
				addressLookup[visibleRowCounter] = i;
				visibleRowCounter++;
			}
		}

		Array.Resize(ref addressLookup, visibleRowCounter);
		_addressLookup = addressLookup;
	}

	public void ResetSearch()
	{
		_lastSearchSnapshot = DebugApi.GetMemoryState(MemoryType);
		_addressLookup = Enumerable.Range(0, _lastSearchSnapshot.Length).ToArray();
		MaxAddress = Math.Max(0, _lastSearchSnapshot.Length - 1);
		_hiddenAddresses.Clear();
		_undoHistory.Clear();
		IsUndoEnabled = false;
		_memoryState = DebugApi.GetMemoryState(MemoryType);
		RefreshData(true);
	}

	public void Undo()
	{
		if(_undoHistory.Count > 0) {
			_hiddenAddresses = _undoHistory[^1].HiddenAddresses;
			_lastSearchSnapshot = _undoHistory[^1].SearchSnapshot;
			_undoHistory.RemoveAt(_undoHistory.Count - 1);
			IsUndoEnabled = _undoHistory.Count > 0;
			UpdateAddressLookup();
			RefreshList(true);
		}
	}

	public void ResetCounters()
	{
		DebugApi.ResetMemoryAccessCounts();
		RefreshList(true);
	}

	private class SearchHistory
	{
		public HashSet<int> HiddenAddresses;
		public byte[] SearchSnapshot;

		public SearchHistory(HashSet<int> hiddenAddresses, byte[] searchSnapshot)
		{
			HiddenAddresses = new(hiddenAddresses);
			SearchSnapshot = searchSnapshot;
		}
	}
}

public class MemoryAddressViewModel : INotifyPropertyChanged
{
	public event PropertyChangedEventHandler? PropertyChanged;

	private string _addressString = "";
	public string AddressString
	{
		get
		{
			UpdateFields();
			return _addressString;
		}
	}

	public string Value { get; set; } = "";
	public string PrevValue { get; set; } = "";

	public string ReadCount { get; set; } = "";
	public string WriteCount { get; set; } = "";
	public string ExecCount { get; set; } = "";

	public string LastRead { get; set; } = "";
	public string LastWrite { get; set; } = "";
	public string LastExec { get; set; } = "";

	public bool IsMatch { get; set; } = true;

	private int _index;
	private MemorySearchViewModel _search;

	public MemoryAddressViewModel(int index, MemorySearchViewModel memorySearch)
	{
		_index = index;
		_search = memorySearch;
	}

	static private PropertyChangedEventArgs[] _args = new[] {
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.AddressString)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.Value)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.PrevValue)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.ReadCount)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.WriteCount)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.ExecCount)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.LastRead)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.LastWrite)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.LastExec)),
			new PropertyChangedEventArgs(nameof(MemoryAddressViewModel.IsMatch))
		};

	[MethodImpl(MethodImplOptions.AggressiveInlining)]
	public void Update()
	{
		if(PropertyChanged != null) {
			PropertyChanged(this, _args[0]);
			PropertyChanged(this, _args[1]);
			PropertyChanged(this, _args[2]);
			PropertyChanged(this, _args[3]);
			PropertyChanged(this, _args[4]);
			PropertyChanged(this, _args[5]);
			PropertyChanged(this, _args[6]);
			PropertyChanged(this, _args[7]);
			PropertyChanged(this, _args[8]);
			PropertyChanged(this, _args[9]);
		}
	}

	private void UpdateFields()
	{
		if(_index >= _search.AddressLookup.Length) {
			return;
		}

		int address = _search.AddressLookup[_index];
		_addressString = address.ToString("X4");

		uint value = 0;
		uint prevValue = 0;
		for(int i = 0; i < (int)_search.ValueSize && address + i < _search.MemoryState.Length; i++) {
			value |= (uint)_search.MemoryState[address + i] << (i * 8);
			prevValue |= (uint)_search.PrevMemoryState[address + i] << (i * 8);
		}

		switch(_search.Format) {
			case MemorySearchFormat.Hex: {
				string format = "X" + (((int)_search.ValueSize) * 2);
				Value = value.ToString(format);
				PrevValue = prevValue.ToString(format);
				break;
			}

			case MemorySearchFormat.Signed: {
				switch(_search.ValueSize) {
					case MemorySearchValueSize.Byte:
						Value = ((sbyte)value).ToString();
						PrevValue = ((sbyte)prevValue).ToString();
						break;
					case MemorySearchValueSize.Word:
						Value = ((short)value).ToString();
						PrevValue = ((short)prevValue).ToString();
						break;
					case MemorySearchValueSize.Dword:
						Value = ((int)value).ToString();
						PrevValue = ((int)prevValue).ToString();
						break;
				}
				break;
			}

			case MemorySearchFormat.Unsigned: {
				switch(_search.ValueSize) {
					case MemorySearchValueSize.Byte:
						Value = ((byte)value).ToString();
						PrevValue = ((byte)prevValue).ToString();
						break;
					case MemorySearchValueSize.Word:
						Value = ((ushort)value).ToString();
						PrevValue = ((ushort)prevValue).ToString();
						break;
					case MemorySearchValueSize.Dword:
						Value = ((uint)value).ToString();
						PrevValue = ((uint)prevValue).ToString();
						break;
				}
				break;
			}
		}

		AddressCounters counters = DebugApi.GetMemoryAccessCounts((uint)address, 1, _search.MemoryType)[0];
		UInt64 masterClock = EmuApi.GetTimingInfo(_search.MemoryType.ToCpuType()).MasterClock;

		ReadCount = CodeTooltipHelper.FormatCount(counters.ReadCounter);
		WriteCount = CodeTooltipHelper.FormatCount(counters.WriteCounter);
		ExecCount = CodeTooltipHelper.FormatCount(counters.ExecCounter);

		LastRead = CodeTooltipHelper.FormatCount(masterClock - counters.ReadStamp, counters.ReadStamp);
		LastWrite = CodeTooltipHelper.FormatCount(masterClock - counters.WriteStamp, counters.WriteStamp);
		LastExec = CodeTooltipHelper.FormatCount(masterClock - counters.ExecStamp, counters.ExecStamp);

		IsMatch = _search.IsMatch(address);
	}
}

public enum MemorySearchFormat
{
	Hex,
	Signed,
	Unsigned,
}

public enum MemorySearchValueSize
{
	Byte = 1,
	Word = 2,
	Dword = 4,
}

public enum MemorySearchCompareTo
{
	PreviousSearchValue,
	PreviousRefreshValue,
	SpecificValue,
	SpecificAddress
}

public enum MemorySearchOperator
{
	Equal,
	NotEqual,
	LessThan,
	GreaterThan,
	LessThanOrEqual,
	GreaterThanOrEqual,
}