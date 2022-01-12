using Mesen.Debugger.Controls;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Disassembly
{
	public class CpuDisassemblyManager : IDisassemblyManager
	{
#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.
		protected ICodeDataProvider _provider;
		private ISymbolProvider _symbolProvider;
#pragma warning restore CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

		public ICodeDataProvider Provider { get { return this._provider; } }

		public virtual CpuType CpuType { get { return CpuType.Cpu; } }
		public virtual SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.CpuMemory; } }
		public virtual SnesMemoryType PrgMemoryType { get { return SnesMemoryType.PrgRom; } }
		public virtual int AddressSize { get { return 6; } }
		public virtual int ByteCodeSize { get { return 4; } }
		public virtual bool AllowSourceView { get { return true; } }

		public virtual void RefreshCode(ISymbolProvider symbolProvider, SourceFileInfo? file)
		{
			_symbolProvider = symbolProvider;
			if(file == null) {
				this._provider = new CodeDataProvider(CpuType.Cpu);
			} else {
				this._provider = new SymbolCodeDataProvider(CpuType.Cpu, symbolProvider, file.Value);
			}
		}

		protected virtual int GetFullAddress(int address, int length)
		{
			CpuState state = DebugApi.GetCpuState<CpuState>(CpuType.Cpu);
			if(length == 4) {
				//Append current DB register to 2-byte addresses
				return (state.DBR << 16) | address;
			} else if(length == 2) {
				//Add direct register to 1-byte addresses
				return (state.D + address);
			}

			return address;
		}

		public LocationInfo GetLocationInfo(string word, int lineIndex)
		{
			LocationInfo location = new LocationInfo();

			int arraySeparatorIndex = word.IndexOf("+");
			if(arraySeparatorIndex >= 0) {
				int index;
				if(int.TryParse(word.Substring(arraySeparatorIndex + 1), out index)) {
					location.ArrayIndex = index;
				}
				word = word.Substring(0, arraySeparatorIndex);
			}

			//TODO
			/*if(_provider is SymbolCodeDataProvider && _symbolProvider != null) {
				int rangeStart, rangeEnd;
				GetSymbolByteRange(lineIndex, out rangeStart, out rangeEnd);
				location.Symbol = _symbolProvider.GetSymbol(word, rangeStart, rangeEnd);
			}*/

			location.Label = LabelManager.GetLabel(word);

			int address;
			if(location.Label != null) {
				address = location.Label.GetRelativeAddress(this.CpuType).Address;
				if(address >= 0) {
					location.Address = location.Label.GetRelativeAddress(this.CpuType).Address + (location.ArrayIndex ?? 0);
				} else {
					location.Address = -1;
				}
			} else if(word.StartsWith("$")) {
				word = word.Replace("$", "");
				if(Int32.TryParse(word, System.Globalization.NumberStyles.HexNumber, null, out address)) {
					location.Address = GetFullAddress(address, word.Length);
				}
			} else if(Int32.TryParse(word, out address)) {
				location.Address = (int)address;
			} else {
				location.Address = -1;
			}

			if(location.Label == null && location.Address >= 0) {
				AddressInfo relAddress = new AddressInfo() { Address = location.Address, Type = RelativeMemoryType };
				CodeLabel? label = LabelManager.GetLabel(relAddress);
				if(label != null && !string.IsNullOrWhiteSpace(label.Label)) {
					//ignore comment-only labels
					location.Label = label; 
				}
			}

			if(location.Label != null && location.Address >= 0) {
				AddressInfo absAddress = location.Label.GetAbsoluteAddress();
				AddressInfo absIndexedAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = location.Address, Type = RelativeMemoryType });
				if(absIndexedAddress.Address > absAddress.Address) {
					location.ArrayIndex = absIndexedAddress.Address - absAddress.Address;
				}
			}

			return location;
		}

		//TODO
		/*public Dictionary<string, string>? GetTooltipData(string word, int lineIndex)
		{
			if(_provider.GetCodeLineData(lineIndex).Flags.HasFlag(LineFlags.ShowAsData)) {
				//Disable tooltips for .db statements
				return null;
			}

			LocationInfo location = GetLocationInfo(word, lineIndex);

			if(location.Symbol != null) {
				AddressInfo? symbolAddress = _symbolProvider.GetSymbolAddressInfo(location.Symbol);

				if(symbolAddress != null && symbolAddress.Value.Address >= 0) {
					int relativeAddress = DebugApi.GetRelativeAddress(symbolAddress.Value, this.CpuType).Address;
					byte byteValue = relativeAddress >= 0 ? DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress) : (byte)0;
					UInt16 wordValue = relativeAddress >= 0 ? (UInt16)(byteValue | (DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress + 1) << 8)) : (UInt16)0;

					var values = new Dictionary<string, string>() {
						{ "Symbol", location.Symbol.Name + (location.ArrayIndex != null ? $"+{location.ArrayIndex.Value}" : "") }
					};

					if(relativeAddress >= 0) {
						values["CPU Address"] = "$" + relativeAddress.ToString("X4");
					} else {
						values["CPU Address"] = "<out of scope>";
					}

					if(symbolAddress.Value.Type == SnesMemoryType.PrgRom) {
						values["PRG Offset"] = "$" + (symbolAddress.Value.Address + (location.ArrayIndex ?? 0)).ToString("X4");
					}

					values["Value"] = (relativeAddress >= 0 ? $"${byteValue:X2} (byte){Environment.NewLine}${wordValue:X4} (word)" : "n/a");
					return values;
				} else {
					return new Dictionary<string, string>() {
						{ "Symbol", location.Symbol.Name },
						{ "Constant", location.Symbol.Address.HasValue ? ("$" + location.Symbol.Address.Value.ToString("X2")) : "<unknown>" }
					};
				}
			} else if(location.Label != null) {
				AddressInfo absAddress = location.Label.GetAbsoluteAddress();
				int relativeAddress;
				if(location.Address >= 0) {
					relativeAddress = location.Address;
					AddressInfo absIndexedAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = location.Address, Type = RelativeMemoryType });
					if(absIndexedAddress.Address > absAddress.Address) {
						location.ArrayIndex = absIndexedAddress.Address - absAddress.Address;
					}
				} else if(absAddress.Type.IsRelativeMemory() || absAddress.Type == SnesMemoryType.Register) {
					relativeAddress = absAddress.Address;
				} else {
					relativeAddress = location.Label.GetRelativeAddress(this.CpuType).Address + (location.ArrayIndex ?? 0);
				}

				byte byteValue = relativeAddress >= 0 ? DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress) : (byte)0;
				UInt16 wordValue = relativeAddress >= 0 ? (UInt16)(byteValue | (DebugApi.GetMemoryValue(this.RelativeMemoryType, (UInt32)relativeAddress + 1) << 8)) : (UInt16)0;

				var values = new Dictionary<string, string>() {
					{ "Label", location.Label.Label + (location.ArrayIndex != null ? $"+{location.ArrayIndex.Value}" : "") },
					{ "Address", relativeAddress >= 0 ? "$" + relativeAddress.ToString("X4") : "n/a" },
					{ "Value", relativeAddress >= 0 ? $"${byteValue:X2} (byte){Environment.NewLine}${wordValue:X4} (word)" : "n/a" },
				};

				if(!string.IsNullOrWhiteSpace(location.Label.Comment)) {
					values["Comment"] = location.Label.Comment;
				}
				return values;
			} else if(location.Address >= 0) {
				byte byteValue = DebugApi.GetMemoryValue(this.RelativeMemoryType, (uint)location.Address);
				UInt16 wordValue = (UInt16)(byteValue | (DebugApi.GetMemoryValue(this.RelativeMemoryType, (uint)location.Address + 1) << 8));
				return new Dictionary<string, string>() {
					{ "Address", "$" + location.Address.ToString("X4") },
					{ "Value", $"${byteValue:X2} (byte){Environment.NewLine}${wordValue:X4} (word)" }
				};
			}

			return null;
		}*/

		/*private void GetSymbolByteRange(int lineIndex, out int rangeStart, out int rangeEnd)
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
		}*/
	}
}
