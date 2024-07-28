using Avalonia.Media;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace Mesen.Debugger
{
	public class WatchManager
	{
		public delegate void WatchChangedEventHandler(bool resetSelection);

		public static Regex FormatSuffixRegex = new Regex(@"^(.*),\s*([B|H|S|U])([\d]){0,1}$", RegexOptions.Compiled);
		private static Regex _arrayWatchRegex = new Regex(@"\[((\$[0-9A-Fa-f]+)|(\d+)|([@_a-zA-Z0-9]+))\s*,\s*(\d+)\]", RegexOptions.Compiled);

		public event WatchChangedEventHandler? WatchChanged;
		
		private List<string> _watchEntries = new List<string>();
		private CpuType _cpuType;

		private static Dictionary<CpuType, WatchManager> _watchManagers = new Dictionary<CpuType, WatchManager>();

		public static WatchManager GetWatchManager(CpuType cpuType)
		{
			WatchManager? manager;
			if(!_watchManagers.TryGetValue(cpuType, out manager)) {
				manager = new WatchManager(cpuType);
				_watchManagers[cpuType] = manager;
			}
			return manager;
		}

		public WatchManager(CpuType cpuType)
		{
			_cpuType = cpuType;
		}

		public List<string> WatchEntries
		{
			get { return _watchEntries; }
			set
			{
				_watchEntries = new List<string>(value);
				WatchChanged?.Invoke(true);
			}
		}

		public List<WatchValueInfo> GetWatchContent(IList<WatchValueInfo> previousValues)
		{
			WatchFormatStyle defaultStyle = ConfigManager.Config.Debug.Debugger.WatchFormat;
			int defaultByteLength = 1;
			if(defaultStyle == WatchFormatStyle.Signed) {
				defaultByteLength = 4;
			}

			var list = new List<WatchValueInfo>();
			for(int i = 0; i < _watchEntries.Count; i++) {
				string expression = _watchEntries[i].Trim();
				string newValue = "";
				EvalResultType resultType;

				string exprToEvaluate = expression;
				WatchFormatStyle style = defaultStyle;
				int byteLength = defaultByteLength;
				if(expression.StartsWith("{") && expression.EndsWith("}")) {
					//Default to 2-byte values when using {} syntax
					byteLength = 2;
				}

				ProcessFormatSpecifier(ref exprToEvaluate, ref style, ref byteLength);

				Int64 numericValue = -1;

				bool forceHasChanged = false;
				Match match = _arrayWatchRegex.Match(expression);
				if(match.Success) {
					//Watch expression matches the array display syntax (e.g: [$300,10] = display 10 bytes starting from $300)
					newValue = ProcessArrayDisplaySyntax(style, ref forceHasChanged, match);
				} else {
					Int64 result = DebugApi.EvaluateExpression(exprToEvaluate, _cpuType, out resultType, true);
					switch(resultType) {
						case EvalResultType.Numeric:
							numericValue = result;
							newValue = FormatValue(result, style, byteLength);
							break;

						case EvalResultType.Boolean: newValue = result == 0 ? "false" : "true";	break;
						case EvalResultType.Invalid: newValue = "<invalid expression>"; forceHasChanged = true; break;
						case EvalResultType.DivideBy0: newValue = "<division by zero>"; forceHasChanged = true; break;
						case EvalResultType.OutOfScope: newValue = "<label out of scope>"; forceHasChanged = true; break;
					}
				}

				bool isChanged = forceHasChanged || (i < previousValues.Count ? (previousValues[i].Value != newValue) : false);
				list.Add(new WatchValueInfo() { Expression = expression, Value = newValue, IsChanged = isChanged, NumericValue = numericValue });
			}

			list.Add(new WatchValueInfo());

			return list;
		}

		private string FormatValue(Int64 value, WatchFormatStyle style, int byteLength)
		{
			switch(style) {
				case WatchFormatStyle.Unsigned: return ((UInt32)value).ToString();
				case WatchFormatStyle.Hex: return "$" + value.ToString("X" + byteLength * 2);
				case WatchFormatStyle.Binary:
					string binary = Convert.ToString(value, 2).PadLeft(byteLength * 8, '0');
					for(int i = binary.Length - 4; i > 0; i -= 4) {
						binary = binary.Insert(i, ".");
					}
					return "%" + binary;
				case WatchFormatStyle.Signed:
					int bitCount = byteLength * 8;
					if(bitCount < 64) {
						if(((value >> (bitCount - 1)) & 0x01) == 0x01) {
							//Negative value
							return (value | (-(1L << bitCount))).ToString();
						} else {
							//Position value
							return value.ToString();
						}
					} else {
						return value.ToString();
					}

				default: throw new Exception("Unsupported format");
			}
		}

		private bool ProcessFormatSpecifier(ref string expression, ref WatchFormatStyle style, ref int byteLength)
		{
			Match match = WatchManager.FormatSuffixRegex.Match(expression);
			if(!match.Success) {
				return false;
			}

			string format = match.Groups[2].Value.ToUpperInvariant();
			switch(format[0]) {
				case 'S': style = WatchFormatStyle.Signed; break;
				case 'H': style = WatchFormatStyle.Hex; break;
				case 'B': style = WatchFormatStyle.Binary; break;
				case 'U': style = WatchFormatStyle.Unsigned; break;
				default: throw new Exception("Invalid format");
			}

			if(match.Groups[3].Success) {
				byteLength = Math.Max(Math.Min(Int32.Parse(match.Groups[3].Value), 4), 1);
			} else {
				byteLength = 1;
			}

			expression = match.Groups[1].Value;
			return true;
		}

		private string ProcessArrayDisplaySyntax(WatchFormatStyle style, ref bool forceHasChanged, Match match)
		{
			string newValue;
			Int64 address;
			if(match.Groups[2].Value.Length > 0) {
				address = Int64.Parse(match.Groups[2].Value.Substring(1), System.Globalization.NumberStyles.HexNumber);
			} else if(match.Groups[3].Value.Length > 0) {
				address = Int64.Parse(match.Groups[3].Value);
			} else if(match.Groups[4].Value.Length > 0) {
				string token = match.Groups[4].Value.Trim();
				CodeLabel? label = LabelManager.GetLabel(token);
				if(label == null) {
					Int64 value = DebugApi.EvaluateExpression(token, _cpuType, out EvalResultType result, true);
					if(result == EvalResultType.Numeric) {
						address = value;
					} else {
						forceHasChanged = true;
						return "<invalid label>";
					}
				} else {
					address = label.GetRelativeAddress(_cpuType).Address;
				}
			} else {
				return "<invalid expression>";
			}

			int elemCount = int.Parse(match.Groups[5].Value);

			if(address >= 0) {
				List<string> values = new List<string>(elemCount);
				MemoryType memType = _cpuType.ToMemoryType();
				for(Int64 j = address, end = address + elemCount; j < end; j++) {
					if(j > UInt32.MaxValue) {
						break;
					}
					int memValue = DebugApi.GetMemoryValue(memType, (UInt32)j);
					values.Add(FormatValue(memValue, style, 1));
				}
				newValue = string.Join(" ", values);
			} else {
				newValue = "<label out of scope>";
				forceHasChanged = true;
			}

			return newValue;
		}

		public void AddWatch(params string[] expressions)
		{
			foreach(string expression in expressions) {
				_watchEntries.Add(expression);
			}
			WatchChanged?.Invoke(false);
			DebugWorkspaceManager.AutoSave();
		}

		public void UpdateWatch(int index, string expression)
		{
			if(string.IsNullOrWhiteSpace(expression)) {
				if(index < _watchEntries.Count) {
					RemoveWatch(index);
				}
			} else {
				if(index >= _watchEntries.Count) {
					_watchEntries.Add(expression);
				} else {
					if(_watchEntries[index] == expression) {
						return;
					}
					_watchEntries[index] = expression;
				}
				WatchChanged?.Invoke(false);
			}
			DebugWorkspaceManager.AutoSave();
		}

		public void RemoveWatch(params int[] indexes)
		{
			HashSet<int> set = new HashSet<int>(indexes);
			_watchEntries = _watchEntries.Where((el, index) => !set.Contains(index)).ToList();
			WatchChanged?.Invoke(true);
			DebugWorkspaceManager.AutoSave();
		}

		public void Import(string filename)
		{
			if(File.Exists(filename)) {
				WatchEntries = new List<string>(File.ReadAllLines(filename));
			}
		}

		public void Export(string filename)
		{
			File.WriteAllLines(filename, WatchEntries);
		}

		private string GetFormatString(WatchFormatStyle format, int byteLength)
		{
			string formatString = ", ";
			switch(format) {
				case WatchFormatStyle.Binary: formatString += "B"; break;
				case WatchFormatStyle.Hex: formatString += "H"; break;
				case WatchFormatStyle.Signed: formatString += "S"; break;
				case WatchFormatStyle.Unsigned: formatString += "U"; break;
				default: throw new Exception("Unsupported type");
			}
			if(byteLength > 1) {
				formatString += byteLength.ToString();
			}
			return formatString;
		}

		internal void ClearSelectionFormat(int[] indexes)
		{
			SetSelectionFormat("", indexes);
		}

		public void SetSelectionFormat(WatchFormatStyle format, int byteLength, int[] indexes)
		{
			string formatString = GetFormatString(format, byteLength);
			SetSelectionFormat(formatString, indexes);
		}

		private void SetSelectionFormat(string formatString, int[] indexes)
		{
			foreach(int i in indexes) {
				if(i < _watchEntries.Count) {
					Match match = WatchManager.FormatSuffixRegex.Match(_watchEntries[i]);
					if(match.Success) {
						UpdateWatch(i, match.Groups[1].Value + formatString);
					} else {
						UpdateWatch(i, _watchEntries[i] + formatString);
					}
				}
			}
		}
	}

	public class WatchValueInfo : ReactiveObject
	{
		[Reactive] public string Value { get; set; } = "";
		[Reactive] public string Expression { get; set; } = "";
		[Reactive] public bool IsChanged { get; set; } = false;
		public Int64 NumericValue { get; set; } = -1;
	}

	public enum WatchFormatStyle
	{
		Unsigned,
		Signed,
		Hex,
		Binary
	}
}
