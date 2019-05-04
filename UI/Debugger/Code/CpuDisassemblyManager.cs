using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using Mesen.GUI.Debugger.Labels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Mesen.GUI.Debugger.Integration.DbgImporter;

namespace Mesen.GUI.Debugger.Code
{
	public class CpuDisassemblyManager : IDisassemblyManager
	{
		protected ICodeDataProvider _provider;
		private DbgImporter _symbolProvider;

		public ICodeDataProvider Provider { get { return this._provider; } }

		public virtual SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.CpuMemory; } }
		public virtual int AddressSize { get { return 6; } }
		public virtual int ByteCodeSize { get { return 4; } }
		public virtual bool AllowSourceView { get { return true; } }

		public virtual void RefreshCode(DbgImporter symbolProvider, DbgImporter.FileInfo file)
		{
			_symbolProvider = symbolProvider;
			if(file == null) {
				this._provider = new CodeDataProvider(CpuType.Cpu);
			} else {
				this._provider = new DbgCodeDataProvider(CpuType.Cpu, symbolProvider, file);
			}
		}

		public void ToggleBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				AddressInfo relAddress = new AddressInfo() {
					Address = address,
					Type = RelativeMemoryType
				};

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
				if(absAddress.Address < 0) {
					BreakpointManager.ToggleBreakpoint(relAddress);
				} else {
					BreakpointManager.ToggleBreakpoint(absAddress);
				}
			}
		}

		public void EnableDisableBreakpoint(int lineIndex)
		{
			int address = this._provider.GetLineAddress(lineIndex);
			if(address >= 0) {
				BreakpointManager.EnableDisableBreakpoint(new AddressInfo() {
					Address = address,
					Type = RelativeMemoryType
				});
			}
		}

		public Dictionary<string, string> GetTooltipData(string word, int lineIndex)
		{
			int? arrayIndex = null;
			int arraySeparatorIndex = word.IndexOf("+");
			if(arraySeparatorIndex >= 0) {
				int index;
				if(int.TryParse(word.Substring(arraySeparatorIndex + 1), out index)) {
					arrayIndex = index;
				}
				word = word.Substring(0, arraySeparatorIndex);
			}

			if(_provider is DbgCodeDataProvider && _symbolProvider != null) {
				int rangeStart, rangeEnd;
				GetSymbolByteRange(lineIndex, out rangeStart, out rangeEnd);
				SymbolInfo symbol = _symbolProvider.GetSymbol(word, rangeStart, rangeEnd);
				if(symbol != null) {
					AddressInfo? symbolAddress = _symbolProvider.GetSymbolAddressInfo(symbol);

					if(symbolAddress != null && symbolAddress.Value.Address >= 0) {
						int relativeAddress = DebugApi.GetRelativeAddress(symbolAddress.Value).Address;
						byte byteValue = relativeAddress >= 0 ? DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress) : (byte)0;
						UInt16 wordValue = relativeAddress >= 0 ? (UInt16)(byteValue | (DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress + 1) << 8)) : (UInt16)0;

						var values = new Dictionary<string, string>() {
							{ "Symbol", symbol.Name + (arrayIndex != null ? $"+{arrayIndex.Value}" : "") }
						};

						if(relativeAddress >= 0) {
							values["CPU Address"] = "$" + relativeAddress.ToString("X4");
						} else {
							values["CPU Address"] = "<out of scope>";
						}

						if(symbolAddress.Value.Type == SnesMemoryType.PrgRom) {
							values["PRG Offset"] = "$" + (symbolAddress.Value.Address + (arrayIndex ?? 0)).ToString("X4");
						}

						values["Value"] = (relativeAddress >= 0 ? $"${byteValue.ToString("X2")} (byte){Environment.NewLine}${wordValue.ToString("X4")} (word)" : "n/a");
						return values;
					} else {
						return new Dictionary<string, string>() {
							{ "Symbol", symbol.Name },
							{ "Constant", symbol.Address.HasValue ? ("$" + symbol.Address.Value.ToString("X2")) : "<unknown>" }
						};
					}
				}
			} else {
				CodeLabel label = LabelManager.GetLabel(word);
				if(label != null) {
					AddressInfo absAddress = label.GetAbsoluteAddress();
					int relativeAddress;
					if(absAddress.Type == SnesMemoryType.Register) {
						relativeAddress = absAddress.Address;
					} else {
						relativeAddress = label.GetRelativeAddress().Address;
					}

					byte byteValue = relativeAddress >= 0 ? DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress) : (byte)0;
					UInt16 wordValue = relativeAddress >= 0 ? (UInt16)(byteValue | (DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress + 1) << 8)) : (UInt16)0;

					var values = new Dictionary<string, string>() {
						{ "Label", label.Label + (arrayIndex != null ? $"+{arrayIndex.Value}" : "") },
						{ "Address", (relativeAddress >= 0 ? "$" + relativeAddress.ToString("X4") : "n/a") },
						{ "Value", (relativeAddress >= 0 ? $"${byteValue.ToString("X2")} (byte){Environment.NewLine}${wordValue.ToString("X4")} (word)" : "n/a") },
					};

					if(!string.IsNullOrWhiteSpace(label.Comment)) {
						values["Comment"] = label.Comment;
					}
					return values;
				} 
			}

			System.Globalization.NumberStyles style = System.Globalization.NumberStyles.None;
			if(word.StartsWith("$")) {
				style = System.Globalization.NumberStyles.HexNumber;
				word = word.Replace("$", "");
			}

			uint address;
			if(UInt32.TryParse(word, style, null, out address)) {
				byte byteValue = DebugApi.GetMemoryValue(this.RelativeMemoryType, address);
				UInt16 wordValue = (UInt16)(byteValue | (DebugApi.GetMemoryValue(this.RelativeMemoryType, address + 1) << 8));
				return new Dictionary<string, string>() {
					{ "Address", "$" + address.ToString("X4") },
					{ "Value", $"${byteValue.ToString("X2")} (byte){Environment.NewLine}${wordValue.ToString("X4")} (word)" }
				};
			}

			return null;
		}

		private void GetSymbolByteRange(int lineIndex, out int rangeStart, out int rangeEnd)
		{
			int lineCount = _provider.GetLineCount();
			while(lineIndex < lineCount - 2 && _provider.GetCodeLineData(lineIndex).AbsoluteAddress < 0) {
				//Find the address of the next line with an address
				lineIndex++;
			}

			rangeStart = _provider.GetCodeLineData(lineIndex).AbsoluteAddress;
			if(rangeStart >= 0) {
				while(lineIndex < lineCount - 2 && _provider.GetCodeLineData(lineIndex + 1).AbsoluteAddress < 0) {
					//Find the next line with an address
					lineIndex++;
				}

				rangeEnd = _provider.GetCodeLineData(lineIndex + 1).AbsoluteAddress - 1;

				if(rangeStart < rangeEnd && (rangeEnd - rangeStart) < 500) {
					return;
				}
			}

			rangeStart = 0;
			rangeEnd = Int32.MaxValue;
		}
	}
}
