using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger
{
	class WatchManager
	{
		public static event EventHandler WatchChanged;
		private static List<string> _watchEntries = new List<string>();
		private static Regex _arrayWatchRegex = new Regex(@"\[((\$[0-9A-Fa-f]+)|(\d+)|([@_a-zA-Z0-9]+))\s*,\s*(\d+)\]", RegexOptions.Compiled);
		public static Regex FormatSuffixRegex = new Regex(@"^(.*),\s*([B|H|S|U])([\d]){0,1}$", RegexOptions.Compiled);

		public static List<string> WatchEntries
		{
			get { return _watchEntries; }
			set
			{
				_watchEntries = new List<string>(value);
				WatchChanged?.Invoke(null, EventArgs.Empty);
			}
		}

		public static List<WatchValueInfo> GetWatchContent(List<WatchValueInfo> previousValues)
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

				bool forceHasChanged = false;
				Match match = _arrayWatchRegex.Match(expression);
				if(match.Success) {
					//Watch expression matches the array display syntax (e.g: [$300,10] = display 10 bytes starting from $300)
					newValue = ProcessArrayDisplaySyntax(style, ref forceHasChanged, match);
				} else {
					Int32 result = DebugApi.EvaluateExpression(exprToEvaluate, out resultType, true);
					switch(resultType) {
						case EvalResultType.Numeric: newValue = FormatValue(result, style, byteLength); break;
						case EvalResultType.Boolean: newValue = result == 0 ? "false" : "true";	break;
						case EvalResultType.Invalid: newValue = "<invalid expression>"; forceHasChanged = true; break;
						case EvalResultType.DivideBy0: newValue = "<division by zero>"; forceHasChanged = true; break;
						case EvalResultType.OutOfScope: newValue = "<label out of scope>"; forceHasChanged = true; break;
					}
				}

				list.Add(new WatchValueInfo() { Expression = expression, Value = newValue, HasChanged = forceHasChanged || (i < previousValues.Count ? (previousValues[i].Value != newValue) : false) });
			}

			return list;
		}

		private static string FormatValue(int value, WatchFormatStyle style, int byteLength)
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
					if(bitCount < 32) {
						if(((value >> (bitCount - 1)) & 0x01) == 0x01) {
							//Negative value
							return (value | (-(1 << bitCount))).ToString();
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

		public static bool IsArraySyntax(string expression)
		{
			return _arrayWatchRegex.IsMatch(expression);
		}

		private static bool ProcessFormatSpecifier(ref string expression, ref WatchFormatStyle style, ref int byteLength)
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

		private static string ProcessArrayDisplaySyntax(WatchFormatStyle style, ref bool forceHasChanged, Match match)
		{
			string newValue;
			int address;
			if(match.Groups[2].Value.Length > 0) {
				address = int.Parse(match.Groups[2].Value.Substring(1), System.Globalization.NumberStyles.HexNumber);
			} else if(match.Groups[3].Value.Length > 0) {
				address = int.Parse(match.Groups[3].Value);
			} else {
				return "<invalid expression>";
				/*CodeLabel label = LabelManager.GetLabel(match.Groups[4].Value);
				if(label == null) {
					forceHasChanged = true;
					return "<invalid label>";
				}
				address = label.GetRelativeAddress();*/
			}
			int elemCount = int.Parse(match.Groups[5].Value);

			if(address >= 0) {
				List<string> values = new List<string>(elemCount);
				for(int j = address, end = address + elemCount; j < end; j++) {
					int memValue = DebugApi.GetMemoryValue(SnesMemoryType.CpuMemory, (uint)j);
					values.Add(FormatValue(memValue, style, 1));
				}
				newValue = string.Join(" ", values);
			} else {
				newValue = "<label out of scope>";
				forceHasChanged = true;
			}

			return newValue;
		}

		public static void AddWatch(params string[] expressions)
		{
			foreach(string expression in expressions) {
				_watchEntries.Add(expression);
			}
			WatchChanged?.Invoke(null, EventArgs.Empty);
		}

		public static void UpdateWatch(int index, string expression)
		{
			if(string.IsNullOrWhiteSpace(expression)) {
				RemoveWatch(index);
			} else {
				if(index >= _watchEntries.Count) {
					_watchEntries.Add(expression);
				} else {
					_watchEntries[index] = expression;
				}
				WatchChanged?.Invoke(null, EventArgs.Empty);
			}
		}

		public static void RemoveWatch(params int[] indexes)
		{
			HashSet<int> set = new HashSet<int>(indexes);
			_watchEntries = _watchEntries.Where((el, index) => !set.Contains(index)).ToList();
			//_previousValues = _previousValues.Where((el, index) => !set.Contains(index)).ToList();
			WatchChanged?.Invoke(null, EventArgs.Empty);
		}

		public static void Import(string filename)
		{
			if(File.Exists(filename)) {
				WatchManager.WatchEntries = new List<string>(File.ReadAllLines(filename));
			}
		}

		public static void Export(string filename)
		{
			File.WriteAllLines(filename, WatchManager.WatchEntries);
		}
	}

	public class WatchValueInfo
	{
		public string Expression { get; set; }
		public string Value { get; set; }
		public bool HasChanged { get; set; }
	}

	public enum WatchFormatStyle
	{
		Unsigned,
		Signed,
		Hex,
		Binary
	}
}
